#!/usr/bin/env bash
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
    grep -F "warning:" "$build_log" > "$warnings_report" || :
}

rm -rf "$BUILD_DIR_IN_CONTAINER"
mkdir -p "$BUILD_DIR_IN_CONTAINER"
cd "$BUILD_DIR_IN_CONTAINER"

if [[ "$BUILD_TYPE" == "release" ]]; then
    "${QT_PREFIX_IN_CONTAINER}/bin/qmake" ../src/QtWeb.pro CONFIG+=release CONFIG-=debug
else
    "${QT_PREFIX_IN_CONTAINER}/bin/qmake" ../src/QtWeb.pro CONFIG+=debug CONFIG-=release
fi

build_log="${BUILD_DIR_IN_CONTAINER}/build.log"
gcc_warnings_report="${BUILD_DIR_IN_CONTAINER}/gcc-warnings.txt"

if [[ "$RUN_ANALYSIS" == "1" ]]; then
    require_tool bear
    require_tool clang-tidy
    clang_tidy_fixes="${BUILD_DIR_IN_CONTAINER}/clang-tidy-fixes.yaml"
    clang_tidy_extra_args=()

    bear make -j"$JOBS" 2>&1 | tee "$build_log"
    collect_gcc_warnings "$build_log" "$gcc_warnings_report"

    mapfile -t ANALYSIS_SOURCES < <(find "${REPO_ROOT}/src" -path "${REPO_ROOT}/src/qt" -prune -o -type f -name "*.cpp" -print | sort)
    [[ "${#ANALYSIS_SOURCES[@]}" -gt 0 ]] || fail "no analysis sources found under ${REPO_ROOT}/src"

    if [[ "$EXPORT_FIXES" == "1" ]]; then
        rm -f "$clang_tidy_fixes"
        clang_tidy_extra_args+=(-export-fixes="$clang_tidy_fixes")
    fi

    clang-tidy \
        -p "$BUILD_DIR_IN_CONTAINER" \
        --header-filter="(^|.*/)src/.*" \
        "${clang_tidy_extra_args[@]}" \
        "${ANALYSIS_SOURCES[@]}" \
        > "${BUILD_DIR_IN_CONTAINER}/clang-tidy.txt" 2>&1

    echo "built: ${BUILD_DIR_IN_CONTAINER}/QtWeb"
    echo "compile_commands: ${BUILD_DIR_IN_CONTAINER}/compile_commands.json"
    echo "build log: ${build_log}"
    echo "gcc warnings: ${gcc_warnings_report}"
    echo "clang-tidy report: ${BUILD_DIR_IN_CONTAINER}/clang-tidy.txt"
    if [[ "$EXPORT_FIXES" == "1" ]]; then
        echo "clang-tidy fixes: ${clang_tidy_fixes}"
    fi
else
    make -j"$JOBS" 2>&1 | tee "$build_log"
    collect_gcc_warnings "$build_log" "$gcc_warnings_report"
    echo "built: ${BUILD_DIR_IN_CONTAINER}/QtWeb"
    echo "build log: ${build_log}"
    echo "gcc warnings: ${gcc_warnings_report}"
fi
