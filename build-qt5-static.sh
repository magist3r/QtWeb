#!/usr/bin/env bash
set -euo pipefail

QT_VERSION="5.5.1"
DEFAULT_JOBS="$(nproc 2>/dev/null || echo 8)"
JOBS="${COMPILE_JOBS:-$DEFAULT_JOBS}"
OUTPUT_DIR=""
RUNTIME="auto"
CLEAN=false
BUILD_TYPE="release"
IMAGE_TAG="qtweb-qt5-static-poc:${QT_VERSION}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$SCRIPT_DIR"
LOCK_FILE="${REPO_ROOT}/toolchains/qt5-static/sources.lock"
DOCKERFILE="${REPO_ROOT}/toolchains/qt5-static/Dockerfile"
INNER_SCRIPT="/workspace/toolchains/qt5-static/build-inside-container.sh"

usage() {
    cat <<'EOF'
Usage: ./build-qt5-static.sh [options]

Options:
  --jobs <N>                 Parallel build jobs (default: nproc)
  --output-dir <path>        Output directory inside repo (default: artifacts/qt5-static-5.5.1-<release|debug>)
  --runtime <auto|podman|docker>
                             Container runtime selector (default: auto)
  --debug                    Build debug Qt libraries
  --clean                    Remove output directory before running
  --help                     Show this help message

Environment overrides:
  QT5_SRC_URL
  QT5_SRC_SHA256
  QT5_WEBKIT_SRC_URL
  QT5_WEBKIT_SHA256
  ICU_SRC_URL
  ICU_SRC_SHA256
EOF
}

fail() {
    echo "error: $*" >&2
    exit 1
}

abs_path() {
    case "$1" in
        /*) printf '%s\n' "$1" ;;
        *) printf '%s\n' "${REPO_ROOT}/$1" ;;
    esac
}

basename_from_url() {
    local url="$1"
    url="${url%%\?*}"
    printf '%s\n' "${url##*/}"
}

source_file_name() {
    local source_url="$1"
    local lock_url="$2"
    local lock_file="$3"

    if [[ "$source_url" == "$lock_url" && -n "$lock_file" ]]; then
        printf '%s\n' "$lock_file"
        return
    fi

    basename_from_url "$source_url"
}

ensure_output_in_repo() {
    local output_path="$1"
    local error_message="$2"

    case "${output_path}/" in
        "${REPO_ABS}/"*) ;;
        *) fail "$error_message" ;;
    esac

    if [[ "$output_path" == "$REPO_ABS" ]]; then
        fail "--output-dir cannot be repository root"
    fi
}

pick_runtime() {
    if [[ "$RUNTIME" != "auto" && "$RUNTIME" != "podman" && "$RUNTIME" != "docker" ]]; then
        fail "--runtime must be one of: auto, podman, docker"
    fi

    if [[ "$RUNTIME" == "auto" ]]; then
        if command -v podman >/dev/null 2>&1; then
            echo "podman"
            return
        fi
        if command -v docker >/dev/null 2>&1; then
            echo "docker"
            return
        fi
        fail "neither podman nor docker is installed"
    fi

    if ! command -v "$RUNTIME" >/dev/null 2>&1; then
        fail "requested runtime '$RUNTIME' is not installed"
    fi
    echo "$RUNTIME"
}

download_file() {
    local url="$1"
    local dst="$2"

    if [[ -f "$dst" ]]; then
        echo "reusing archive: $dst"
        return
    fi

    echo "downloading: $url"
    if command -v curl >/dev/null 2>&1; then
        curl -fL --retry 3 --retry-delay 2 -o "$dst" "$url"
        return
    fi
    if command -v wget >/dev/null 2>&1; then
        wget -O "$dst" "$url"
        return
    fi

    fail "neither curl nor wget is installed"
}

