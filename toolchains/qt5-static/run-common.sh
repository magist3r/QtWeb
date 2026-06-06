#!/usr/bin/env bash
BUILD_TYPE="release"
RUN_WITH_GDB=0
RUN_BUILD=0
RUN_CHECK=0
RUN_RUNTIME_CHECK=0
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
        --check)
            RUN_CHECK=1
            shift
            ;;
        --runtime-check)
            RUN_RUNTIME_CHECK=1
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

if [[ "${RUN_WITH_GDB}" -eq 1 && ( "${RUN_CHECK}" -eq 1 || "${RUN_RUNTIME_CHECK}" -eq 1 ) ]]; then
    echo "error: --gdb cannot be used with --check or --runtime-check" >&2
    exit 1
fi

if [[ "${RUN_CHECK}" -eq 1 && "${RUN_RUNTIME_CHECK}" -eq 1 ]]; then
    echo "error: --check cannot be used with --runtime-check" >&2
    exit 1
fi

BINARY="${BINARY_ROOT}/build-docker-${BUILD_TYPE}/${BINARY_NAME}"
if [[ "${RUN_BUILD}" -eq 1 ]]; then
    if [[ "${BUILD_TYPE}" == "debug" ]]; then
        "${BUILD_SCRIPT}" --debug
    else
        "${BUILD_SCRIPT}"
    fi
elif [[ ! -x "${BINARY}" ]]; then
    echo "error: ${APP_LABEL} binary not found: ${BINARY}" >&2
    echo "hint: rerun with --rebuild" >&2
    exit 1
fi

if [[ "${RUN_CHECK}" -eq 1 ]]; then
    "${REPO_ROOT}/toolchains/qt5-static/check-shared-deps.py" "${BINARY}"
elif [[ "${RUN_RUNTIME_CHECK}" -eq 1 ]]; then
    set +e
    xvfb-run -a -s "-screen 0 1280x1024x24" timeout 20s "${BINARY}" "$@"
    status="$?"
    set -e
    if [[ "${status}" -ne 0 && "${status}" -ne 124 ]]; then
        exit "${status}"
    fi
elif [[ "${RUN_WITH_GDB}" -eq 1 ]]; then
    exec gdb -ex run --args "${BINARY}" "$@"
else
    exec "${BINARY}" "$@"
fi
