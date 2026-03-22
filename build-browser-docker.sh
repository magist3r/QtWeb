#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}" && pwd)"

fail() {
    echo "error: $*" >&2
    exit 1
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

IMAGE_TAG="${IMAGE_TAG:-qtweb-qt5-static-poc:5.5.1}"
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

case "$BUILD_TYPE" in
    release|debug)
        QT_PREFIX_IN_CONTAINER="${REPO_ROOT}/artifacts/qt5-static-5.5.1-${BUILD_TYPE}/install"
        BUILD_DIR="${REPO_ROOT}/build-docker-${BUILD_TYPE}"
        if [[ "$BUILD_TYPE" == "release" ]]; then
            QMAKE_CONFIG_ARGS="CONFIG+=release CONFIG-=debug"
        else
            QMAKE_CONFIG_ARGS="CONFIG+=debug CONFIG-=release"
        fi
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
    -e QMAKE_CONFIG_ARGS="${QMAKE_CONFIG_ARGS}" \
    -e EXPORT_FIXES="${EXPORT_FIXES}" \
    -v "${REPO_ROOT}:${REPO_ROOT}" \
    -e RUN_ANALYSIS="${RUN_ANALYSIS}" \
    -w "${REPO_ROOT}" \
    "${IMAGE_TAG}" \
    /bin/bash -lc '
        set -euo pipefail

        fail() {
            echo "error: $*" >&2
            exit 1
        }

        require_tool() {
            command -v "$1" >/dev/null 2>&1 || fail "missing tool in image: $1"
        }

        collect_gcc_warnings() {
            local build_log="$1"
            local warnings_report="$2"
            grep -F "warning:" "${build_log}" > "${warnings_report}" || :
        }

        rm -rf "${BUILD_DIR_IN_CONTAINER}"
        mkdir -p "${BUILD_DIR_IN_CONTAINER}"
        cd "${BUILD_DIR_IN_CONTAINER}"
        "${QT_PREFIX_IN_CONTAINER}/bin/qmake" ../src/QtWeb.pro ${QMAKE_CONFIG_ARGS}
        build_log="${BUILD_DIR_IN_CONTAINER}/build.log"
        gcc_warnings_report="${BUILD_DIR_IN_CONTAINER}/gcc-warnings.txt"

        if [[ "${RUN_ANALYSIS}" == "1" ]]; then
            require_tool bear
            require_tool clang-tidy
            clang_tidy_fixes="${BUILD_DIR_IN_CONTAINER}/clang-tidy-fixes.yaml"
            clang_tidy_extra_args=()

            bear make -j"${JOBS}" 2>&1 | tee "${build_log}"
            collect_gcc_warnings "${build_log}" "${gcc_warnings_report}"

            mapfile -t ANALYSIS_SOURCES < <(find "${REPO_ROOT}/src" -path "${REPO_ROOT}/src/qt" -prune -o -type f -name "*.cpp" -print | sort)
            [[ "${#ANALYSIS_SOURCES[@]}" -gt 0 ]] || fail "no analysis sources found under ${REPO_ROOT}/src"

            if [[ "${EXPORT_FIXES}" == "1" ]]; then
                rm -f "${clang_tidy_fixes}"
                clang_tidy_extra_args+=(-export-fixes="${clang_tidy_fixes}")
            fi

            if [[ "${#clang_tidy_extra_args[@]}" -gt 0 ]]; then
                clang-tidy \
                    -p "${BUILD_DIR_IN_CONTAINER}" \
                    --header-filter="(^|.*/)src/.*" \
                    "${clang_tidy_extra_args[@]}" \
                    "${ANALYSIS_SOURCES[@]}" \
                    > "${BUILD_DIR_IN_CONTAINER}/clang-tidy.txt" 2>&1
            else
                clang-tidy \
                    -p "${BUILD_DIR_IN_CONTAINER}" \
                    --header-filter="(^|.*/)src/.*" \
                    "${ANALYSIS_SOURCES[@]}" \
                    > "${BUILD_DIR_IN_CONTAINER}/clang-tidy.txt" 2>&1
            fi

            echo "built: ${BUILD_DIR_IN_CONTAINER}/QtWeb"
            echo "compile_commands: ${BUILD_DIR_IN_CONTAINER}/compile_commands.json"
            echo "build log: ${build_log}"
            echo "gcc warnings: ${gcc_warnings_report}"
            echo "clang-tidy report: ${BUILD_DIR_IN_CONTAINER}/clang-tidy.txt"
            if [[ "${EXPORT_FIXES}" == "1" ]]; then
                echo "clang-tidy fixes: ${clang_tidy_fixes}"
            fi
        else
            make -j"${JOBS}" 2>&1 | tee "${build_log}"
            collect_gcc_warnings "${build_log}" "${gcc_warnings_report}"
            echo "built: ${BUILD_DIR_IN_CONTAINER}/QtWeb"
            echo "build log: ${build_log}"
            echo "gcc warnings: ${gcc_warnings_report}"
        fi
    '
