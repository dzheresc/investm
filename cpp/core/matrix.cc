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

void MultiplyInto(const Matrix& a, std::span<const double> x,
                  std::span<double> y) {
  INVESTM_CHECK(a.cols() == x.size() && a.rows() == y.size(),
                "dimension mismatch in product");
  const std::size_t n = a.cols();
  for (std::size_t i = 0; i < a.rows(); ++i) {
    const std::span<const double> row = a.RowSpan(i);
    // Four independent partial sums break the floating-point add chain so
    // the loop pipelines (and vectorizes) without -ffast-math.
    double s0 = 0.0;
    double s1 = 0.0;
    double s2 = 0.0;
    double s3 = 0.0;
    std::size_t j = 0;
    for (; j + 4 <= n; j += 4) {
      s0 += row[j] * x[j];
      s1 += row[j + 1] * x[j + 1];
      s2 += row[j + 2] * x[j + 2];
      s3 += row[j + 3] * x[j + 3];
    }
    for (; j < n; ++j) s0 += row[j] * x[j];
    y[i] = (s0 + s1) + (s2 + s3);
  }
}

Vector operator*(const Matrix& a, std::span<const double> x) {
  Vector y(a.rows(), 0.0);
  if (a.rows() > 0) MultiplyInto(a, x, y);
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

namespace {

// Householder reduction of a symmetric matrix to tridiagonal form. On
// return `v` holds the orthogonal transformation, `d` the diagonal and
// `e` the subdiagonal (e[0] = 0). This is the classic tred2 algorithm
// (Bowdler, Martin, Reinsch and Wilkinson), O(n^3) once instead of per
// Jacobi sweep.
void Tridiagonalize(Matrix& v, Vector& d, Vector& e) {
  const std::size_t n = v.rows();
  for (std::size_t j = 0; j < n; ++j) d[j] = v(n - 1, j);
  for (std::size_t i = n - 1; i > 0; --i) {
    double scale = 0.0;
    double h = 0.0;
    for (std::size_t k = 0; k < i; ++k) scale += std::fabs(d[k]);
    if (scale == 0.0) {
      e[i] = d[i - 1];
      for (std::size_t j = 0; j < i; ++j) {
        d[j] = v(i - 1, j);
        v(i, j) = 0.0;
        v(j, i) = 0.0;
      }
    } else {
      for (std::size_t k = 0; k < i; ++k) {
        d[k] /= scale;
        h += d[k] * d[k];
      }
      double f = d[i - 1];
      double g = std::sqrt(h);
      if (f > 0) g = -g;
      e[i] = scale * g;
      h -= f * g;
      d[i - 1] = f - g;
      for (std::size_t j = 0; j < i; ++j) e[j] = 0.0;
      for (std::size_t j = 0; j < i; ++j) {
        f = d[j];
        v(j, i) = f;
        g = e[j] + v(j, j) * f;
        for (std::size_t k = j + 1; k < i; ++k) {
          g += v(k, j) * d[k];
          e[k] += v(k, j) * f;
        }
        e[j] = g;
      }
      f = 0.0;
      for (std::size_t j = 0; j < i; ++j) {
        e[j] /= h;
        f += e[j] * d[j];
      }
      const double hh = f / (h + h);
      for (std::size_t j = 0; j < i; ++j) e[j] -= hh * d[j];
      for (std::size_t j = 0; j < i; ++j) {
        f = d[j];
        g = e[j];
        for (std::size_t k = j; k < i; ++k) {
          v(k, j) -= f * e[k] + g * d[k];
        }
        d[j] = v(i - 1, j);
        v(i, j) = 0.0;
      }
    }
    d[i] = h;
  }
  // Accumulate the transformations.
  for (std::size_t i = 0; i + 1 < n; ++i) {
    v(n - 1, i) = v(i, i);
    v(i, i) = 1.0;
    const double h = d[i + 1];
    if (h != 0.0) {
      for (std::size_t k = 0; k <= i; ++k) d[k] = v(k, i + 1) / h;
      for (std::size_t j = 0; j <= i; ++j) {
        double g = 0.0;
        for (std::size_t k = 0; k <= i; ++k) g += v(k, i + 1) * v(k, j);
        for (std::size_t k = 0; k <= i; ++k) v(k, j) -= g * d[k];
      }
    }
    for (std::size_t k = 0; k <= i; ++k) v(k, i + 1) = 0.0;
  }
  for (std::size_t j = 0; j < n; ++j) {
    d[j] = v(n - 1, j);
    v(n - 1, j) = 0.0;
  }
  v(n - 1, n - 1) = 1.0;
  e[0] = 0.0;
}

// Implicit QL iterations on the tridiagonal matrix (tql2). `vt` holds the
// eigenvectors as rows (the transpose of Tridiagonalize's output) so each
// Givens rotation updates two contiguous rows. Returns false if an
// eigenvalue fails to converge.
bool TridiagonalQl(Matrix& vt, Vector& d, Vector& e) {
  const std::size_t n = d.size();
  for (std::size_t i = 1; i < n; ++i) e[i - 1] = e[i];
  e[n - 1] = 0.0;
  double f = 0.0;
  double tst1 = 0.0;
  constexpr double kEps = 2.220446049250313e-16;
  for (std::size_t l = 0; l < n; ++l) {
    tst1 = std::max(tst1, std::fabs(d[l]) + std::fabs(e[l]));
    std::size_t m = l;
    while (m < n - 1 && std::fabs(e[m]) > kEps * tst1) ++m;
    if (m > l) {
      int iterations = 0;
      do {
        if (++iterations > 64) return false;
        double g = d[l];
        double p = (d[l + 1] - g) / (2.0 * e[l]);
        double r = std::hypot(p, 1.0);
        if (p < 0) r = -r;
        d[l] = e[l] / (p + r);
        d[l + 1] = e[l] * (p + r);
        const double dl1 = d[l + 1];
        double h = g - d[l];
        for (std::size_t i = l + 2; i < n; ++i) d[i] -= h;
        f += h;
        p = d[m];
        double c = 1.0;
        double c2 = c;
        double c3 = c;
        const double el1 = e[l + 1];
        double s = 0.0;
        double s2 = 0.0;
        for (std::size_t i = m; i-- > l;) {
          c3 = c2;
          c2 = c;
          s2 = s;
          g = c * e[i];
          h = c * p;
          r = std::hypot(p, e[i]);
          e[i + 1] = s * r;
          s = e[i] / r;
          c = p / r;
          p = c * d[i] - s * g;
          d[i + 1] = h + s * (c * g + s * d[i]);
          double* row_i = &vt(i, 0);
          double* row_j = &vt(i + 1, 0);
          for (std::size_t k = 0; k < n; ++k) {
            const double hk = row_j[k];
            row_j[k] = s * row_i[k] + c * hk;
            row_i[k] = c * row_i[k] - s * hk;
          }
        }
        p = -s * s2 * c3 * el1 * e[l] / dl1;
        e[l] = s * p;
        d[l] = c * p;
      } while (std::fabs(e[l]) > kEps * tst1);
    }
    d[l] += f;
    e[l] = 0.0;
  }
  return true;
}

}  // namespace

SymmetricEigen EigenSymmetric(const Matrix& a) {
  INVESTM_CHECK(a.rows() == a.cols(), "eigen needs a square matrix");
  const std::size_t n = a.rows();
  SymmetricEigen result{Vector(n), Matrix(n, n)};
  if (n == 0) return result;
  Matrix v = a;
  Vector d(n);
  Vector e(n);
  Tridiagonalize(v, d, e);
  Matrix vt = v.Transpose();
  INVESTM_CHECK(TridiagonalQl(vt, d, e), "eigenvalues failed to converge");
  std::vector<std::size_t> order(n);
  std::iota(order.begin(), order.end(), std::size_t{0});
  std::sort(order.begin(), order.end(),
            [&](std::size_t i, std::size_t j) { return d[i] > d[j]; });
  for (std::size_t k = 0; k < n; ++k) {
    result.values[k] = d[order[k]];
    const double* row = &vt(order[k], 0);
    for (std::size_t r = 0; r < n; ++r) result.vectors(r, k) = row[r];
  }
  return result;
}

double LargestEigenvalue(const Matrix& a, int max_iterations,
                         double tolerance) {
  INVESTM_CHECK(a.rows() == a.cols() && a.rows() > 0,
                "need a non-empty square matrix");
  const std::size_t n = a.rows();
  // Power iteration from a fixed, non-symmetric start vector; the result
  // is padded by the residual norm |Av - lambda v|, so for a symmetric
  // matrix it is an upper bound on the eigenvalue it converged to.
  Vector v(n);
  for (std::size_t i = 0; i < n; ++i) {
    v[i] = 1.0 + 0.1 * static_cast<double>(i % 7);
  }
  double norm = std::sqrt(Dot(v, v));
  for (double& x : v) x /= norm;
  Vector av(n);
  double lambda = 0.0;
  for (int it = 0; it < max_iterations; ++it) {
    MultiplyInto(a, v, av);
    const double next = Dot(v, av);
    norm = std::sqrt(Dot(av, av));
    if (norm == 0.0) return 0.0;
    const bool done = std::fabs(next - lambda) <=
                      tolerance * std::max(1e-300, std::fabs(next));
    lambda = next;
    if (done) break;
    for (std::size_t i = 0; i < n; ++i) v[i] = av[i] / norm;
  }
  double residual = 0.0;
  for (std::size_t i = 0; i < n; ++i) {
    const double r = av[i] - lambda * v[i];
    residual += r * r;
  }
  return lambda + std::sqrt(residual);
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
