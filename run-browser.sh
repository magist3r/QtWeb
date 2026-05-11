#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

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
    if [[ "${BUILD_TYPE}" == "debug" ]]; then
        "${SCRIPT_DIR}/build-browser-docker.sh" --debug
    else
        "${SCRIPT_DIR}/build-browser-docker.sh"
    fi
elif [[ ! -x "${BINARY}" ]]; then
    echo "error: browser binary not found: ${BINARY}" >&2
    echo "hint: rerun with --rebuild" >&2
    exit 1
fi

if [[ "${RUN_WITH_GDB}" -eq 1 ]]; then
    exec gdb -ex run --args "${BINARY}" "$@"
else
    exec "${BINARY}" "$@"
fi
