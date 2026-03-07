#!/usr/bin/env bash
set -euo pipefail

: "${QT_VERSION:?QT_VERSION is required}"
: "${OUTPUT_DIR:?OUTPUT_DIR is required}"
: "${QT_SRC_ARCHIVE:?QT_SRC_ARCHIVE is required}"
: "${QTWEBKIT_ARCHIVE:?QTWEBKIT_ARCHIVE is required}"
: "${ICU_SRC_ARCHIVE:?ICU_SRC_ARCHIVE is required}"

JOBS="${JOBS:-8}"
CLEAN="${CLEAN:-false}"
BUILD_TYPE="${BUILD_TYPE:-release}"
WORK_DIR="${OUTPUT_DIR}/build"
LOG_DIR="${OUTPUT_DIR}/logs"
INSTALL_DIR="${OUTPUT_DIR}/install"
ICU_INSTALL_DIR="${OUTPUT_DIR}/icu-static"
OPENSSL_INCLUDE_DIR="/usr/include"
OPENSSL_LIB_DIR="/usr/lib/x86_64-linux-gnu"
MANIFEST_FILE="${OUTPUT_DIR}/build-manifest.txt"
QT_SRC_DIR="${WORK_DIR}/qt-everywhere-opensource-src-${QT_VERSION}"
PATCH_DIR="/workspace/toolchains/qt5-static/patches"

fail() {
    echo "error: $*" >&2
    exit 1
}

MIN_DEP_CONFIGURE_FLAGS=(
    -no-dbus
    -no-gtkstyle
    -no-fontconfig
    -xkb-config-root /usr/share/X11/xkb
    -icu
    -qt-pcre
    -qt-freetype
    -qt-xcb
    -qt-xkbcommon-x11
)
case "$BUILD_TYPE" in
    release)
        BUILD_CONFIGURE_FLAG="-release"
        ;;
    debug)
        BUILD_CONFIGURE_FLAG="-debug"
        ;;
    *)
        fail "BUILD_TYPE must be release or debug, got: $BUILD_TYPE"
        ;;
esac

CONFIGURE_FLAGS=(
    -opensource
    -confirm-license
    "$BUILD_CONFIGURE_FLAG"
    -static
    -prefix "$INSTALL_DIR"
    -nomake tests
    -nomake examples
    -nomake tools
    -qt-zlib
    -qt-libpng
    -qt-libjpeg
    -I "${ICU_INSTALL_DIR}/include"
    -L "${ICU_INSTALL_DIR}/lib"
    -L "${OPENSSL_LIB_DIR}"
    -openssl-linked
    "${MIN_DEP_CONFIGURE_FLAGS[@]}"
)

require_file() {
    local path="$1"
    [[ -f "$path" ]] || fail "missing file: $path"
}

require_dir() {
    local path="$1"
    [[ -d "$path" ]] || fail "missing directory: $path"
}

log_verify() {
    local message="$1"
    echo "$message"
    echo "$message" >> "${LOG_DIR}/verify.log"
}

apply_patches() {
    local patch_files=()
    local patch_file

    if [[ ! -d "$PATCH_DIR" ]]; then
        return 0
    fi

    mapfile -t patch_files < <(find "$PATCH_DIR" -maxdepth 1 -type f -name '*.patch' | sort)
    if [[ "${#patch_files[@]}" -eq 0 ]]; then
        return 0
    fi

    for patch_file in "${patch_files[@]}"; do
        patch -N -p1 < "$patch_file"
    done
}

configure_qt() {
    local log_file="$1"
    echo "==> configure Qt"
    ./configure -v "${CONFIGURE_FLAGS[@]}" >"$log_file" 2>&1
}

webkit_disabled_by_static_notice() {
    local log_file="$1"
    grep -q "Using static linking will disable the WebKit module." "$log_file"
}

build_static_icu() {
    local extract_dir icu_top_dir icu_source_dir

    require_file "$ICU_SRC_ARCHIVE"

    if [[ -f "${ICU_INSTALL_DIR}/lib/libicuuc.a" && -f "${ICU_INSTALL_DIR}/lib/libicui18n.a" && -f "${ICU_INSTALL_DIR}/lib/libicudata.a" ]]; then
        echo "==> reusing static ICU install: ${ICU_INSTALL_DIR}"
        return
    fi

    echo "==> build static ICU"
    extract_dir="${WORK_DIR}/_icu_extract"
    rm -rf "$extract_dir"
    mkdir -p "$extract_dir"
    tar -xf "$ICU_SRC_ARCHIVE" -C "$extract_dir"

    icu_top_dir="$(find "$extract_dir" -mindepth 1 -maxdepth 1 -type d | head -n 1)"
    [[ -n "$icu_top_dir" ]] || fail "could not identify extracted ICU source root"

    if [[ -f "${icu_top_dir}/source/configure" ]]; then
        icu_source_dir="${icu_top_dir}/source"
    elif [[ -f "${icu_top_dir}/icu/source/configure" ]]; then
        icu_source_dir="${icu_top_dir}/icu/source"
    else
        fail "could not find ICU configure script in extracted archive"
    fi

    rm -rf "$ICU_INSTALL_DIR"
    pushd "$icu_source_dir" >/dev/null
    ./configure \
        --prefix="$ICU_INSTALL_DIR" \
        --disable-shared \
        --enable-static \
        --disable-dyload \
        --with-data-packaging=static \
        CFLAGS="-fPIC" \
        CXXFLAGS="-fPIC" >"${LOG_DIR}/icu-configure.log" 2>&1
    make -j"$JOBS" >"${LOG_DIR}/icu-build.log" 2>&1
    make install >"${LOG_DIR}/icu-install.log" 2>&1
    popd >/dev/null

    rm -rf "$extract_dir"
}

