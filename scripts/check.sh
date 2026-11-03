#!/usr/bin/env bash
# Runs the same checks as CI: formatting, lint, build, C++ and Python tests.
set -euo pipefail
cd "$(dirname "$0")/.."

echo "== clang-format"
find cpp -name '*.h' -o -name '*.cc' | xargs clang-format --dry-run --Werror

echo "== cpplint"
cpplint --quiet --recursive cpp

echo "== build"
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DINVESTM_WARNINGS_AS_ERRORS=ON
cmake --build build -j "$(nproc)"

echo "== ctest"
ctest --test-dir build --output-on-failure -j "$(nproc)"

echo "== python"
cd python
yapf --diff --recursive --style=google investm
python3 -m pytest -q
pylint --rcfile=.pylintrc investm
