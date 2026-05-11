#!/usr/bin/env bash
set -euo pipefail

: "${QT_VERSION:?QT_VERSION is required}"
: "${OUTPUT_DIR:?OUTPUT_DIR is required}"
: "${QT_SRC_ARCHIVE:?QT_SRC_ARCHIVE is required}"
: "${QTWEBKIT_ARCHIVE:?QTWEBKIT_ARCHIVE is required}"
: "${ICU_SRC_ARCHIVE:?ICU_SRC_ARCHIVE is required}"

JOBS="${JOBS:-$(nproc 2>/dev/null || echo 8)}"
QTWEBKIT_JOBS="$JOBS"
if (( QTWEBKIT_JOBS > 16 )); then
    QTWEBKIT_JOBS=16
fi
CLEAN="${CLEAN:-false}"
BUILD_TYPE="${BUILD_TYPE:-release}"
BUILD_SCOPE="${BUILD_SCOPE:-all}"
WORK_DIR="${OUTPUT_DIR}/build"
LOG_DIR="${OUTPUT_DIR}/logs"
INSTALL_DIR="${OUTPUT_DIR}/install"
ICU_INSTALL_DIR="${OUTPUT_DIR}/icu-static"
OPENSSL_INCLUDE_DIR="/usr/include"
OPENSSL_LIB_DIR="/usr/lib/x86_64-linux-gnu"
SYSTEM_LIB_DIR="/usr/lib/x86_64-linux-gnu"
DEJAVU_FONT_DIR="/usr/share/fonts/truetype/dejavu"
MANIFEST_FILE="${OUTPUT_DIR}/build-manifest.txt"
PATCH_DIR="/workspace/toolchains/qt5-static/patches"
QT_SRC_DIR=""
QTWEBKIT_SOURCE_DIR=""
QTWEBKIT_BUILD_DIR="${WORK_DIR}/qtwebkit-build-${BUILD_TYPE}"
SMOKE_DIR="/workspace/smoke-tests/qtwebkit-smoke"
SMOKE_BUILD_DIR="${SMOKE_DIR}/build-docker-${BUILD_TYPE}"

fail() {
    echo "error: $*" >&2
    exit 1
}

case "$BUILD_TYPE" in
    release)
        BUILD_CONFIGURE_FLAG="-release"
        CMAKE_BUILD_TYPE="Release"
        ;;
    debug)
        BUILD_CONFIGURE_FLAG="-debug"
        CMAKE_BUILD_TYPE="Debug"
        ;;
    *)
        fail "BUILD_TYPE must be release or debug, got: $BUILD_TYPE"
        ;;
esac

case "$BUILD_SCOPE" in
    all|qt|qtwebkit) ;;
    *)
        fail "BUILD_SCOPE must be all, qt, or qtwebkit; got: $BUILD_SCOPE"
        ;;
esac

CONFIGURE_FLAGS=(
    -opensource
    -confirm-license
    "$BUILD_CONFIGURE_FLAG"
    -static
    -prefix "$INSTALL_DIR"
    -skip qtdeclarative
    -skip qtwebengine
    -nomake tests
    -nomake examples
    -nomake tools
    -no-dbus
    -no-gtk
    -no-fontconfig
    -qt-pcre
    -qt-freetype
    -qt-zlib
    -qt-libpng
    -qt-libjpeg
    -icu
    -I "${ICU_INSTALL_DIR}/include"
    -L "${ICU_INSTALL_DIR}/lib"
    -openssl-linked
    -L "${OPENSSL_LIB_DIR}"
)

require_file() {
    local path="$1"
    [[ -f "$path" ]] || fail "missing file: $path"
}

require_dir() {
    local path="$1"
    [[ -d "$path" ]] || fail "missing directory: $path"
}

is_thin_archive() {
    local path="$1"
    file "$path" 2>/dev/null | grep -q 'thin archive'
}

log_verify() {
    local message="$1"
    echo "$message"
    echo "$message" >> "${LOG_DIR}/verify.log"
}

locate_qt_source_dir() {
    find "$WORK_DIR" -maxdepth 1 -mindepth 1 -type d -name "qt-everywhere*-${QT_VERSION}" | head -n 1
}

