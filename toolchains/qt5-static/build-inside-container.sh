#!/usr/bin/env bash
set -euo pipefail

: "${QT_VERSION:?QT_VERSION is required}"
: "${OUTPUT_DIR:?OUTPUT_DIR is required}"
: "${QT_SRC_ARCHIVE:?QT_SRC_ARCHIVE is required}"
: "${QTWEBKIT_ARCHIVE:?QTWEBKIT_ARCHIVE is required}"

JOBS="${JOBS:-8}"
CLEAN="${CLEAN:-false}"
WORK_DIR="${OUTPUT_DIR}/build"
LOG_DIR="${OUTPUT_DIR}/logs"
INSTALL_DIR="${OUTPUT_DIR}/install"
MANIFEST_FILE="${OUTPUT_DIR}/build-manifest.txt"
QT_SRC_DIR="${WORK_DIR}/qt-everywhere-opensource-src-${QT_VERSION}"
PATCH_DIR="/workspace/toolchains/qt5-static/patches"
MIN_DEP_CONFIGURE_FLAGS=(
    -no-dbus
    -no-gtkstyle
    -no-fontconfig
    -icu
    -qt-pcre
    -qt-freetype
    -qt-xcb
    -qt-xkbcommon-x11
)
CONFIGURE_FLAGS=(
    -opensource
    -confirm-license
    -release
    -static
    -prefix "$INSTALL_DIR"
    -nomake tests
    -nomake examples
    -nomake tools
    -qt-zlib
    -qt-libpng
    -qt-libjpeg
    -no-openssl
    "${MIN_DEP_CONFIGURE_FLAGS[@]}"
)

fail() {
    echo "error: $*" >&2
    exit 1
}

require_file() {
    local path="$1"
    [[ -f "$path" ]] || fail "missing file: $path"
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
        patch -p1 < "$patch_file"
    done
}

configure_qt() {
    local log_file="$1"
    echo "==> configure Qt"
    ./configure "${CONFIGURE_FLAGS[@]}" >"$log_file" 2>&1
}

webkit_disabled_by_static_notice() {
    local log_file="$1"
    grep -q "Using static linking will disable the WebKit module." "$log_file"
}

mkdir -p "$WORK_DIR" "$LOG_DIR"
require_file "$QT_SRC_ARCHIVE"
require_file "$QTWEBKIT_ARCHIVE"

case "${CLEAN,,}" in
    1|true|yes|on)
        rm -rf "$QT_SRC_DIR" "$INSTALL_DIR"
        ;;
esac

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

missing=0
for lib in "${required_libs[@]}"; do
    if [[ ! -f "${INSTALL_DIR}/lib/${lib}" ]]; then
        log_verify "missing: ${INSTALL_DIR}/lib/${lib}"
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
    echo "minimal_dep_configure_flags=${MIN_DEP_CONFIGURE_FLAGS[*]}"
    echo "qt_src_archive=${QT_SRC_ARCHIVE}"
    echo "qtwebkit_archive=${QTWEBKIT_ARCHIVE}"
    echo "qt_src_url=${QT_SRC_URL:-}"
    echo "qtwebkit_url=${QTWEBKIT_URL:-}"
    echo "qt_src_sha256=${QT_SRC_SHA256:-}"
    echo "qtwebkit_sha256=${QTWEBKIT_SHA256:-}"
    echo "qt_src_md5=${QT_SRC_MD5:-}"
    echo "qtwebkit_md5=${QTWEBKIT_MD5:-}"
    echo "verified_libs=${required_libs[*]}"
} > "$MANIFEST_FILE"

log_verify "verification passed"
