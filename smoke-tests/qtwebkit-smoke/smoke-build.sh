#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

BUILD_TYPE="release"
while [[ $# -gt 0 ]]; do
    case "$1" in
        --debug)
            BUILD_TYPE="debug"
            shift
            ;;
        *)
            echo "error: unknown argument: $1" >&2
            exit 1
            ;;
    esac
done

case "$BUILD_TYPE" in
    release|debug) ;;
    *)
        echo "error: BUILD_TYPE must be release or debug, got: ${BUILD_TYPE}" >&2
        exit 1
        ;;
esac

BUILD_DIR="${SCRIPT_DIR}/build-${BUILD_TYPE}"
if [[ "$BUILD_TYPE" == "release" ]]; then
    QMAKE_CONFIG_ARGS=("CONFIG+=release" "CONFIG-=debug")
else
    QMAKE_CONFIG_ARGS=("CONFIG+=debug" "CONFIG-=release")
fi

QT_PREFIX="${QT_PREFIX:-${REPO_ROOT}/artifacts/qt5-static-5.5.1-${BUILD_TYPE}/install}"
QMAKE_REAL="${QT_PREFIX}/bin/qmake"
SHIM_DIR="${BUILD_DIR}/.qmake-shim/bin"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"

if [[ ! -x "${QMAKE_REAL}" ]]; then
    echo "error: qmake not found at ${QMAKE_REAL}" >&2
    exit 1
fi

rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
mkdir -p "${SHIM_DIR}"
ln -sf "${QMAKE_REAL}" "${SHIM_DIR}/qmake"
cat > "${SHIM_DIR}/qt.conf" <<EOF
[Paths]
Prefix=${QT_PREFIX}
EOF

cd "${BUILD_DIR}"

"${SHIM_DIR}/qmake" "${SCRIPT_DIR}/qtwebkit-smoke.pro" "${QMAKE_CONFIG_ARGS[@]}"
make -j"${JOBS}"

echo "built: ${BUILD_DIR}/qtwebkit-smoke"
