#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

IMAGE_TAG="${IMAGE_TAG:-qtweb-qt5-static-poc:5.5.1}"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"

docker run --rm \
    -u "$(id -u):$(id -g)" \
    -e JOBS="${JOBS}" \
    -v "${REPO_ROOT}:/workspace" \
    -w /workspace \
    "${IMAGE_TAG}" \
    /bin/bash -lc '
        mkdir -p smoke-tests/qtwebkit-smoke/build-docker
        cd smoke-tests/qtwebkit-smoke/build-docker
        /workspace/artifacts/qt5-static-5.5.1/install/bin/qmake ../qtwebkit-smoke.pro
        make -j"${JOBS}"
        echo "built: /workspace/smoke-tests/qtwebkit-smoke/build-docker/qtwebkit-smoke"
    '
