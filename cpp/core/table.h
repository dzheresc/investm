// Console output helpers for the worked-example programs.

#ifndef CPP_CORE_TABLE_H_
#define CPP_CORE_TABLE_H_

#include <string>
#include <string_view>
#include <vector>

namespace investm {

void PrintHeading(std::string_view title);
void PrintValue(std::string_view label, double value,
                std::string_view unit = "");
void PrintPercent(std::string_view label, double fraction);

// A fixed-width table printed to stdout.
class Table {
 public:
  explicit Table(std::vector<std::string> header, int width = 12);

  void AddRow(const std::vector<double>& values, int precision = 4);
  void AddRow(std::vector<std::string> cells);
  void Print() const;

 private:
  void PrintCells(const std::vector<std::string>& cells) const;

  std::vector<std::string> header_;
  std::vector<std::vector<std::string>> rows_;
  int width_;
};

}  // namespace investm

#endif  // CPP_CORE_TABLE_H_