configure_build_env_for_qt() {
    require_dir "$ICU_INSTALL_DIR"
    echo "==> verify system OpenSSL static archives"
    require_file "${OPENSSL_INCLUDE_DIR}/openssl/ssl.h"
    require_file "${OPENSSL_LIB_DIR}/libssl.a"
    require_file "${OPENSSL_LIB_DIR}/libcrypto.a"
    export PKG_CONFIG_PATH="${ICU_INSTALL_DIR}/lib/pkgconfig${PKG_CONFIG_PATH:+:${PKG_CONFIG_PATH}}"
    export CPPFLAGS="-I${ICU_INSTALL_DIR}/include${CPPFLAGS:+ ${CPPFLAGS}}"
    export LDFLAGS="-L${OPENSSL_LIB_DIR} -L${ICU_INSTALL_DIR}/lib${LDFLAGS:+ ${LDFLAGS}}"
    export OPENSSL_LIBS="-Wl,-Bstatic ${OPENSSL_LIB_DIR}/libssl.a ${OPENSSL_LIB_DIR}/libcrypto.a -Wl,-Bdynamic -ldl -lpthread -lz"
}

sync_installed_qmake_metadata() {
    local qtbase_build_dir="${QT_SRC_DIR}/qtbase"
    local build_qconfig="${qtbase_build_dir}/mkspecs/qconfig.pri"
    local build_qmodule="${qtbase_build_dir}/mkspecs/qmodule.pri"
    local build_network_prl="${qtbase_build_dir}/lib/libQt5Network.prl"
    local install_qconfig="${INSTALL_DIR}/mkspecs/qconfig.pri"
    local install_qmodule="${INSTALL_DIR}/mkspecs/qmodule.pri"
    local install_network_prl="${INSTALL_DIR}/lib/libQt5Network.prl"
    local install_lib_expr='$$[QT_INSTALL_LIBS]'

    require_file "$build_qconfig"
    require_file "$build_qmodule"
    require_file "$build_network_prl"
    require_dir "${INSTALL_DIR}/mkspecs"
    require_dir "${INSTALL_DIR}/lib"

    cp "$build_qconfig" "$install_qconfig"
    cp "$build_qmodule" "$install_qmodule"
    sed "s|${qtbase_build_dir}/lib|${install_lib_expr}|g" "$build_network_prl" > "$install_network_prl"
}

mkdir -p "$WORK_DIR" "$LOG_DIR"
require_file "$QT_SRC_ARCHIVE"
require_file "$QTWEBKIT_ARCHIVE"
require_file "$ICU_SRC_ARCHIVE"

case "${CLEAN,,}" in
    1|true|yes|on)
        rm -rf "$QT_SRC_DIR" "$INSTALL_DIR" "$ICU_INSTALL_DIR"
        ;;
esac

build_static_icu
configure_build_env_for_qt

if [[ ! -d "$QT_SRC_DIR" ]]; then
    tar -xf "$QT_SRC_ARCHIVE" -C "$WORK_DIR"
fi
[[ -d "$QT_SRC_DIR" ]] || fail "expected source directory not found: $QT_SRC_DIR"

if [[ ! -d "${QT_SRC_DIR}/qtwebkit" ]]; then
    EXTRACT_DIR="${WORK_DIR}/_qtwebkit_extract"
    rm -rf "$EXTRACT_DIR"
    mkdir -p "$EXTRACT_DIR"
    tar -xf "$QTWEBKIT_ARCHIVE" -C "$EXTRACT_DIR"

    QTWEBKIT_SRC_DIR="$(find "$EXTRACT_DIR" -mindepth 1 -maxdepth 1 -type d | head -n 1)"
    [[ -n "$QTWEBKIT_SRC_DIR" ]] || fail "could not identify extracted QtWebKit source directory"

    mv "$QTWEBKIT_SRC_DIR" "${QT_SRC_DIR}/qtwebkit"
    rm -rf "$EXTRACT_DIR"
fi

pushd "$QT_SRC_DIR" >/dev/null
log_verify "configure minimal-deps flags: ${MIN_DEP_CONFIGURE_FLAGS[*]}"
apply_patches
configure_qt "${LOG_DIR}/configure.log"

if webkit_disabled_by_static_notice "${LOG_DIR}/configure.log"; then
    log_verify "warning: configure still reports static WebKit disable notice; continuing to build and validating via installed libraries"
fi

