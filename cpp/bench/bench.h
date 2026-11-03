// A small, dependency-free micro-benchmark harness.
//
// Each case runs its body repeatedly: first to calibrate an iteration count
// that fills roughly `min_time` seconds, then for `repetitions` timed
// batches. The report shows the median and minimum time per call. In
// `--quick` mode every case runs exactly once (for valgrind, which slows
// code down 20-100x).

#ifndef CPP_BENCH_BENCH_H_
#define CPP_BENCH_BENCH_H_

#include <functional>
#include <string>
#include <vector>

namespace investm::bench {

// Keeps the optimizer from discarding a computed value.
void DoNotOptimize(double value);

struct Case {
  std::string name;
  std::function<void()> body;
};

struct Options {
  double min_time = 0.2;  // Seconds per timed batch.
  int repetitions = 5;
  bool quick = false;
  std::string filter;     // Substring; empty runs everything.
  std::string csv_path;   // Optional machine-readable output.
};

Options ParseOptions(int argc, char** argv);

// Runs the cases and prints a table; returns the process exit code.
int RunAll(const std::vector<Case>& cases, const Options& options);

}  // namespace investm::bench

#endif  // CPP_BENCH_BENCH_H_
