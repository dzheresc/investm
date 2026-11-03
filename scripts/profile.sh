#!/usr/bin/env bash
# Runs the valgrind suite over the benchmark driver and the chapter examples.
#
#   scripts/profile.sh            # everything
#   scripts/profile.sh memcheck   # one tool: memcheck|callgrind|cachegrind|
#                                 #           massif|dhat
#   FILTER=ch08 scripts/profile.sh callgrind   # narrow the benchmark cases
#
# Builds build/profile (RelWithDebInfo: optimized, with symbols) and writes
# every report to build/profile/reports. Helgrind and DRD are not run: the
# code is single threaded, so there are no data races to look for.
set -euo pipefail
cd "$(dirname "$0")/.."

TOOLS=("${@:-memcheck callgrind cachegrind massif dhat}")
TOOLS=(${TOOLS[*]})
BUILD=build/profile
OUT="$BUILD/reports"
FILTER="${FILTER:-}"

cmake -S . -B "$BUILD" -DCMAKE_BUILD_TYPE=RelWithDebInfo >/dev/null
cmake --build "$BUILD" -j "$(nproc)" >/dev/null
mkdir -p "$OUT"

BENCH="$BUILD/cpp/bench/investm_bench"
# One pass per case is enough under instrumentation.
BENCH_ARGS=(--quick)
[[ -n "$FILTER" ]] && BENCH_ARGS+=("--filter=$FILTER")

run_memcheck() {
  echo "== memcheck: examples and benchmark (any error fails the run)"
  local failed=0
  for exe in "$BUILD"/cpp/ch*/ch*_examples "$BENCH"; do
    local name
    name=$(basename "$exe")
    local args=()
    [[ "$exe" == "$BENCH" ]] && args=("${BENCH_ARGS[@]}")
    if ! valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
        --errors-for-leak-kinds=all --error-exitcode=1 \
        --log-file="$OUT/memcheck.$name.log" "$exe" "${args[@]}" >/dev/null; then
      echo "   FAIL $name (see $OUT/memcheck.$name.log)"
      failed=1
    fi
  done
  grep -h "ERROR SUMMARY" "$OUT"/memcheck.*.log | sed "s/^==[0-9]*== //" |
    sort | uniq -c
  return "$failed"
}

run_callgrind() {
  echo "== callgrind: instruction counts per function"
  valgrind --tool=callgrind --callgrind-out-file="$OUT/callgrind.out" \
    "$BENCH" "${BENCH_ARGS[@]}" >/dev/null 2>"$OUT/callgrind.log"
  callgrind_annotate --inclusive=yes "$OUT/callgrind.out" \
    >"$OUT/callgrind.inclusive.txt"
  callgrind_annotate "$OUT/callgrind.out" >"$OUT/callgrind.self.txt"
  sed -n '1,40p' "$OUT/callgrind.self.txt" | grep -E 'PROGRAM TOTALS|^ *[0-9,]+ ' |
    head -15
}

run_cachegrind() {
  echo "== cachegrind: cache and branch simulation"
  valgrind --tool=cachegrind --cache-sim=yes --branch-sim=yes \
    --cachegrind-out-file="$OUT/cachegrind.out" \
    "$BENCH" "${BENCH_ARGS[@]}" >/dev/null 2>"$OUT/cachegrind.log"
  cg_annotate "$OUT/cachegrind.out" >"$OUT/cachegrind.txt"
  grep -E '(I|D|LL)[[:alnum:] ]*(refs|misses|miss rate)|Mispred' \
    "$OUT/cachegrind.log" | sed 's/^==[0-9]*== //'
}

run_massif() {
  echo "== massif: heap profile"
  valgrind --tool=massif --massif-out-file="$OUT/massif.out" \
    "$BENCH" "${BENCH_ARGS[@]}" >/dev/null 2>"$OUT/massif.log"
  ms_print "$OUT/massif.out" >"$OUT/massif.txt"
  grep -m1 -A1 'MB\|KB\|^    [0-9.]*\^' "$OUT/massif.txt" | head -3 || true
}

run_dhat() {
  echo "== dhat: allocation counts and lifetimes"
  valgrind --tool=dhat --dhat-out-file="$OUT/dhat.json" \
    "$BENCH" "${BENCH_ARGS[@]}" >/dev/null 2>"$OUT/dhat.log"
  grep -E 'Total:|At t-gmax:|At t-end:|Reads:|Writes:' "$OUT/dhat.log" |
    sed 's/^==[0-9]*== //'
  echo "   view $OUT/dhat.json at https://nnethercote.github.io/dh_view/dh_view.html"
}

status=0
for tool in "${TOOLS[@]}"; do
  "run_$tool" || status=1
done
echo "Reports in $OUT"
exit "$status"
