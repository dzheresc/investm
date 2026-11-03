#include "cpp/core/matrix.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <random>
#include <vector>

#include "cpp/core/regression.h"
#include "gtest/gtest.h"

namespace investm {
namespace {

TEST(MatrixTest, ProductAndTranspose) {
  const Matrix a({{1, 2}, {3, 4}});
  const Matrix b({{0, 1}, {1, 0}});
  const Matrix p = a * b;
  EXPECT_DOUBLE_EQ(p(0, 0), 2.0);
  EXPECT_DOUBLE_EQ(p(1, 1), 3.0);
  EXPECT_DOUBLE_EQ(a.Transpose()(0, 1), 3.0);
  const Vector v = a * Vector{1.0, 1.0};
  EXPECT_DOUBLE_EQ(v[1], 7.0);
  EXPECT_DOUBLE_EQ(QuadraticForm(Vector{1.0, 1.0}, a), 10.0);
}

TEST(MatrixTest, SolveAndInverse) {
  const Matrix a({{4, 1, 2}, {1, 5, 3}, {2, 3, 6}});
  const Vector b{1, 2, 3};
  const auto x = Solve(a, b);
  ASSERT_TRUE(x.has_value());
  const Vector check = a * *x;
  for (std::size_t i = 0; i < 3; ++i) EXPECT_NEAR(check[i], b[i], 1e-12);
  const auto inv = Inverse(a);
  ASSERT_TRUE(inv.has_value());
  const Matrix id = a * *inv;
  for (std::size_t i = 0; i < 3; ++i) {
    for (std::size_t j = 0; j < 3; ++j) {
      EXPECT_NEAR(id(i, j), i == j ? 1.0 : 0.0, 1e-12);
    }
  }
}

TEST(MatrixTest, SingularMatrixIsReported) {
  const Matrix a({{1, 2}, {2, 4}});
  EXPECT_FALSE(Solve(a, Vector{1, 1}).has_value());
  EXPECT_FALSE(Inverse(a).has_value());
}

TEST(MatrixTest, CholeskyReconstructs) {
  const Matrix a({{4, 2}, {2, 3}});
  const auto l = Cholesky(a);
  ASSERT_TRUE(l.has_value());
  const Matrix back = *l * l->Transpose();
  EXPECT_NEAR(back(0, 1), 2.0, 1e-14);
  EXPECT_NEAR(back(1, 1), 3.0, 1e-14);
  EXPECT_FALSE(Cholesky(Matrix({{1, 2}, {2, 1}})).has_value());
}

TEST(MatrixTest, EigenvaluesOfTwoByTwo) {
  const Matrix a({{2, 1}, {1, 2}});
  const SymmetricEigen e = EigenSymmetric(a);
  EXPECT_NEAR(e.values[0], 3.0, 1e-12);
  EXPECT_NEAR(e.values[1], 1.0, 1e-12);
  EXPECT_NEAR(std::fabs(e.vectors(0, 0)), std::sqrt(0.5), 1e-12);
  // A v = lambda v for every pair.
  for (std::size_t k = 0; k < 2; ++k) {
    const Vector v = e.vectors.Column(k);
    const Vector av = a * v;
    for (std::size_t i = 0; i < 2; ++i) {
      EXPECT_NEAR(av[i], e.values[k] * v[i], 1e-12);
    }
  }
}

// Random symmetric positive semi-definite matrix B'B / m.
Matrix RandomSpd(std::size_t n, unsigned seed) {
  std::mt19937 rng(seed);
  std::normal_distribution<double> z(0.0, 1.0);
  const std::size_t m = n + 3;
  Matrix b(m, n);
  for (std::size_t i = 0; i < m; ++i) {
    for (std::size_t j = 0; j < n; ++j) b(i, j) = z(rng);
  }
  Matrix a = b.Transpose() * b;
  for (std::size_t i = 0; i < n; ++i) {
    for (std::size_t j = 0; j < n; ++j) a(i, j) /= static_cast<double>(m);
  }
  return a;
}

void ExpectValidDecomposition(const Matrix& a, const SymmetricEigen& e,
                              double tol) {
  const std::size_t n = a.rows();
  ASSERT_EQ(e.values.size(), n);
  for (std::size_t k = 1; k < n; ++k) {
    EXPECT_GE(e.values[k - 1], e.values[k]);
  }
  for (std::size_t k = 0; k < n; ++k) {
    const Vector v = e.vectors.Column(k);
    const Vector av = a * v;
    for (std::size_t i = 0; i < n; ++i) {
      EXPECT_NEAR(av[i], e.values[k] * v[i], tol);
    }
    for (std::size_t l = 0; l < n; ++l) {
      double dot = 0.0;
      const Vector w = e.vectors.Column(l);
      for (std::size_t i = 0; i < n; ++i) dot += v[i] * w[i];
      EXPECT_NEAR(dot, k == l ? 1.0 : 0.0, tol);
    }
  }
}

TEST(MatrixTest, EigenOfRandomSpdMatrices) {
  for (const std::size_t n : std::vector<std::size_t>{1, 2, 3, 7, 30, 80}) {
    const Matrix a = RandomSpd(n, static_cast<unsigned>(n));
    ExpectValidDecomposition(a, EigenSymmetric(a), 1e-10);
  }
}

TEST(MatrixTest, EigenOfDiagonalZeroAndRepeated) {
  const Matrix d = Matrix::Diagonal(Vector{1.0, 5.0, 3.0});
  const SymmetricEigen ed = EigenSymmetric(d);
  EXPECT_NEAR(ed.values[0], 5.0, 1e-14);
  EXPECT_NEAR(ed.values[1], 3.0, 1e-14);
  EXPECT_NEAR(ed.values[2], 1.0, 1e-14);
  ExpectValidDecomposition(d, ed, 1e-14);

  const Matrix zero(4, 4);
  ExpectValidDecomposition(zero, EigenSymmetric(zero), 1e-14);

  // 2I + 1 1' has eigenvalues 5 (once) and 2 (three times).
  Matrix rep(4, 4, 1.0);
  for (std::size_t i = 0; i < 4; ++i) rep(i, i) = 3.0;
  const SymmetricEigen er = EigenSymmetric(rep);
  EXPECT_NEAR(er.values[0], 6.0, 1e-12);
  for (std::size_t k = 1; k < 4; ++k) EXPECT_NEAR(er.values[k], 2.0, 1e-12);
  ExpectValidDecomposition(rep, er, 1e-12);

  EXPECT_TRUE(EigenSymmetric(Matrix()).values.empty());
}

TEST(MatrixTest, LargestEigenvalueMatchesFullDecomposition) {
  for (const std::size_t n : std::vector<std::size_t>{1, 5, 40}) {
    const Matrix a = RandomSpd(n, 100U + static_cast<unsigned>(n));
    const double exact = EigenSymmetric(a).values[0];
    const double power = LargestEigenvalue(a);
    // Padded by the residual: never below the true value, and close to it.
    EXPECT_GE(power, exact * (1.0 - 1e-12));
    EXPECT_LE(power, exact * (1.0 + 1e-4));
  }
  EXPECT_DOUBLE_EQ(LargestEigenvalue(Matrix(3, 3)), 0.0);
}

TEST(MatrixTest, MultiplyIntoMatchesProduct) {
  const Matrix a = RandomSpd(13, 9);
  Vector x(13);
  for (std::size_t i = 0; i < x.size(); ++i) {
    x[i] = 0.1 * static_cast<double>(i) - 0.5;
  }
  const Vector expected = a * x;
  Vector y(13, -1.0);
  MultiplyInto(a, x, y);
  for (std::size_t i = 0; i < y.size(); ++i) {
    EXPECT_NEAR(y[i], expected[i], 1e-13);
  }
}

// Textbook triple loop, for checking the unrolled product bit for bit.
Matrix NaiveProduct(const Matrix& a, const Matrix& b) {
  Matrix p(a.rows(), b.cols());
  for (std::size_t i = 0; i < a.rows(); ++i) {
    for (std::size_t k = 0; k < a.cols(); ++k) {
      for (std::size_t j = 0; j < b.cols(); ++j) p(i, j) += a(i, k) * b(k, j);
    }
  }
  return p;
}

Matrix Uniform(std::size_t rows, std::size_t cols, unsigned seed) {
  std::mt19937 rng(seed);
  std::uniform_real_distribution<double> u(-1.0, 1.0);
  Matrix m(rows, cols);
  for (std::size_t i = 0; i < rows; ++i) {
    for (std::size_t j = 0; j < cols; ++j) m(i, j) = u(rng);
  }
  return m;
}

TEST(MatrixTest, ProductMatchesNaiveLoopExactly) {
  // Inner dimensions on both sides of the 4-way unroll, plus empty ones.
  for (const std::size_t inner : std::vector<std::size_t>{0, 1, 3, 4, 5, 9}) {
    const Matrix a = Uniform(7, inner, 11);
    const Matrix b = Uniform(inner, 6, 12);
    const Matrix p = a * b;
    const Matrix expected = NaiveProduct(a, b);
    ASSERT_EQ(p.rows(), 7U);
    ASSERT_EQ(p.cols(), 6U);
    for (std::size_t i = 0; i < 7; ++i) {
      for (std::size_t j = 0; j < 6; ++j) EXPECT_EQ(p(i, j), expected(i, j));
    }
  }
  EXPECT_EQ((Matrix(0, 3) * Matrix(3, 2)).rows(), 0U);
  EXPECT_EQ((Matrix(2, 3) * Matrix(3, 0)).cols(), 0U);
}

TEST(MatrixTest, InverseOfRandomMatrices) {
  for (const std::size_t n : std::vector<std::size_t>{1, 2, 5, 17, 40}) {
    const Matrix a = Uniform(n, n, static_cast<unsigned>(n) + 20);
    const auto inv = Inverse(a);
    ASSERT_TRUE(inv.has_value());
    const Matrix left = *inv * a;
    const Matrix right = a * *inv;
    for (std::size_t i = 0; i < n; ++i) {
      for (std::size_t j = 0; j < n; ++j) {
        EXPECT_NEAR(left(i, j), i == j ? 1.0 : 0.0, 1e-10);
        EXPECT_NEAR(right(i, j), i == j ? 1.0 : 0.0, 1e-10);
      }
      // Each column agrees with an independent solve against e_i.
      Vector e(n, 0.0);
      e[i] = 1.0;
      const Vector col = Solve(a, e).value();
      for (std::size_t r = 0; r < n; ++r) EXPECT_EQ((*inv)(r, i), col[r]);
    }
  }
}

TEST(MatrixTest, EigenToleratesRoundingAsymmetry) {
  // A tiny asymmetry from rounding must not change the decomposition much.
  Matrix a = RandomSpd(12, 31);
  a(3, 7) += 1e-13;
  ExpectValidDecomposition(a, EigenSymmetric(a), 1e-10);
}

TEST(MatrixTest, CovarianceMatchesTwoPassDefinition) {
  const Matrix data = Uniform(50, 9, 41);
  const Matrix cov = CovarianceMatrix(data);
  for (std::size_t i = 0; i < 9; ++i) {
    for (std::size_t j = 0; j < 9; ++j) {
      double mi = 0.0;
      double mj = 0.0;
      for (std::size_t r = 0; r < 50; ++r) {
        mi += data(r, i);
        mj += data(r, j);
      }
      mi /= 50.0;
      mj /= 50.0;
      double s = 0.0;
      for (std::size_t r = 0; r < 50; ++r) {
        s += (data(r, i) - mi) * (data(r, j) - mj);
      }
      EXPECT_NEAR(cov(i, j), s / 49.0, 1e-15);
      EXPECT_EQ(cov(i, j), cov(j, i));
    }
  }
}

TEST(MatrixTest, CovarianceAndCorrelation) {
  const Matrix data({{1, 2}, {2, 4}, {3, 6}, {4, 8}});
  const Matrix cov = CovarianceMatrix(data);
  EXPECT_NEAR(cov(0, 0), 5.0 / 3.0, 1e-14);
  EXPECT_NEAR(cov(0, 1), 10.0 / 3.0, 1e-14);
  EXPECT_NEAR(CorrelationFromCovariance(cov)(0, 1), 1.0, 1e-14);
  const Matrix back =
      CovarianceFromCorrelation(Vector{0.2, 0.3}, Matrix({{1, 0.5}, {0.5, 1}}));
  EXPECT_NEAR(back(0, 1), 0.03, 1e-15);
}

TEST(RegressionTest, RecoversCoefficients) {
  Matrix x(6, 2);
  Vector y(6);
  const double x1[] = {1, 2, 3, 4, 5, 6};
  const double x2[] = {2, 1, 4, 3, 6, 5};
  for (std::size_t r = 0; r < 6; ++r) {
    x(r, 0) = x1[r];
    x(r, 1) = x2[r];
    y[r] = 0.5 + 2.0 * x1[r] - 1.0 * x2[r];
  }
  const auto fit = MultipleOls(x, y);
  ASSERT_TRUE(fit.has_value());
  EXPECT_NEAR(fit->coefficients[0], 0.5, 1e-12);
  EXPECT_NEAR(fit->coefficients[1], 2.0, 1e-12);
  EXPECT_NEAR(fit->coefficients[2], -1.0, 1e-12);
  EXPECT_NEAR(fit->r_squared, 1.0, 1e-12);
  EXPECT_NEAR(fit->Predict(Vector{1.0, 1.0}), 1.5, 1e-12);
}

TEST(RegressionTest, SharedDesignMatchesIndependentFits) {
  const Matrix x = Uniform(40, 3, 51);
  const Matrix ys = Uniform(40, 5, 52);
  for (const bool intercept : {true, false}) {
    const auto design = OlsDesign::Create(x, intercept);
    ASSERT_TRUE(design.has_value());
    EXPECT_EQ(design->num_observations(), 40U);
    for (std::size_t c = 0; c < ys.cols(); ++c) {
      const Vector y = ys.Column(c);
      const LinearRegression shared = design->Fit(y);
      const LinearRegression alone = MultipleOls(x, y, intercept).value();
      EXPECT_EQ(shared.has_intercept, intercept);
      ASSERT_EQ(shared.coefficients.size(), alone.coefficients.size());
      for (std::size_t k = 0; k < shared.coefficients.size(); ++k) {
        EXPECT_EQ(shared.coefficients[k], alone.coefficients[k]);
        EXPECT_EQ(shared.std_errors[k], alone.std_errors[k]);
      }
      EXPECT_EQ(shared.r_squared, alone.r_squared);
      EXPECT_EQ(shared.residual_std_dev, alone.residual_std_dev);
    }
  }
  EXPECT_FALSE(OlsDesign::Create(Uniform(3, 3, 53)).has_value());
}

TEST(RegressionTest, CollinearRegressorsReturnNullopt) {
  Matrix x(5, 2);
  for (std::size_t r = 0; r < 5; ++r) {
    x(r, 0) = static_cast<double>(r);
    x(r, 1) = 2.0 * static_cast<double>(r);
  }
  EXPECT_FALSE(MultipleOls(x, Vector{1, 2, 3, 4, 5}).has_value());
}

}  // namespace
}  // namespace investm
