#include "cpp/core/matrix.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <utility>
#include <vector>

#include "cpp/core/check.h"

namespace investm {

Matrix::Matrix(std::size_t rows, std::size_t cols, double fill)
    : rows_(rows), cols_(cols), data_(rows * cols, fill) {}

Matrix::Matrix(std::initializer_list<std::initializer_list<double>> rows)
    : rows_(rows.size()), cols_(rows.size() ? rows.begin()->size() : 0) {
  data_.reserve(rows_ * cols_);
  for (const auto& row : rows) {
    INVESTM_CHECK(row.size() == cols_, "ragged initializer");
    data_.insert(data_.end(), row.begin(), row.end());
  }
}

Matrix Matrix::Identity(std::size_t n) {
  Matrix m(n, n);
  for (std::size_t i = 0; i < n; ++i) m(i, i) = 1.0;
  return m;
}

Matrix Matrix::Diagonal(std::span<const double> diag) {
  Matrix m(diag.size(), diag.size());
  for (std::size_t i = 0; i < diag.size(); ++i) m(i, i) = diag[i];
  return m;
}

Matrix Matrix::FromColumns(const std::vector<Vector>& columns) {
  INVESTM_CHECK(!columns.empty(), "no columns");
  Matrix m(columns.front().size(), columns.size());
  for (std::size_t c = 0; c < columns.size(); ++c) {
    INVESTM_CHECK(columns[c].size() == m.rows(), "ragged columns");
    for (std::size_t r = 0; r < m.rows(); ++r) m(r, c) = columns[c][r];
  }
  return m;
}

Vector Matrix::Row(std::size_t r) const {
  return Vector(data_.begin() + static_cast<std::ptrdiff_t>(r * cols_),
                data_.begin() + static_cast<std::ptrdiff_t>((r + 1) * cols_));
}

Vector Matrix::Column(std::size_t c) const {
  Vector v(rows_);
  for (std::size_t r = 0; r < rows_; ++r) v[r] = (*this)(r, c);
  return v;
}

Matrix Matrix::Transpose() const {
  Matrix t(cols_, rows_);
  for (std::size_t r = 0; r < rows_; ++r) {
    for (std::size_t c = 0; c < cols_; ++c) t(c, r) = (*this)(r, c);
  }
  return t;
}

Matrix operator*(const Matrix& a, const Matrix& b) {
  INVESTM_CHECK(a.cols() == b.rows(), "dimension mismatch in product");
  Matrix p(a.rows(), b.cols());
  // i-k-j loop order walks both operands row-wise (cache friendly).
  for (std::size_t i = 0; i < a.rows(); ++i) {
    for (std::size_t k = 0; k < a.cols(); ++k) {
      const double aik = a(i, k);
      for (std::size_t j = 0; j < b.cols(); ++j) p(i, j) += aik * b(k, j);
    }
  }
  return p;
}

Vector operator*(const Matrix& a, std::span<const double> x) {
  INVESTM_CHECK(a.cols() == x.size(), "dimension mismatch in product");
  Vector y(a.rows(), 0.0);
  for (std::size_t i = 0; i < a.rows(); ++i) {
    double s = 0.0;
    for (std::size_t j = 0; j < a.cols(); ++j) s += a(i, j) * x[j];
    y[i] = s;
  }
  return y;
}

Matrix operator+(const Matrix& a, const Matrix& b) {
  INVESTM_CHECK(a.rows() == b.rows() && a.cols() == b.cols(),
                "dimension mismatch in sum");
  Matrix s(a.rows(), a.cols());
  for (std::size_t i = 0; i < a.rows(); ++i) {
    for (std::size_t j = 0; j < a.cols(); ++j) s(i, j) = a(i, j) + b(i, j);
  }
  return s;
}

Matrix operator-(const Matrix& a, const Matrix& b) { return a + (-1.0) * b; }

Matrix operator*(double s, const Matrix& a) {
  Matrix m(a.rows(), a.cols());
  for (std::size_t i = 0; i < a.rows(); ++i) {
    for (std::size_t j = 0; j < a.cols(); ++j) m(i, j) = s * a(i, j);
  }
  return m;
}

double Dot(std::span<const double> a, std::span<const double> b) {
  INVESTM_CHECK(a.size() == b.size(), "dimension mismatch in dot product");
  return std::inner_product(a.begin(), a.end(), b.begin(), 0.0);
}

double QuadraticForm(std::span<const double> x, const Matrix& a,
                     std::span<const double> y) {
  INVESTM_CHECK(x.size() == a.rows() && y.size() == a.cols(),
                "dimension mismatch in quadratic form");
  double s = 0.0;
  for (std::size_t i = 0; i < a.rows(); ++i) {
    double row = 0.0;
    for (std::size_t j = 0; j < a.cols(); ++j) row += a(i, j) * y[j];
    s += x[i] * row;
  }
  return s;
}

namespace {

// In-place LU with partial pivoting. Returns false if singular.
bool LuDecompose(Matrix& lu, std::vector<std::size_t>& perm) {
  const std::size_t n = lu.rows();
  perm.resize(n);
  std::iota(perm.begin(), perm.end(), std::size_t{0});
  double scale = 0.0;
  for (std::size_t i = 0; i < n; ++i) {
    for (std::size_t j = 0; j < n; ++j) {
      scale = std::max(scale, std::fabs(lu(i, j)));
    }
  }
  const double tiny = 1e-14 * (scale > 0.0 ? scale : 1.0);
  for (std::size_t k = 0; k < n; ++k) {
    std::size_t pivot = k;
    for (std::size_t i = k + 1; i < n; ++i) {
      if (std::fabs(lu(i, k)) > std::fabs(lu(pivot, k))) pivot = i;
    }
    if (std::fabs(lu(pivot, k)) <= tiny) return false;
    if (pivot != k) {
      for (std::size_t j = 0; j < n; ++j) std::swap(lu(k, j), lu(pivot, j));
      std::swap(perm[k], perm[pivot]);
    }
    for (std::size_t i = k + 1; i < n; ++i) {
      lu(i, k) /= lu(k, k);
      const double f = lu(i, k);
      for (std::size_t j = k + 1; j < n; ++j) lu(i, j) -= f * lu(k, j);
    }
  }
  return true;
}

Vector LuSolve(const Matrix& lu, const std::vector<std::size_t>& perm,
               std::span<const double> b) {
  const std::size_t n = lu.rows();
  Vector x(n);
  for (std::size_t i = 0; i < n; ++i) {
    double s = b[perm[i]];
    for (std::size_t j = 0; j < i; ++j) s -= lu(i, j) * x[j];
    x[i] = s;
  }
  for (std::size_t i = n; i-- > 0;) {
    double s = x[i];
    for (std::size_t j = i + 1; j < n; ++j) s -= lu(i, j) * x[j];
    x[i] = s / lu(i, i);
  }
  return x;
}

}  // namespace

std::optional<Vector> Solve(const Matrix& a, std::span<const double> b) {
  INVESTM_CHECK(a.rows() == a.cols() && a.rows() == b.size(),
                "Solve needs a square system");
  Matrix lu = a;
  std::vector<std::size_t> perm;
  if (!LuDecompose(lu, perm)) return std::nullopt;
  return LuSolve(lu, perm, b);
}

std::optional<Matrix> Inverse(const Matrix& a) {
  INVESTM_CHECK(a.rows() == a.cols(), "Inverse needs a square matrix");
  const std::size_t n = a.rows();
  Matrix lu = a;
  std::vector<std::size_t> perm;
  if (!LuDecompose(lu, perm)) return std::nullopt;
  Matrix inv(n, n);
  Vector e(n, 0.0);
  for (std::size_t c = 0; c < n; ++c) {
    std::fill(e.begin(), e.end(), 0.0);
    e[c] = 1.0;
    const Vector col = LuSolve(lu, perm, e);
    for (std::size_t r = 0; r < n; ++r) inv(r, c) = col[r];
  }
  return inv;
}

std::optional<Matrix> Cholesky(const Matrix& a) {
  INVESTM_CHECK(a.rows() == a.cols(), "Cholesky needs a square matrix");
  const std::size_t n = a.rows();
  Matrix l(n, n);
  for (std::size_t j = 0; j < n; ++j) {
    double d = a(j, j);
    for (std::size_t k = 0; k < j; ++k) d -= l(j, k) * l(j, k);
    if (d <= 0.0) return std::nullopt;
    l(j, j) = std::sqrt(d);
    for (std::size_t i = j + 1; i < n; ++i) {
      if (std::fabs(a(i, j) - a(j, i)) > 1e-12 * (1.0 + std::fabs(a(i, j)))) {
        return std::nullopt;
      }
      double s = a(i, j);
      for (std::size_t k = 0; k < j; ++k) s -= l(i, k) * l(j, k);
      l(i, j) = s / l(j, j);
    }
  }
  return l;
}

SymmetricEigen EigenSymmetric(const Matrix& a) {
  INVESTM_CHECK(a.rows() == a.cols(), "eigen needs a square matrix");
  const std::size_t n = a.rows();
  Matrix m = a;
  Matrix v = Matrix::Identity(n);
  for (int sweep = 0; sweep < 100; ++sweep) {
    double off = 0.0;
    for (std::size_t p = 0; p < n; ++p) {
      for (std::size_t q = p + 1; q < n; ++q) off += m(p, q) * m(p, q);
    }
    if (off < 1e-30) break;
    for (std::size_t p = 0; p < n; ++p) {
      for (std::size_t q = p + 1; q < n; ++q) {
        if (m(p, q) == 0.0) continue;
        const double theta = (m(q, q) - m(p, p)) / (2.0 * m(p, q));
        const double t = (theta >= 0.0 ? 1.0 : -1.0) /
                         (std::fabs(theta) + std::sqrt(theta * theta + 1.0));
        const double c = 1.0 / std::sqrt(t * t + 1.0);
        const double s = t * c;
        for (std::size_t k = 0; k < n; ++k) {
          const double mkp = m(k, p);
          const double mkq = m(k, q);
          m(k, p) = c * mkp - s * mkq;
          m(k, q) = s * mkp + c * mkq;
        }
        for (std::size_t k = 0; k < n; ++k) {
          const double mpk = m(p, k);
          const double mqk = m(q, k);
          m(p, k) = c * mpk - s * mqk;
          m(q, k) = s * mpk + c * mqk;
        }
        for (std::size_t k = 0; k < n; ++k) {
          const double vkp = v(k, p);
          const double vkq = v(k, q);
          v(k, p) = c * vkp - s * vkq;
          v(k, q) = s * vkp + c * vkq;
        }
      }
    }
  }
  std::vector<std::size_t> order(n);
  std::iota(order.begin(), order.end(), std::size_t{0});
  std::sort(order.begin(), order.end(),
            [&](std::size_t i, std::size_t j) { return m(i, i) > m(j, j); });
  SymmetricEigen result{Vector(n), Matrix(n, n)};
  for (std::size_t k = 0; k < n; ++k) {
    result.values[k] = m(order[k], order[k]);
    for (std::size_t r = 0; r < n; ++r) {
      result.vectors(r, k) = v(r, order[k]);
    }
  }
  return result;
}

Matrix CovarianceMatrix(const Matrix& data, int ddof) {
  const std::size_t t = data.rows();
  const std::size_t n = data.cols();
  INVESTM_CHECK(static_cast<double>(t) - ddof > 0, "too few observations");
  Vector means(n, 0.0);
  for (std::size_t r = 0; r < t; ++r) {
    for (std::size_t c = 0; c < n; ++c) means[c] += data(r, c);
  }
  for (double& m : means) m /= static_cast<double>(t);
  Matrix cov(n, n);
  for (std::size_t r = 0; r < t; ++r) {
    for (std::size_t i = 0; i < n; ++i) {
      const double di = data(r, i) - means[i];
      for (std::size_t j = i; j < n; ++j) {
        cov(i, j) += di * (data(r, j) - means[j]);
      }
    }
  }
  const double denom = static_cast<double>(t) - ddof;
  for (std::size_t i = 0; i < n; ++i) {
    for (std::size_t j = i; j < n; ++j) {
      cov(i, j) /= denom;
      cov(j, i) = cov(i, j);
    }
  }
  return cov;
}

Matrix CorrelationFromCovariance(const Matrix& cov) {
  const std::size_t n = cov.rows();
  Matrix corr(n, n);
  for (std::size_t i = 0; i < n; ++i) {
    for (std::size_t j = 0; j < n; ++j) {
      corr(i, j) = cov(i, j) / std::sqrt(cov(i, i) * cov(j, j));
    }
  }
  return corr;
}

Matrix CovarianceFromCorrelation(std::span<const double> std_devs,
                                 const Matrix& corr) {
  const std::size_t n = std_devs.size();
  INVESTM_CHECK(corr.rows() == n && corr.cols() == n, "dimension mismatch");
  Matrix cov(n, n);
  for (std::size_t i = 0; i < n; ++i) {
    for (std::size_t j = 0; j < n; ++j) {
      cov(i, j) = std_devs[i] * std_devs[j] * corr(i, j);
    }
  }
  return cov;
}

}  // namespace investm
