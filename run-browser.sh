#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_SCRIPT="${SCRIPT_DIR}/build-browser-docker.sh"

BUILD_TYPE="release"
RUN_WITH_GDB=0
RUN_BUILD=0
while [[ $# -gt 0 ]]; do
    case "$1" in
        --rebuild)
            RUN_BUILD=1
            shift
            ;;
        --debug)
            BUILD_TYPE="debug"
            shift
            ;;
        --gdb)
            RUN_WITH_GDB=1
            BUILD_TYPE="debug"
            shift
            ;;
        --)
            shift
            break
            ;;
        *)
            break
            ;;
    esac
done

case "${BUILD_TYPE}" in
    release|debug) ;;
    *)
        echo "error: BUILD_TYPE must be release or debug, got: ${BUILD_TYPE}" >&2
        exit 1
        ;;
esac

BINARY="${SCRIPT_DIR}/build-docker-${BUILD_TYPE}/QtWeb"
if [[ "${RUN_BUILD}" -eq 1 ]]; then
    BUILD_ARGS=()
    if [[ "${BUILD_TYPE}" == "debug" ]]; then
        BUILD_ARGS+=(--debug)
    fi
    "${BUILD_SCRIPT}" "${BUILD_ARGS[@]}"
elif [[ ! -x "${BINARY}" ]]; then
    echo "error: browser binary not found: ${BINARY}" >&2
    echo "hint: rerun with --rebuild" >&2
    exit 1
fi

RUN_BINARY="${BINARY}"
if [[ "${RUN_WITH_GDB}" -eq 1 ]]; then
    CMD=(gdb -ex run --args "${RUN_BINARY}")
else
    CMD=("${RUN_BINARY}")
fi

exec "${CMD[@]}" "$@"