apply_matching_patches() {
    local tree_name="$1"
    local tree_patch_dir="${PATCH_DIR}/${tree_name}"
    local patch_files=()
    local patch_file

    if [[ ! -d "$tree_patch_dir" ]]; then
        return 0
    fi

    mapfile -t patch_files < <(find "$tree_patch_dir" -maxdepth 1 -type f -name '*.patch' | sort)
    if [[ "${#patch_files[@]}" -eq 0 ]]; then
        return 0
    fi

    for patch_file in "${patch_files[@]}"; do
        if patch --dry-run -p1 < "$patch_file" >/dev/null 2>&1; then
            echo "==> apply ${tree_name} patch: $(basename "$patch_file")"
            patch -p1 < "$patch_file"
        elif patch --dry-run -R -p1 < "$patch_file" >/dev/null 2>&1; then
            echo "==> ${tree_name} patch already applied: $(basename "$patch_file")"
        else
            fail "${tree_name} patch does not apply cleanly: $(basename "$patch_file")"
        fi
    done
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

configure_qt() {
    echo "==> configure Qt"
    ./configure -v "${CONFIGURE_FLAGS[@]}" >"${LOG_DIR}/configure.log" 2>&1
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

install_qt_runtime_fonts() {
    local font_dir="${INSTALL_DIR}/lib/fonts"
    local font_files=("${DEJAVU_FONT_DIR}"/*.ttf)

    require_dir "${INSTALL_DIR}/lib"
    require_dir "$DEJAVU_FONT_DIR"
    [[ -e "${font_files[0]}" ]] || fail "no DejaVu TTF fonts found in ${DEJAVU_FONT_DIR}"

    mkdir -p "$font_dir"
    cp -f "${font_files[@]}" "$font_dir"/
}

sync_installed_qtwebkit_metadata() {
    local webkit_module_pri="${INSTALL_DIR}/mkspecs/modules/qt_lib_webkit.pri"
    local webkitwidgets_module_pri="${INSTALL_DIR}/mkspecs/modules/qt_lib_webkitwidgets.pri"
    local install_lib_expr='$$QT_MODULE_LIB_BASE'
    local icu_lib_expr='$$[QT_INSTALL_PREFIX]/../icu-static/lib'
    local webkit_private_libs="-L${install_lib_expr} -lWebCore -lPAL -lJavaScriptCore -lWTF ${SYSTEM_LIB_DIR}/libxml2.a ${SYSTEM_LIB_DIR}/liblzma.a ${icu_lib_expr}/libicui18n.a ${icu_lib_expr}/libicuuc.a ${icu_lib_expr}/libicudata.a ${SYSTEM_LIB_DIR}/libsqlite3.a -lz -lbmalloc ${SYSTEM_LIB_DIR}/libhyphen.a -lharfbuzz-icu -latomic"

    require_file "$webkit_module_pri"

    if ! grep -Fq -- "-L${install_lib_expr}" "$webkit_module_pri"; then
        sed -i "s|^QMAKE_LIBS_PRIVATE += |QMAKE_LIBS_PRIVATE += -L${install_lib_expr} |" "$webkit_module_pri"
    fi

    sed -i "s|^QMAKE_LIBS_PRIVATE += .*|QMAKE_LIBS_PRIVATE += ${webkit_private_libs}|" "$webkit_module_pri"

    if [[ -f "$webkitwidgets_module_pri" ]]; then
        sed -i 's/\(QT\.webkitwidgets\.module_config = .*v2\)\s*$/\1 staticlib/' "$webkitwidgets_module_pri"
    fi
}

sync_installed_qt_plugin_metadata() {
    local qwebp_prl="${INSTALL_DIR}/plugins/imageformats/libqwebp.prl"

    if [[ -f "$qwebp_prl" ]]; then
        sed -i "s|-lwebpmux -lwebpdemux -lwebp|${SYSTEM_LIB_DIR}/libwebpmux.a ${SYSTEM_LIB_DIR}/libwebpdemux.a ${SYSTEM_LIB_DIR}/libwebp.a|g" "$qwebp_prl"
    fi
}

build_qtwebkit() {
    local extract_dir source_root
    local top_level_entries=()

    require_file "$QTWEBKIT_ARCHIVE"
    if [[ -f "${INSTALL_DIR}/lib/libQt5WebKit.a" && -f "${INSTALL_DIR}/lib/libQt5WebKitWidgets.a" ]]; then
        if is_thin_archive "${INSTALL_DIR}/lib/libQt5WebKit.a" || is_thin_archive "${INSTALL_DIR}/lib/libQt5WebKitWidgets.a"; then
            fail "installed QtWebKit archives are thin; remove the installed QtWebKit artifacts and rerun so they can be rebuilt as normal archives"
        fi
        sync_installed_qtwebkit_metadata
        echo "==> reusing installed QtWebKit from ${INSTALL_DIR}"
        return
    fi

    echo "==> extract QtWebKit"
    extract_dir="${WORK_DIR}/_qtwebkit_extract"
    rm -rf "$extract_dir" "$QTWEBKIT_BUILD_DIR"
    mkdir -p "$extract_dir" "$QTWEBKIT_BUILD_DIR"
    tar -xf "$QTWEBKIT_ARCHIVE" -C "$extract_dir"

    mapfile -t top_level_entries < <(find "$extract_dir" -mindepth 1 -maxdepth 1 | sort)
    if [[ "${#top_level_entries[@]}" -eq 1 && -d "${top_level_entries[0]}" ]]; then
        source_root="${top_level_entries[0]}"
        QTWEBKIT_SOURCE_DIR="${WORK_DIR}/$(basename "$source_root")"
    else
        require_file "${extract_dir}/CMakeLists.txt"
        require_dir "${extract_dir}/Source"
        source_root="$extract_dir"
        QTWEBKIT_SOURCE_DIR="${WORK_DIR}/qtwebkit-source"
    fi

    rm -rf "$QTWEBKIT_SOURCE_DIR"
    mv "$source_root" "$QTWEBKIT_SOURCE_DIR"
    if [[ "$source_root" != "$extract_dir" ]]; then
        rm -rf "$extract_dir"
    fi

    pushd "$QTWEBKIT_SOURCE_DIR" >/dev/null
    apply_matching_patches "qtwebkit"
    popd >/dev/null

    pushd "$QTWEBKIT_BUILD_DIR" >/dev/null
    echo "==> configure QtWebKit"
    local cmake_args=(
        -DPORT=Qt
        -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
        -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}"
        -DCMAKE_PREFIX_PATH="${INSTALL_DIR};${ICU_INSTALL_DIR}"
        -DQt5_DIR="${INSTALL_DIR}/lib/cmake/Qt5"
        -DICU_ROOT="${ICU_INSTALL_DIR}"
        -DENABLE_API_TESTS=OFF
        -DENABLE_TOOLS=OFF
        -DENABLE_GEOLOCATION=OFF
        -DENABLE_PRINT_SUPPORT=ON
        -DENABLE_VIDEO=OFF
        -DENABLE_WEBKIT=OFF
        -DENABLE_WEBKIT2=OFF
        -DUSE_THIN_ARCHIVES=OFF
        -DUSE_GSTREAMER=OFF
        -DUSE_LD_GOLD=OFF
        -DUSE_WOFF2=OFF # Avoid WOFF2/Brotli runtime deps; pages fall back to other fonts when available.
        -DENABLE_WEB_CRYPTO=OFF # Avoid libgcrypt/libtasn1 deps; HTTPS still works, but window.crypto.subtle is unavailable.
        -DENABLE_XSLT=OFF # Avoid libxslt dependency; legacy XML+XSLT pages will not transform client-side.
        "${QTWEBKIT_SOURCE_DIR}"
    )
    cmake -G Ninja "${cmake_args[@]}" >"${LOG_DIR}/qtwebkit-configure.log" 2>&1
    echo "==> build QtWebKit"
    ninja -j"$QTWEBKIT_JOBS" >"${LOG_DIR}/qtwebkit-build.log" 2>&1
    echo "==> install QtWebKit"
    ninja install >"${LOG_DIR}/qtwebkit-install.log" 2>&1
    sync_installed_qtwebkit_metadata
    popd >/dev/null
}

mkdir -p "$WORK_DIR" "$LOG_DIR"
require_file "$QT_SRC_ARCHIVE"
require_file "$QTWEBKIT_ARCHIVE"
require_file "$ICU_SRC_ARCHIVE"

case "${CLEAN,,}" in
    1|true|yes|on)
        find "$WORK_DIR" -maxdepth 1 -mindepth 1 -type d -name "qt-everywhere*-${QT_VERSION}" -exec rm -rf {} +
        rm -rf "$QTWEBKIT_BUILD_DIR" "$INSTALL_DIR" "$ICU_INSTALL_DIR"
        ;;
esac

if [[ "$BUILD_SCOPE" == "all" || "$BUILD_SCOPE" == "qt" ]]; then
    build_static_icu
    configure_build_env_for_qt

    if [[ ! -x "${INSTALL_DIR}/bin/qmake" ]]; then
        find "$WORK_DIR" -maxdepth 1 -mindepth 1 -type d -name "qt-everywhere*-${QT_VERSION}" -exec rm -rf {} +
    fi

    QT_SRC_DIR="$(locate_qt_source_dir)"
    if [[ -z "$QT_SRC_DIR" ]]; then
        tar -xf "$QT_SRC_ARCHIVE" -C "$WORK_DIR"
    fi
    QT_SRC_DIR="$(locate_qt_source_dir)"
    [[ -d "$QT_SRC_DIR" ]] || fail "expected source directory not found: $QT_SRC_DIR"

    pushd "$QT_SRC_DIR" >/dev/null
    apply_matching_patches "qt"
    configure_qt
    echo "==> build Qt (make -j${JOBS})"
    make -j"$JOBS" >"${LOG_DIR}/build.log" 2>&1
    echo "==> install Qt"
    make install >"${LOG_DIR}/install.log" 2>&1
    sync_installed_qmake_metadata
    install_qt_runtime_fonts
    popd >/dev/null
fi

if [[ "$BUILD_SCOPE" == "all" || "$BUILD_SCOPE" == "qtwebkit" ]]; then
    build_static_icu
    configure_build_env_for_qt
    require_file "${INSTALL_DIR}/bin/qmake"
    install_qt_runtime_fonts
    sync_installed_qt_plugin_metadata
    build_qtwebkit
    echo "==> build QtWebKit smoke test"
    require_file "${SMOKE_DIR}/smoke-build-entrypoint.sh"
    JOBS="$JOBS" \
        QT_PREFIX_IN_CONTAINER="$INSTALL_DIR" \
        BUILD_DIR_IN_CONTAINER="${SMOKE_BUILD_DIR#/workspace/}" \
        BUILD_TYPE="$BUILD_TYPE" \
        "${SMOKE_DIR}/smoke-build-entrypoint.sh" >"${LOG_DIR}/smoke-build.log" 2>&1
    require_file "${SMOKE_BUILD_DIR}/qtwebkit-smoke"
fi

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
)

if [[ "$BUILD_SCOPE" == "all" || "$BUILD_SCOPE" == "qtwebkit" ]]; then
    required_libs+=(
        "libQt5PrintSupport.a"
        "libQt5WebKit.a"
        "libQt5WebKitWidgets.a"
    )
fi

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
if ! grep -q 'openssl-linked' "${INSTALL_DIR}/mkspecs/modules/qt_lib_network_private.pri"; then
    log_verify "missing openssl-linked in: ${INSTALL_DIR}/mkspecs/modules/qt_lib_network_private.pri"
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
    echo "build_scope=${BUILD_SCOPE}"
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
    if [[ "$BUILD_SCOPE" == "all" || "$BUILD_SCOPE" == "qtwebkit" ]]; then
        echo "smoke_binary=${SMOKE_BUILD_DIR}/qtwebkit-smoke"
    fi
} > "$MANIFEST_FILE"

log_verify "verification passed"
