#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd "$(dirname "$0")/.." && pwd)"
: "${QT_ROOT:?Set QT_ROOT to your Qt 6.8+ SDK directory}"
cmake -S "$project_root" -B "$project_root/build" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON \
  "-DCMAKE_PREFIX_PATH=$QT_ROOT;$project_root/.deps/codecs"
cmake --build "$project_root/build" --parallel 4
export PATH="$QT_ROOT/bin:$PATH"
export LD_LIBRARY_PATH="$QT_ROOT/lib:$project_root/.deps/codecs/lib:${LD_LIBRARY_PATH:-}"
QT_QPA_PLATFORM=offscreen ctest --test-dir "$project_root/build" --output-on-failure
cmake --install "$project_root/build" --prefix "$project_root/release"
