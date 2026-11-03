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
  const std::size_t inner = a.cols();
  const std::size_t m = b.cols();
  Matrix p(a.rows(), m);
  // i-k-j order walks every operand row-wise. Two refinements on top:
  //  * Tiling: a kBlockK x kBlockJ tile of b (16 KB) stays in L1 while it
  //    is applied to every row of p, instead of b being streamed from L2
  //    once per row of a.
  //  * Four rows of b are folded into each pass over p, so p is loaded and
  //    stored once per four multiply-adds.
  // Every p(i, j) still receives its terms in increasing k, one addition at
  // a time, so the result is bit-identical to the textbook triple loop.
  constexpr std::size_t kBlockK = 32;
  constexpr std::size_t kBlockJ = 64;
  for (std::size_t kk = 0; kk < inner; kk += kBlockK) {
    const std::size_t k_end = std::min(inner, kk + kBlockK);
    for (std::size_t jj = 0; jj < m; jj += kBlockJ) {
      const std::size_t j_end = std::min(m, jj + kBlockJ);
      for (std::size_t i = 0; i < a.rows(); ++i) {
        const std::span<const double> ai = a.RowSpan(i);
        const std::span<double> pi = p.RowSpan(i);
        std::size_t k = kk;
        for (; k + 4 <= k_end; k += 4) {
          const double a0 = ai[k];
          const double a1 = ai[k + 1];
          const double a2 = ai[k + 2];
          const double a3 = ai[k + 3];
          const std::span<const double> b0 = b.RowSpan(k);
          const std::span<const double> b1 = b.RowSpan(k + 1);
          const std::span<const double> b2 = b.RowSpan(k + 2);
          const std::span<const double> b3 = b.RowSpan(k + 3);
          for (std::size_t j = jj; j < j_end; ++j) {
            pi[j] =
                (((pi[j] + a0 * b0[j]) + a1 * b1[j]) + a2 * b2[j]) + a3 * b3[j];
          }
        }
        for (; k < k_end; ++k) {
          const double aik = ai[k];
          const std::span<const double> bk = b.RowSpan(k);
          for (std::size_t j = jj; j < j_end; ++j) pi[j] += aik * bk[j];
        }
      }
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
  // P A = L U, so A^-1 = U^-1 L^-1 P. All n right-hand sides are solved
  // together, one contiguous row of the result at a time, instead of n
  // column solves that each allocate and write the result with stride n.
  //  1. Z = L^-1 is unit lower triangular, so row j only has entries
  //     0..j; skipping the known zeros makes this pass n^3 / 6.
  //  2. X = U^-1 Z by back substitution on whole rows.
  //  3. Column k of X is column perm[k] of A^-1.
  // Each entry sees the same nonzero operations in the same order as the
  // column-by-column LuSolve, so the result is identical.
  Matrix x(n, n);
  for (std::size_t i = 0; i < n; ++i) {
    const std::span<double> xi = x.RowSpan(i);
    xi[i] = 1.0;
    for (std::size_t j = 0; j < i; ++j) {
      const double l = lu(i, j);
      const std::span<const double> xj = x.RowSpan(j);
      for (std::size_t c = 0; c <= j; ++c) xi[c] -= l * xj[c];
    }
  }
  for (std::size_t i = n; i-- > 0;) {
    const std::span<double> xi = x.RowSpan(i);
    for (std::size_t j = i + 1; j < n; ++j) {
      const double u = lu(i, j);
      const std::span<const double> xj = x.RowSpan(j);
      for (std::size_t c = 0; c < n; ++c) xi[c] -= u * xj[c];
    }
    const double pivot = lu(i, i);
    for (double& v : xi) v /= pivot;
  }
  Matrix inv(n, n);
  for (std::size_t r = 0; r < n; ++r) {
    const std::span<const double> xr = x.RowSpan(r);
    const std::span<double> out = inv.RowSpan(r);
    for (std::size_t k = 0; k < n; ++k) out[perm[k]] = xr[k];
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

// Householder reduction of a symmetric matrix to tridiagonal form: the
// classic tred2 algorithm (Bowdler, Martin, Reinsch and Wilkinson), O(n^3)
// once instead of per Jacobi sweep. `d` gets the diagonal and `e` the
// subdiagonal (e[0] = 0).
//
// tred2 is written for column access: its O(n^3) inner loops run down
// columns of V. Here it works on W = V' instead, so every inner loop walks
// a contiguous row of the row-major storage. On entry `w` holds A' (equal
// to A when A is symmetric); on return it holds the transpose of the
// orthogonal transformation (its columns as rows).
void Tridiagonalize(Matrix& w, Vector& d, Vector& e) {
  const std::size_t n = w.rows();
  for (std::size_t j = 0; j < n; ++j) d[j] = w(j, n - 1);
  for (std::size_t i = n - 1; i > 0; --i) {
    double scale = 0.0;
    double h = 0.0;
    for (std::size_t k = 0; k < i; ++k) scale += std::fabs(d[k]);
    if (scale == 0.0) {
      e[i] = d[i - 1];
      for (std::size_t j = 0; j < i; ++j) {
        d[j] = w(j, i - 1);
        w(j, i) = 0.0;
        w(i, j) = 0.0;
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
        w(i, j) = f;
        g = e[j] + w(j, j) * f;
        for (std::size_t k = j + 1; k < i; ++k) {
          g += w(j, k) * d[k];
          e[k] += w(j, k) * f;
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
          w(j, k) -= f * e[k] + g * d[k];
        }
        d[j] = w(j, i - 1);
        w(j, i) = 0.0;
      }
    }
    d[i] = h;
  }
  // Accumulate the transformations.
  for (std::size_t i = 0; i + 1 < n; ++i) {
    w(i, n - 1) = w(i, i);
    w(i, i) = 1.0;
    const double h = d[i + 1];
    if (h != 0.0) {
      for (std::size_t k = 0; k <= i; ++k) d[k] = w(i + 1, k) / h;
      for (std::size_t j = 0; j <= i; ++j) {
        double g = 0.0;
        for (std::size_t k = 0; k <= i; ++k) g += w(i + 1, k) * w(j, k);
        for (std::size_t k = 0; k <= i; ++k) w(j, k) -= g * d[k];
      }
    }
    for (std::size_t k = 0; k <= i; ++k) w(i + 1, k) = 0.0;
  }
  for (std::size_t j = 0; j < n; ++j) {
    d[j] = w(j, n - 1);
    w(j, n - 1) = 0.0;
  }
  w(n - 1, n - 1) = 1.0;
  e[0] = 0.0;
}

// Applies one QL sweep's Givens rotations to the eigenvector matrix `v`
// (eigenvectors as columns). Rotation i, for i = m - 1 down to l, mixes
// columns i and i + 1. Doing them one at a time streams two whole columns
// per rotation; instead each row of v is visited once per sweep and the
// value moving down the cascade is carried in a register. Four rows are
// interleaved so the four carry chains overlap. The arithmetic on every
// entry is unchanged.
void ApplySweep(Matrix& v, std::size_t l, std::size_t m,
                std::span<const double> cos, std::span<const double> sin) {
  const std::size_t n = v.rows();
  std::size_t k = 0;
  for (; k + 4 <= n; k += 4) {
    const std::span<double> r0 = v.RowSpan(k);
    const std::span<double> r1 = v.RowSpan(k + 1);
    const std::span<double> r2 = v.RowSpan(k + 2);
    const std::span<double> r3 = v.RowSpan(k + 3);
    double x0 = r0[m];
    double x1 = r1[m];
    double x2 = r2[m];
    double x3 = r3[m];
    for (std::size_t i = m; i-- > l;) {
      const double c = cos[i];
      const double s = sin[i];
      const double y0 = r0[i];
      const double y1 = r1[i];
      const double y2 = r2[i];
      const double y3 = r3[i];
      r0[i + 1] = s * y0 + c * x0;
      r1[i + 1] = s * y1 + c * x1;
      r2[i + 1] = s * y2 + c * x2;
      r3[i + 1] = s * y3 + c * x3;
      x0 = c * y0 - s * x0;
      x1 = c * y1 - s * x1;
      x2 = c * y2 - s * x2;
      x3 = c * y3 - s * x3;
    }
    r0[l] = x0;
    r1[l] = x1;
    r2[l] = x2;
    r3[l] = x3;
  }
  for (; k < n; ++k) {
    const std::span<double> r = v.RowSpan(k);
    double x = r[m];
    for (std::size_t i = m; i-- > l;) {
      const double y = r[i];
      r[i + 1] = sin[i] * y + cos[i] * x;
      x = cos[i] * y - sin[i] * x;
    }
    r[l] = x;
  }
}

// Implicit QL iterations on the tridiagonal matrix (tql2), accumulating
// the rotations into `v` (eigenvectors as columns). Returns false if an
// eigenvalue fails to converge.
bool TridiagonalQl(Matrix& v, Vector& d, Vector& e) {
  const std::size_t n = d.size();
  Vector cos(n);
  Vector sin(n);
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
          cos[i] = c;
          sin[i] = s;
        }
        ApplySweep(v, l, m, cos, sin);
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
  Matrix w = a.Transpose();
  Vector d(n);
  Vector e(n);
  Tridiagonalize(w, d, e);
  Matrix v = w.Transpose();
  INVESTM_CHECK(TridiagonalQl(v, d, e), "eigenvalues failed to converge");
  std::vector<std::size_t> order(n);
  std::iota(order.begin(), order.end(), std::size_t{0});
  std::sort(order.begin(), order.end(),
            [&](std::size_t i, std::size_t j) { return d[i] > d[j]; });
  for (std::size_t k = 0; k < n; ++k) {
    result.values[k] = d[order[k]];
    for (std::size_t r = 0; r < n; ++r) result.vectors(r, k) = v(r, order[k]);
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
  // Each observation is centred once (O(n)) rather than inside the O(n^2)
  // update, and four observations are applied per pass over the upper
  // triangle, so the n x n accumulator (larger than L1 for n ~ 100) is
  // streamed t / 4 times instead of t times. Every cov(i, j) still adds its
  // terms in observation order, so the result is unchanged.
  constexpr std::size_t kRows = 4;
  Matrix centered(kRows, n);
  for (std::size_t r0 = 0; r0 < t; r0 += kRows) {
    const std::size_t rows = std::min(kRows, t - r0);
    for (std::size_t q = 0; q < rows; ++q) {
      const std::span<const double> row = data.RowSpan(r0 + q);
      const std::span<double> c = centered.RowSpan(q);
      for (std::size_t j = 0; j < n; ++j) c[j] = row[j] - means[j];
    }
    const std::span<const double> c0 = centered.RowSpan(0);
    const std::span<const double> c1 = centered.RowSpan(1);
    const std::span<const double> c2 = centered.RowSpan(2);
    const std::span<const double> c3 = centered.RowSpan(3);
    for (std::size_t i = 0; i < n; ++i) {
      const std::span<double> cov_i = cov.RowSpan(i);
      if (rows == kRows) {
        const double d0 = c0[i];
        const double d1 = c1[i];
        const double d2 = c2[i];
        const double d3 = c3[i];
        for (std::size_t j = i; j < n; ++j) {
          cov_i[j] = (((cov_i[j] + d0 * c0[j]) + d1 * c1[j]) + d2 * c2[j]) +
                     d3 * c3[j];
        }
      } else {
        for (std::size_t q = 0; q < rows; ++q) {
          const std::span<const double> cq = centered.RowSpan(q);
          const double dq = cq[i];
          for (std::size_t j = i; j < n; ++j) cov_i[j] += dq * cq[j];
        }
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