verify_checksum() {
    local file_path="$1"
    local sha256_expected="$2"
    local md5_expected="$3"
    local name="$4"
    local actual

    if [[ -n "$sha256_expected" ]]; then
        actual="$(sha256sum "$file_path" | awk '{print $1}')"
        if [[ "$actual" != "$sha256_expected" ]]; then
            fail "$name sha256 mismatch: expected $sha256_expected got $actual"
        fi
        echo "$name sha256 verified"
        return
    fi

    if [[ -n "$md5_expected" ]]; then
        echo "warning: $name uses md5 fallback because sha256 is not pinned" >&2
        actual="$(md5sum "$file_path" | awk '{print $1}')"
        if [[ "$actual" != "$md5_expected" ]]; then
            fail "$name md5 mismatch: expected $md5_expected got $actual"
        fi
        echo "$name md5 verified"
        return
    fi

    fail "$name has no checksum configured"
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --jobs)
            [[ $# -ge 2 ]] || fail "--jobs requires a value"
            JOBS="$2"
            shift 2
            ;;
        --output-dir)
            [[ $# -ge 2 ]] || fail "--output-dir requires a value"
            OUTPUT_DIR="$2"
            shift 2
            ;;
        --runtime)
            [[ $# -ge 2 ]] || fail "--runtime requires a value"
            RUNTIME="$2"
            shift 2
            ;;
        --debug)
            BUILD_TYPE="debug"
            shift
            ;;
        --clean)
            CLEAN=true
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

if [[ "$BUILD_TYPE" != "release" && "$BUILD_TYPE" != "debug" ]]; then
    fail "unsupported build type: $BUILD_TYPE"
fi

if [[ -z "$OUTPUT_DIR" ]]; then
    OUTPUT_DIR="artifacts/qt5-static-${QT_VERSION}-${BUILD_TYPE}"
fi

[[ -f "$LOCK_FILE" ]] || fail "missing lock file: $LOCK_FILE"
[[ -f "$DOCKERFILE" ]] || fail "missing Dockerfile: $DOCKERFILE"
source "$LOCK_FILE"

[[ "${LOCK_QT_VERSION:-}" == "$QT_VERSION" ]] || fail "lock file qt version mismatch"

QT5_SRC_URL="${QT5_SRC_URL:-${LOCK_QT5_SRC_URL}}"
QT5_SRC_SHA256="${QT5_SRC_SHA256:-${LOCK_QT5_SRC_SHA256:-}}"
QT5_SRC_MD5="${LOCK_QT5_SRC_MD5:-}"

QT5_WEBKIT_SRC_URL="${QT5_WEBKIT_SRC_URL:-${LOCK_QT5_WEBKIT_SRC_URL}}"
QT5_WEBKIT_SHA256="${QT5_WEBKIT_SHA256:-${LOCK_QT5_WEBKIT_SHA256:-}}"
QT5_WEBKIT_MD5="${LOCK_QT5_WEBKIT_MD5:-}"

ICU_SRC_URL="${ICU_SRC_URL:-${LOCK_ICU_SRC_URL}}"
ICU_SRC_SHA256="${ICU_SRC_SHA256:-${LOCK_ICU_SRC_SHA256:-}}"
ICU_SRC_MD5="${LOCK_ICU_SRC_MD5:-}"

if [[ -z "$QT5_SRC_URL" || -z "$QT5_WEBKIT_SRC_URL" || -z "$ICU_SRC_URL" ]]; then
    fail "source URLs are empty in lock file"
fi

QT5_SRC_FILE="$(source_file_name "$QT5_SRC_URL" "$LOCK_QT5_SRC_URL" "${LOCK_QT5_SRC_FILE:-}")"
QT5_WEBKIT_FILE="$(source_file_name "$QT5_WEBKIT_SRC_URL" "$LOCK_QT5_WEBKIT_SRC_URL" "${LOCK_QT5_WEBKIT_FILE:-}")"
ICU_SRC_FILE="$(source_file_name "$ICU_SRC_URL" "$LOCK_ICU_SRC_URL" "${LOCK_ICU_SRC_FILE:-}")"

REPO_ABS="$(cd "$REPO_ROOT" && pwd -P)"
OUTPUT_RAW="$(abs_path "$OUTPUT_DIR")"

[[ "$JOBS" =~ ^[1-9][0-9]*$ ]] || fail "--jobs must be a positive integer"

ensure_output_in_repo "$OUTPUT_RAW" "--output-dir must be inside repo: $REPO_ABS"

if $CLEAN && [[ -d "$OUTPUT_RAW" ]]; then
    rm -rf \
        "${OUTPUT_RAW}/build" \
        "${OUTPUT_RAW}/install" \
        "${OUTPUT_RAW}/icu-static" \
        "${OUTPUT_RAW}/xkb-config-root" \
        "${OUTPUT_RAW}/logs" \
        "${OUTPUT_RAW}/build-manifest.txt"
fi

mkdir -p "$OUTPUT_RAW"
OUTPUT_ABS="$(cd "$OUTPUT_RAW" && pwd -P)"

ensure_output_in_repo "$OUTPUT_ABS" "--output-dir resolves outside repo: $OUTPUT_ABS"

SRC_CACHE_DIR="${OUTPUT_ABS}/src-cache"
mkdir -p "$SRC_CACHE_DIR" "${OUTPUT_ABS}/logs" "${OUTPUT_ABS}/build"

QT5_SRC_ARCHIVE="${SRC_CACHE_DIR}/${QT5_SRC_FILE}"
QT5_WEBKIT_ARCHIVE="${SRC_CACHE_DIR}/${QT5_WEBKIT_FILE}"
ICU_SRC_ARCHIVE="${SRC_CACHE_DIR}/${ICU_SRC_FILE}"

download_file "$QT5_SRC_URL" "$QT5_SRC_ARCHIVE"
verify_checksum "$QT5_SRC_ARCHIVE" "$QT5_SRC_SHA256" "$QT5_SRC_MD5" "$QT5_SRC_FILE"

download_file "$QT5_WEBKIT_SRC_URL" "$QT5_WEBKIT_ARCHIVE"
verify_checksum "$QT5_WEBKIT_ARCHIVE" "$QT5_WEBKIT_SHA256" "$QT5_WEBKIT_MD5" "$QT5_WEBKIT_FILE"

download_file "$ICU_SRC_URL" "$ICU_SRC_ARCHIVE"
verify_checksum "$ICU_SRC_ARCHIVE" "$ICU_SRC_SHA256" "$ICU_SRC_MD5" "$ICU_SRC_FILE"

CONTAINER_RUNTIME="$(pick_runtime)"
echo "using container runtime: $CONTAINER_RUNTIME"

"$CONTAINER_RUNTIME" build -f "$DOCKERFILE" -t "$IMAGE_TAG" "$REPO_ROOT"

"$CONTAINER_RUNTIME" run --rm \
    --user "$(id -u):$(id -g)" \
    -e JOBS="$JOBS" \
    -e CLEAN="$CLEAN" \
    -e BUILD_TYPE="$BUILD_TYPE" \
    -e QT_VERSION="$QT_VERSION" \
    -e OUTPUT_DIR="${OUTPUT_ABS}" \
    -e QT_SRC_ARCHIVE="${QT5_SRC_ARCHIVE}" \
    -e QTWEBKIT_ARCHIVE="${QT5_WEBKIT_ARCHIVE}" \
    -e ICU_SRC_ARCHIVE="${ICU_SRC_ARCHIVE}" \
    -e QT_SRC_URL="$QT5_SRC_URL" \
    -e QTWEBKIT_URL="$QT5_WEBKIT_SRC_URL" \
    -e ICU_SRC_URL="$ICU_SRC_URL" \
    -e QT_SRC_SHA256="$QT5_SRC_SHA256" \
    -e QTWEBKIT_SHA256="$QT5_WEBKIT_SHA256" \
    -e ICU_SRC_SHA256="$ICU_SRC_SHA256" \
    -e QT_SRC_MD5="$QT5_SRC_MD5" \
    -e QTWEBKIT_MD5="$QT5_WEBKIT_MD5" \
    -e ICU_SRC_MD5="$ICU_SRC_MD5" \
    -v "${REPO_ROOT}:/workspace" \
    -v "${REPO_ROOT}:${REPO_ROOT}" \
    -w /workspace \
    "$IMAGE_TAG" \
    "$INNER_SCRIPT"

echo "static Qt5 POC completed: ${OUTPUT_ABS}"
