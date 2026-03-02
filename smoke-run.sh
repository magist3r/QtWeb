#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SMOKE_DIR="${SCRIPT_DIR}/smoke-tests/qtwebkit-smoke"

BUILD_TYPE="release"
RUN_WITH_GDB=0
while [[ $# -gt 0 ]]; do
    case "$1" in
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

BINARY="${SMOKE_DIR}/build-${BUILD_TYPE}/qtwebkit-smoke"

CMD=("${BINARY}")
if [[ "${RUN_WITH_GDB}" -eq 1 ]]; then
    CMD=(gdb -ex run --args "${BINARY}")
fi

exec "${CMD[@]}" "$@"
