#include "cpp/core/matrix.h"

#include <cmath>

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

TEST(MatrixTest, JacobiEigenvalues) {
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
