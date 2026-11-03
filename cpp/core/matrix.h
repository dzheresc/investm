// Small dense linear algebra for portfolio problems.
//
// Portfolios in this companion have tens of assets, not thousands, so a
// straightforward row-major matrix with LU, Cholesky and Jacobi
// eigen-decomposition is all that is needed and keeps the code dependency
// free. Used from Chapter 6 onward (covariance matrices, efficient sets,
// factor models).

#ifndef CPP_CORE_MATRIX_H_
#define CPP_CORE_MATRIX_H_

#include <cstddef>
#include <initializer_list>
#include <optional>
#include <span>
#include <vector>

namespace investm {

using Vector = std::vector<double>;

class Matrix {
 public:
  Matrix() = default;
  Matrix(std::size_t rows, std::size_t cols, double fill = 0.0);
  // Row-by-row initialization: Matrix({{1, 2}, {3, 4}}).
  Matrix(std::initializer_list<std::initializer_list<double>> rows);

  static Matrix Identity(std::size_t n);
  static Matrix Diagonal(std::span<const double> diag);
  // Builds a matrix whose columns are the given vectors.
  static Matrix FromColumns(const std::vector<Vector>& columns);

  std::size_t rows() const { return rows_; }
  std::size_t cols() const { return cols_; }

  double& operator()(std::size_t r, std::size_t c) {
    return data_[r * cols_ + c];
  }
  double operator()(std::size_t r, std::size_t c) const {
    return data_[r * cols_ + c];
  }

  Vector Row(std::size_t r) const;
  Vector Column(std::size_t c) const;
  Matrix Transpose() const;

 private:
  std::size_t rows_ = 0;
  std::size_t cols_ = 0;
  std::vector<double> data_;
};

Matrix operator*(const Matrix& a, const Matrix& b);
Vector operator*(const Matrix& a, std::span<const double> x);
Matrix operator+(const Matrix& a, const Matrix& b);
Matrix operator-(const Matrix& a, const Matrix& b);
Matrix operator*(double s, const Matrix& a);

double Dot(std::span<const double> a, std::span<const double> b);
// x' A y.
double QuadraticForm(std::span<const double> x, const Matrix& a,
                     std::span<const double> y);
inline double QuadraticForm(std::span<const double> x, const Matrix& a) {
  return QuadraticForm(x, a, x);
}

// Solves A x = b by LU decomposition with partial pivoting. Returns nullopt
// for a (numerically) singular matrix.
std::optional<Vector> Solve(const Matrix& a, std::span<const double> b);
std::optional<Matrix> Inverse(const Matrix& a);

// Lower-triangular L with A = L L'. Returns nullopt unless A is symmetric
// positive definite.
std::optional<Matrix> Cholesky(const Matrix& a);

// Eigen-decomposition of a symmetric matrix by cyclic Jacobi rotations.
// Eigenvalues are sorted in decreasing order; column k of `vectors` is the
// unit eigenvector for values[k].
struct SymmetricEigen {
  Vector values;
  Matrix vectors;
};
SymmetricEigen EigenSymmetric(const Matrix& a);

// Sample covariance matrix of the columns of `data` (rows = observations).
Matrix CovarianceMatrix(const Matrix& data, int ddof = 1);
// Converts covariances to correlations.
Matrix CorrelationFromCovariance(const Matrix& cov);
// Builds a covariance matrix from standard deviations and correlations.
Matrix CovarianceFromCorrelation(std::span<const double> std_devs,
                                 const Matrix& corr);

}  // namespace investm

#endif  // CPP_CORE_MATRIX_H_
