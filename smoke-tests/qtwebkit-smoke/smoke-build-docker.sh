#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

IMAGE_TAG="${IMAGE_TAG:-qtweb-qt5-static-poc:5.5.1}"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"
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
    release|debug)
        QT_PREFIX_IN_CONTAINER="/workspace/artifacts/qt5-static-5.5.1-${BUILD_TYPE}/install"
        BUILD_DIR_IN_CONTAINER="smoke-tests/qtwebkit-smoke/build-docker-${BUILD_TYPE}"
        if [[ "$BUILD_TYPE" == "release" ]]; then
            QMAKE_CONFIG_ARGS="CONFIG+=release CONFIG-=debug"
        else
            QMAKE_CONFIG_ARGS="CONFIG+=debug CONFIG-=release"
        fi
        ;;
    *)
        echo "error: BUILD_TYPE must be release or debug, got: ${BUILD_TYPE}" >&2
        exit 1
        ;;
esac

docker run --rm \
    -u "$(id -u):$(id -g)" \
    -e JOBS="${JOBS}" \
    -e QT_PREFIX_IN_CONTAINER="${QT_PREFIX_IN_CONTAINER}" \
    -e BUILD_DIR_IN_CONTAINER="${BUILD_DIR_IN_CONTAINER}" \
    -e QMAKE_CONFIG_ARGS="${QMAKE_CONFIG_ARGS}" \
    -v "${REPO_ROOT}:/workspace" \
    -v "${REPO_ROOT}:${REPO_ROOT}" \
    -w /workspace \
    "${IMAGE_TAG}" \
    /bin/bash -lc '
        set -euo pipefail
        rm -rf "${BUILD_DIR_IN_CONTAINER}"
        mkdir -p "${BUILD_DIR_IN_CONTAINER}"
        cd "${BUILD_DIR_IN_CONTAINER}"
        "${QT_PREFIX_IN_CONTAINER}/bin/qmake" ../qtwebkit-smoke.pro ${QMAKE_CONFIG_ARGS}
        make -j"${JOBS}"
        echo "built: /workspace/${BUILD_DIR_IN_CONTAINER}/qtwebkit-smoke"
    '
