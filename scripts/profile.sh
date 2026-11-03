#!/usr/bin/env bash
# Runs the valgrind suite over the benchmark driver and the chapter examples.
#
#   scripts/profile.sh            # everything
#   scripts/profile.sh memcheck   # one tool: memcheck|callgrind|cachegrind|
#                                 #   massif|dhat|chapters|cases
#   FILTER=ch08 scripts/profile.sh callgrind   # narrow the benchmark cases
#   BASELINE=old/reports/cases scripts/profile.sh cases   # compare two runs
#
# `chapters` runs callgrind over every chapter's example and test binary and
# lists instruction counts, so a slow chapter stands out. `cases` profiles
# each benchmark case on its own (callgrind with cache and branch
# simulation, one dump per case, setup excluded) and tabulates it with
# scripts/callgrind_cases.py.
#
# The simulated caches are pinned (CACHE_ARGS) to a common desktop layout,
# so numbers from this sandbox, CI and a Docker Desktop run line up. With
# valgrind's default, the geometry is read from the host CPU.
#
# Builds build/profile (RelWithDebInfo: optimized, with symbols) and writes
# every report to build/profile/reports. Helgrind and DRD are not run: the
# code is single threaded, so there are no data races to look for.
set -euo pipefail
cd "$(dirname "$0")/.."

TOOLS=("${@:-memcheck callgrind cachegrind massif dhat chapters cases}")
TOOLS=(${TOOLS[*]})
BUILD=build/profile
OUT="$BUILD/reports"
FILTER="${FILTER:-}"
BASELINE="${BASELINE:-}"
CACHE_ARGS=(--I1=32768,8,64 --D1=32768,8,64 --LL=8388608,16,64)

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
    "$BENCH" "${BENCH_ARGS[@]}" --no-dumps >/dev/null 2>"$OUT/callgrind.log"
  callgrind_annotate --inclusive=yes "$OUT/callgrind.out" \
    >"$OUT/callgrind.inclusive.txt"
  callgrind_annotate "$OUT/callgrind.out" >"$OUT/callgrind.self.txt"
  sed -n '1,40p' "$OUT/callgrind.self.txt" | grep -E 'PROGRAM TOTALS|^ *[0-9,]+ ' |
    head -15
}

run_cachegrind() {
  echo "== cachegrind: cache and branch simulation"
  valgrind --tool=cachegrind --cache-sim=yes --branch-sim=yes "${CACHE_ARGS[@]}" \
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

run_chapters() {
  echo "== chapters: callgrind instruction counts for every example and test"
  local dir="$OUT/chapters"
  rm -rf "$dir"
  mkdir -p "$dir"
  local exes=("$BUILD"/cpp/ch*/ch*_examples "$BUILD"/cpp/*/*_test)
  printf '%s\n' "${exes[@]}" | xargs -P "$(nproc)" -I{} sh -c \
    'valgrind --tool=callgrind --callgrind-out-file="$1/$(basename "$2").out" \
       "$2" >/dev/null 2>"$1/$(basename "$2").log"' _ "$dir" {}
  # About 2 M instructions of every run are the dynamic loader and
  # iostream start-up, so only counts well above that are the chapter's.
  for f in "$dir"/*.out; do
    callgrind_annotate "$f" |
      awk -v n="$(basename "$f" .out)" '/PROGRAM TOTALS/ {
        gsub(",", "", $1); printf "%-16s %14d\n", n, $1 }'
  done | sort -k2 -n >"$dir/summary.txt"
  echo "   the 12 largest (all in $dir/summary.txt):"
  tail -12 "$dir/summary.txt"
}

run_cases() {
  echo "== cases: per-case callgrind with cache and branch simulation"
  local dir="$OUT/cases"
  rm -rf "$dir"
  mkdir -p "$dir"
  valgrind --tool=callgrind --cache-sim=yes --branch-sim=yes "${CACHE_ARGS[@]}" \
    --callgrind-out-file="$dir/callgrind.out" \
    "$BENCH" "${BENCH_ARGS[@]}" >/dev/null 2>"$dir/callgrind.log"
  python3 scripts/callgrind_cases.py "$dir" ${BASELINE:+"$BASELINE"} \
    --csv="$dir/summary.csv" | tee "$dir/summary.txt"
}

status=0
for tool in "${TOOLS[@]}"; do
  "run_$tool" || status=1
done
echo "Reports in $OUT"
exit "$status"
