#!/usr/bin/env bash
set -euo pipefail

rm -rf "$BUILD_DIR_IN_CONTAINER"
mkdir -p "$BUILD_DIR_IN_CONTAINER"
cd "$BUILD_DIR_IN_CONTAINER"

if [[ "$BUILD_TYPE" == "release" ]]; then
    "${QT_PREFIX_IN_CONTAINER}/bin/qmake" ../qtwebkit-smoke.pro CONFIG+=release CONFIG-=debug
else
    "${QT_PREFIX_IN_CONTAINER}/bin/qmake" ../qtwebkit-smoke.pro CONFIG+=debug CONFIG-=release
fi

make -j"$JOBS"
cp /etc/ssl/certs/ca-certificates.crt ./ca-certificates.crt
echo "built: /workspace/${BUILD_DIR_IN_CONTAINER}/qtwebkit-smoke"
