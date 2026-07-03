#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="${SCRIPT_DIR}"
SMOKE_DIR="${REPO_ROOT}/smoke-tests/qtwebkit-smoke"

APP_LABEL="smoke"
BUILD_SCRIPT="${SMOKE_DIR}/smoke-build-docker.sh"
BINARY_ROOT="${SMOKE_DIR}"
BINARY_NAME="qtwebkit-smoke"

# shellcheck disable=SC1090
source "${REPO_ROOT}/toolchains/qt5-static/run-common.sh"
