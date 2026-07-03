#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="${SCRIPT_DIR}"

APP_LABEL="browser"
BUILD_SCRIPT="${REPO_ROOT}/build-browser-docker.sh"
BINARY_ROOT="${REPO_ROOT}"
BINARY_NAME="QtWeb"

# shellcheck disable=SC1090
source "${REPO_ROOT}/toolchains/qt5-static/run-common.sh"
