#!/usr/bin/env bash
set -euo pipefail

cd /workspace

GET_SRC_ARGS=()
for arg in "$@"; do
    case "$arg" in
        --use-qtwebkit-23)
            GET_SRC_ARGS+=("$arg")
            ;;
    esac
done

if ((${#GET_SRC_ARGS[@]})); then
    ./get-src.sh "${GET_SRC_ARGS[@]}"
else
    ./get-src.sh
fi
./build.sh "$@"
