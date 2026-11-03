#!/usr/bin/env bash
# Runs the open-source comparison (docs/comparison.md): investm against
# QuantLib in C++, and against QuantLib, numpy-financial, SciPy, statsmodels,
# PyPortfolioOpt, cvxpy, arch and vollib in Python.
#
#   scripts/compare.sh              # both; results in docs/perf/compare-*.json
#   scripts/compare.sh cpp          # only the C++ QuantLib benchmarks
#   scripts/compare.sh python       # only the Python harness
#   QUICK=1 scripts/compare.sh      # one call per case (smoke test)
#
# Needs (Ubuntu): libquantlib0-dev libbenchmark-dev, and
#   pip install -r scripts/compare/requirements.txt
set -euo pipefail
cd "$(dirname "$0")/.."

parts=("$@")
[[ ${#parts[@]} -eq 0 ]] && parts=(cpp python)
out_dir=${OUT_DIR:-docs/perf}
mkdir -p "$out_dir"
export OMP_NUM_THREADS=1 OPENBLAS_NUM_THREADS=1 MKL_NUM_THREADS=1

for part in "${parts[@]}"; do
  case "$part" in
    cpp)
      cmake -S . -B build/compare -DCMAKE_BUILD_TYPE=Release \
        -DINVESTM_BUILD_TESTS=OFF -DINVESTM_BUILD_EXAMPLES=OFF \
        -DINVESTM_BUILD_COMPARE=ON
      cmake --build build/compare -j "$(nproc)" \
        --target investm_compare_quantlib
      if [[ -n ${QUICK:-} ]]; then
        build/compare/cpp/compare/investm_compare_quantlib \
          --benchmark_min_time=1x
      else
        build/compare/cpp/compare/investm_compare_quantlib \
          --benchmark_repetitions=5 --benchmark_report_aggregates_only=true \
          --benchmark_out="$out_dir/compare-cpp.json" \
          --benchmark_out_format=json
      fi
      ;;
    python)
      if [[ -n ${QUICK:-} ]]; then
        python3 scripts/compare/compare_python.py --quick
      else
        python3 scripts/compare/compare_python.py \
          --out "$out_dir/compare-python.json"
      fi
      ;;
    *)
      echo "unknown part: $part (expected cpp or python)" >&2
      exit 2
      ;;
  esac
done
