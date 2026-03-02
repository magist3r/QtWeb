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
    -no-openssl
    -I "${ICU_INSTALL_DIR}/include"
    -L "${ICU_INSTALL_DIR}/lib"
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
        return 1
    fi

    mapfile -t patch_files < <(find "$PATCH_DIR" -maxdepth 1 -type f -name '*.patch' | sort)
    if [[ "${#patch_files[@]}" -eq 0 ]]; then
        return 1
    fi

    for patch_file in "${patch_files[@]}"; do
        if patch --dry-run -p1 < "$patch_file" >/dev/null 2>&1; then
            patch -p1 < "$patch_file"
        elif patch --dry-run -R -p1 < "$patch_file" >/dev/null 2>&1; then
            echo "==> patch already applied: $(basename "$patch_file")"
        else
            fail "could not apply patch: $patch_file"
        fi
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

configure_icu_env_for_qt() {
    require_dir "$ICU_INSTALL_DIR"
    export PKG_CONFIG_PATH="${ICU_INSTALL_DIR}/lib/pkgconfig${PKG_CONFIG_PATH:+:${PKG_CONFIG_PATH}}"
    export CPPFLAGS="-I${ICU_INSTALL_DIR}/include${CPPFLAGS:+ ${CPPFLAGS}}"
    export LDFLAGS="-L${ICU_INSTALL_DIR}/lib${LDFLAGS:+ ${LDFLAGS}}"
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
configure_icu_env_for_qt

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
configure_qt "${LOG_DIR}/configure.log"

if webkit_disabled_by_static_notice "${LOG_DIR}/configure.log" && apply_patches; then
    configure_qt "${LOG_DIR}/configure-retry.log"
    cp "${LOG_DIR}/configure-retry.log" "${LOG_DIR}/configure.log"
fi

if webkit_disabled_by_static_notice "${LOG_DIR}/configure.log"; then
    log_verify "warning: configure still reports static WebKit disable notice; continuing to build and validating via installed libraries"
fi

echo "==> build Qt (make -j${JOBS})"
make -j"$JOBS" >"${LOG_DIR}/build.log" 2>&1
echo "==> install Qt"
make install >"${LOG_DIR}/install.log" 2>&1
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
    echo "verified_libs=${required_libs[*]}"
    echo "verified_icu_libs=${required_icu_libs[*]}"
} > "$MANIFEST_FILE"

log_verify "verification passed"
