#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

QT_PREFIX="${QT_PREFIX:-${REPO_ROOT}/artifacts/qt5-static-5.5.1/install}"
QMAKE_REAL="${QT_PREFIX}/bin/qmake"
BUILD_DIR="${SCRIPT_DIR}/build"
SHIM_DIR="${BUILD_DIR}/.qmake-shim/bin"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"

if [[ ! -x "${QMAKE_REAL}" ]]; then
    echo "error: qmake not found at ${QMAKE_REAL}" >&2
    exit 1
fi

mkdir -p "${BUILD_DIR}"
mkdir -p "${SHIM_DIR}"
ln -sf "${QMAKE_REAL}" "${SHIM_DIR}/qmake"
cat > "${SHIM_DIR}/qt.conf" <<EOF
[Paths]
Prefix=${QT_PREFIX}
EOF

cd "${BUILD_DIR}"

"${SHIM_DIR}/qmake" "${SCRIPT_DIR}/qtwebkit-smoke.pro"
make -j"${JOBS}"

echo "built: ${BUILD_DIR}/qtwebkit-smoke"
