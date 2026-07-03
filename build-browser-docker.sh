#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}" && pwd)"

fail() {
    echo "error: $*" >&2
    exit 1
}

# shellcheck disable=SC1090
source "${REPO_ROOT}/toolchains/qt5-static/common.sh"

SCRIPT_NAME="$(basename "$0")"

report_run_duration() {
    local exit_code="$1"
    local outcome="failed"

    if [[ "$exit_code" -eq 0 ]]; then
        outcome="completed"
    fi

    echo "${SCRIPT_NAME} ${outcome} in $(format_duration "${SECONDS}")"
}

usage() {
    cat <<'EOF'
Usage: ./build-browser-docker.sh [options]

Options:
  --debug         Build the debug browser variant
  --analyze       Build inside Docker and run clang-tidy
  --analyze-only  Alias for --analyze
  --export-fixes  Export YAML fix suggestions for clang-tidy
  --help          Show this help message
EOF
}

IMAGE_TAG="${IMAGE_TAG:-$QT5_STATIC_IMAGE_TAG}"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"
BUILD_TYPE="release"
RUN_ANALYSIS=0
EXPORT_FIXES=0
BUILD_DIR=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --debug)
            BUILD_TYPE="debug"
            shift
            ;;
        --analyze|--analyze-only)
            RUN_ANALYSIS=1
            shift
            ;;
        --export-fixes)
            RUN_ANALYSIS=1
            EXPORT_FIXES=1
            shift
            ;;
        --help)
            usage
            exit 0
            ;;
        *)
            fail "unknown argument: $1"
            ;;
    esac
done

SECONDS=0
trap 'report_run_duration "$?"' EXIT

case "$BUILD_TYPE" in
    release|debug)
        QT_PREFIX_IN_CONTAINER="${REPO_ROOT}/artifacts/qt5-static-${QT_VERSION}-${BUILD_TYPE}/install"
        BUILD_DIR="${REPO_ROOT}/build-docker-${BUILD_TYPE}"
        ;;
    *)
        fail "BUILD_TYPE must be release or debug, got: ${BUILD_TYPE}"
        ;;
esac

docker run --rm \
    -u "$(id -u):$(id -g)" \
    -e JOBS="${JOBS}" \
    -e REPO_ROOT="${REPO_ROOT}" \
    -e QT_PREFIX_IN_CONTAINER="${QT_PREFIX_IN_CONTAINER}" \
    -e BUILD_DIR_IN_CONTAINER="${BUILD_DIR}" \
    -e BUILD_TYPE="${BUILD_TYPE}" \
    -e EXPORT_FIXES="${EXPORT_FIXES}" \
    -v "${REPO_ROOT}:${REPO_ROOT}" \
    -e RUN_ANALYSIS="${RUN_ANALYSIS}" \
    -w "${REPO_ROOT}" \
    "${IMAGE_TAG}" \
    "${REPO_ROOT}/toolchains/qt5-static/browser-build-entrypoint.sh"