echo "==> build Qt (make -j${JOBS})"
make -j"$JOBS" >"${LOG_DIR}/build.log" 2>&1
echo "==> install Qt"
make install >"${LOG_DIR}/install.log" 2>&1
sync_installed_qmake_metadata
popd >/dev/null

QMAKE_BIN="${INSTALL_DIR}/bin/qmake"
require_file "$QMAKE_BIN"

QT_INSTALLED_VERSION="$("$QMAKE_BIN" -query QT_VERSION)"
QT_CONFIG=""
if ! QT_CONFIG="$("$QMAKE_BIN" -query QT_CONFIG 2>/dev/null)"; then
    if [[ -f "${INSTALL_DIR}/mkspecs/qconfig.pri" ]]; then
        QT_CONFIG="$(sed -n 's/^QT_CONFIG += *//p' "${INSTALL_DIR}/mkspecs/qconfig.pri" | tr '\n' ' ')"
    fi
fi

if [[ "$QT_INSTALLED_VERSION" != "$QT_VERSION" ]]; then
    fail "unexpected Qt version in install: $QT_INSTALLED_VERSION"
fi

if ! echo "$QT_CONFIG" | grep -qw static; then
    [[ -f "${INSTALL_DIR}/lib/libQt5Core.a" ]] || fail "installed Qt is not static"
fi

required_libs=(
    "libQt5Core.a"
    "libQt5Gui.a"
    "libQt5Widgets.a"
    "libQt5Network.a"
    "libQt5Xml.a"
    "libQt5PrintSupport.a"
    "libQt5WebKit.a"
    "libQt5WebKitWidgets.a"
)

required_icu_libs=(
    "libicuuc.a"
    "libicui18n.a"
    "libicudata.a"
)
required_ssl_libs=(
    "libssl.a"
    "libcrypto.a"
)

missing=0
for lib in "${required_libs[@]}"; do
    if [[ ! -f "${INSTALL_DIR}/lib/${lib}" ]]; then
        log_verify "missing: ${INSTALL_DIR}/lib/${lib}"
        missing=1
    fi
done

for lib in "${required_icu_libs[@]}"; do
    if [[ ! -f "${ICU_INSTALL_DIR}/lib/${lib}" ]]; then
        log_verify "missing: ${ICU_INSTALL_DIR}/lib/${lib}"
        missing=1
    fi
done
for lib in "${required_ssl_libs[@]}"; do
    if [[ ! -f "${OPENSSL_LIB_DIR}/${lib}" ]]; then
        log_verify "missing: ${OPENSSL_LIB_DIR}/${lib}"
        missing=1
    fi
done
if ! grep -q 'openssl-linked' "${INSTALL_DIR}/mkspecs/qconfig.pri"; then
    log_verify "missing openssl-linked in: ${INSTALL_DIR}/mkspecs/qconfig.pri"
    missing=1
fi
if ! grep -q 'libssl\.a' "${INSTALL_DIR}/lib/libQt5Network.prl" \
    || ! grep -q 'libcrypto\.a' "${INSTALL_DIR}/lib/libQt5Network.prl"; then
    log_verify "missing static OpenSSL archives in: ${INSTALL_DIR}/lib/libQt5Network.prl"
    missing=1
fi
[[ "$missing" -eq 0 ]] || fail "verification failed: required static libraries are missing"

{
    echo "timestamp_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
    echo "qt_version=${QT_INSTALLED_VERSION}"
    echo "qt_config=${QT_CONFIG}"
    echo "jobs=${JOBS}"
    echo "clean=${CLEAN}"
    echo "build_type=${BUILD_TYPE}"
    echo "minimal_dep_configure_flags=${MIN_DEP_CONFIGURE_FLAGS[*]}"
    echo "qt_src_archive=${QT_SRC_ARCHIVE}"
    echo "qtwebkit_archive=${QTWEBKIT_ARCHIVE}"
    echo "icu_src_archive=${ICU_SRC_ARCHIVE}"
    echo "qt_src_url=${QT_SRC_URL:-}"
    echo "qtwebkit_url=${QTWEBKIT_URL:-}"
    echo "icu_src_url=${ICU_SRC_URL:-}"
    echo "qt_src_sha256=${QT_SRC_SHA256:-}"
    echo "qtwebkit_sha256=${QTWEBKIT_SHA256:-}"
    echo "icu_src_sha256=${ICU_SRC_SHA256:-}"
    echo "qt_src_md5=${QT_SRC_MD5:-}"
    echo "qtwebkit_md5=${QTWEBKIT_MD5:-}"
    echo "icu_src_md5=${ICU_SRC_MD5:-}"
    echo "openssl_source=system-package"
    echo "openssl_include_dir=${OPENSSL_INCLUDE_DIR}"
    echo "openssl_lib_dir=${OPENSSL_LIB_DIR}"
    echo "verified_libs=${required_libs[*]}"
    echo "verified_icu_libs=${required_icu_libs[*]}"
    echo "verified_ssl_libs=${required_ssl_libs[*]}"
} > "$MANIFEST_FILE"

log_verify "verification passed"
