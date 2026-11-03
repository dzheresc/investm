#include "cpp/core/table.h"

#include <cstdio>
#include <string>
#include <utility>
#include <vector>

namespace investm {

void PrintHeading(std::string_view title) {
  std::printf("\n=== %.*s ===\n", static_cast<int>(title.size()), title.data());
}

void PrintValue(std::string_view label, double value, std::string_view unit) {
  std::printf("  %-46.*s %14.6f %.*s\n", static_cast<int>(label.size()),
              label.data(), value, static_cast<int>(unit.size()), unit.data());
}

void PrintPercent(std::string_view label, double fraction) {
  std::printf("  %-46.*s %13.4f %%\n", static_cast<int>(label.size()),
              label.data(), 100.0 * fraction);
}

Table::Table(std::vector<std::string> header, int width)
    : header_(std::move(header)), width_(width) {}

void Table::AddRow(const std::vector<double>& values, int precision) {
  std::vector<std::string> cells;
  cells.reserve(values.size());
  for (double v : values) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.*f", precision, v);
    cells.emplace_back(buf);
  }
  rows_.push_back(std::move(cells));
}

void Table::AddRow(std::vector<std::string> cells) {
  rows_.push_back(std::move(cells));
}

void Table::Print() const {
  PrintCells(header_);
  const std::string rule(header_.size() * static_cast<std::size_t>(width_ + 1),
                         '-');
  std::printf("  %s\n", rule.c_str());
  for (const auto& row : rows_) PrintCells(row);
}

void Table::PrintCells(const std::vector<std::string>& cells) const {
  std::printf(" ");
  for (const auto& c : cells) std::printf(" %*s", width_, c.c_str());
  std::printf("\n");
}

}  // namespace investm
