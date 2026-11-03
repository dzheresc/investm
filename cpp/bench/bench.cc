#include "cpp/bench/bench.h"

#include <algorithm>
#include <chrono>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace investm::bench {
namespace {

volatile double g_sink = 0.0;

using Clock = std::chrono::steady_clock;

double SecondsFor(const std::function<void()>& body, std::int64_t iterations) {
  const auto start = Clock::now();
  for (std::int64_t i = 0; i < iterations; ++i) body();
  return std::chrono::duration<double>(Clock::now() - start).count();
}

std::string FormatTime(double seconds) {
  char buf[32];
  if (seconds < 1e-6) {
    std::snprintf(buf, sizeof(buf), "%9.1f ns", seconds * 1e9);
  } else if (seconds < 1e-3) {
    std::snprintf(buf, sizeof(buf), "%9.2f us", seconds * 1e6);
  } else {
    std::snprintf(buf, sizeof(buf), "%9.3f ms", seconds * 1e3);
  }
  return buf;
}

}  // namespace

void DoNotOptimize(double value) { g_sink = g_sink + value; }

Options ParseOptions(int argc, char** argv) {
  Options o;
  for (int i = 1; i < argc; ++i) {
    const char* a = argv[i];
    if (std::strcmp(a, "--quick") == 0) {
      o.quick = true;
    } else if (std::strncmp(a, "--filter=", 9) == 0) {
      o.filter = a + 9;
    } else if (std::strncmp(a, "--min_time=", 11) == 0) {
      o.min_time = std::atof(a + 11);
    } else if (std::strncmp(a, "--repetitions=", 14) == 0) {
      o.repetitions = std::max(1, std::atoi(a + 14));
    } else if (std::strncmp(a, "--csv=", 6) == 0) {
      o.csv_path = a + 6;
    } else {
      std::fprintf(stderr,
                   "usage: %s [--quick] [--filter=substr] [--min_time=s] "
                   "[--repetitions=n] [--csv=path]\n",
                   argv[0]);
      std::exit(2);
    }
  }
  return o;
}

int RunAll(const std::vector<Case>& cases, const Options& options) {
  std::FILE* csv = nullptr;
  if (!options.csv_path.empty()) {
    csv = std::fopen(options.csv_path.c_str(), "w");
    if (csv == nullptr) {
      std::fprintf(stderr, "cannot write %s\n", options.csv_path.c_str());
      return 1;
    }
    std::fprintf(csv, "case,median_seconds,min_seconds,iterations\n");
  }
  std::printf("%-44s %13s %13s %10s\n", "case", "median/call", "min/call",
              "iters");
  for (const Case& c : cases) {
    if (!options.filter.empty() &&
        c.name.find(options.filter) == std::string::npos) {
      continue;
    }
    if (options.quick) {
      c.body();
      std::printf("%-44s %13s %13s %10d\n", c.name.c_str(), "-", "-", 1);
      continue;
    }
    std::int64_t iterations = 1;
    while (true) {
      const double t = SecondsFor(c.body, iterations);
      if (t >= options.min_time / 4 || iterations > (std::int64_t{1} << 30)) break;
      const double scale = t > 0 ? options.min_time / t : 100.0;
      iterations = std::max(iterations * 2,
                            static_cast<std::int64_t>(static_cast<double>(iterations) *
                                      std::min(scale, 100.0)));
    }
    std::vector<double> per_call;
    for (int r = 0; r < options.repetitions; ++r) {
      per_call.push_back(SecondsFor(c.body, iterations) /
                         static_cast<double>(iterations));
    }
    std::sort(per_call.begin(), per_call.end());
    const double median = per_call[per_call.size() / 2];
    std::printf("%-44s %13s %13s %10" PRId64 "\n", c.name.c_str(),
                FormatTime(median).c_str(), FormatTime(per_call.front()).c_str(),
                iterations);
    if (csv != nullptr) {
      std::fprintf(csv, "%s,%.6e,%.6e,%" PRId64 "\n", c.name.c_str(), median,
                   per_call.front(), iterations);
    }
  }
  if (csv != nullptr) std::fclose(csv);
  return 0;
}

}  // namespace investm::bench
