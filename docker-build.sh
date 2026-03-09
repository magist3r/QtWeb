#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IMAGE_NAME="${IMAGE_NAME:-qtweb-qt4-builder}"
ARTIFACT_DIR="$ROOT_DIR/artifacts"
ARTIFACT_PATH="$ARTIFACT_DIR/QtWeb"

docker build -t "$IMAGE_NAME" -f "$ROOT_DIR/Dockerfile.qt4-build" "$ROOT_DIR"

DOCKER_ARGS=(
    --rm
    --user "$(id -u):$(id -g)"
    -e HOME=/tmp/qtweb-home
    -v "$ROOT_DIR:/workspace"
    -w /workspace
)

if [[ -t 0 && -t 1 ]]; then
    DOCKER_ARGS+=(-it)
else
    DOCKER_ARGS+=(-i)
fi

docker run "${DOCKER_ARGS[@]}" "$IMAGE_NAME" /workspace/docker-build-inner.sh "$@"

mkdir -p "$ARTIFACT_DIR"
install -m 0755 "$ROOT_DIR/build/src/QtWeb" "$ARTIFACT_PATH"

printf 'Extracted artifact: %s\n' "$ARTIFACT_PATH"
