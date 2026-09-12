#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${ROOT}/build"

cmake -S "$ROOT" -B "$BUILD_DIR" -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13 -DBUILD_TESTING=OFF
cmake --build "$BUILD_DIR" --target lgnd
sudo install -d /usr/local/bin
sudo install -m 755 "$BUILD_DIR/lgnd" /usr/local/bin/lgnd

echo "Legend instalada em /usr/local/bin/lgnd"
echo "Teste com: lgnd --version"
