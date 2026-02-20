// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/vector.h"

using namespace bun;

template<typename T> bool TESTVECTOR_EQUALITY(T l, T r) { return l == r; }
template<> bool TESTVECTOR_EQUALITY<float>(float l, float r)
{
  if(l == 0.0)
    return fSmall(r);
  if(r == 0.0)
    return fSmall(l);
  return fCompare(l, r, 1);
}
template<> bool TESTVECTOR_EQUALITY<double>(double l, double r)
{
  if(l == 0.0)
    return fSmall(r);
  if(r == 0.0)
    return fSmall(l);
  return fCompare(l, r, 100LL);
}

#define TESTVECTOR(vec, arr, __testret)          \
  for(size_t i = 0; i < N; ++i)                  \
  {                                              \
    TEST(TESTVECTOR_EQUALITY(vec.v[i], arr[i])); \
  }

template<typename T, int N> void TESTVALUES_To(T (&out)[N], T (&in)[N])
{
  for(size_t i = 0; i < N; ++i)
    out[i] = in[i];
}

static const int NUMANSWERS = 18;

static const double Answers2[NUMANSWERS][2] = { { 1, 2 },         { 3, 4 },  { 7, 7 }, { 4, 6 },   { 8, 9 },
                                                { -2, -2 },       { 2, 2 },  { 3, 8 }, { 7, 14 },  { 1.0 / 3.0, 1.0 / 2.0 },
                                                { 7, 7.0 / 2.0 }, { 4, 6 },  { 3, 1 }, { 12, 24 }, { 4, 24 },
                                                { 5, 11 },        { 26, 0 }, { 10, 0 } };

static const double Answers3[NUMANSWERS][3] = { { 1, 2, 5 },
                                                { 3, 4, 6 },
                                                { 7, 7, 7 },
                                                { 4, 6, 11 },
                                                { 8, 9, 12 },
                                                { -2, -2, -1 },
                                                { 2, 2, 1 },
                                                { 3, 8, 30 },
                                                { 7, 14, 35 },
                                                { 1.0 / 3.0, 1.0 / 2.0, 5.0 / 6.0 },
                                                { 7, 7.0 / 2.0, 7.0 / 5.0 },
                                                { 4, 6, 11 },
                                                { 3, 1, -4 },
                                                { 12, 24, 66 },
                                                { 4, 24, -66.0 / 4.0 },
                                                { 5, 11, 26 },
                                                { 251, 0, 0 },
                                                { 26, 0, 0 } };

static const double Answers4[NUMANSWERS][4] = { { 1, 2, 5, 7 },
                                                { 3, 4, 6, 8 },
                                                { 7, 7, 7, 7 },
                                                { 4, 6, 11, 15 },
                                                { 8, 9, 12, 14 },
                                                { -2, -2, -1, -1 },
                                                { 2, 2, 1, 1 },
                                                { 3, 8, 30, 56 },
                                                { 7, 14, 35, 49 },
                                                { 1.0 / 3.0, 1.0 / 2.0, 5.0 / 6.0, 7.0 / 8.0 },
                                                { 7, 7.0 / 2.0, 7.0 / 5.0, 7.0 / 7.0 },
                                                { 4, 6, 11, 15 },
                                                { 3, 1, -4, -8 },
                                                { 12, 24, 66, 120 },
                                                { 4, 24, -66.0 / 4.0, -15 },
                                                { 5, 11, 26, 38 },
                                                { 780, 0, 0, 0 },
                                                { 90, 0, 0, 0 } };

static const double Answers5[NUMANSWERS][5] = { { 1, 2, 5, 7, 9 },
                                                { 3, 4, 6, 8, 10 },
                                                { 7, 7, 7, 7, 7 },
                                                { 4, 6, 11, 15, 19 },
                                                { 8, 9, 12, 14, 16 },
                                                { -2, -2, -1, -1, -1 },
                                                { 2, 2, 1, 1, 1 },
                                                { 3, 8, 30, 56, 90 },
                                                { 7, 14, 35, 49, 63 },
                                                { 1.0 / 3.0, 1.0 / 2.0, 5.0 / 6.0, 7.0 / 8.0, 9.0 / 10.0 },
                                                { 7, 7.0 / 2.0, 7.0 / 5.0, 7.0 / 7.0, 7.0 / 9.0 },
                                                { 4, 6, 11, 15, 19 },
                                                { 3, 1, -4, -8, -12 },
                                                { 12, 24, 66, 120, 190 },
                                                { 4, 24, -66.0 / 4.0, -15, -190.0 / 12.0 },
                                                { 5, 11, 26, 38, 50 },
                                                { 1741, 0, 0, 0, 0 },
                                                { 234, 0, 0, 0, 0 } };

static const double* VAnswers[6] = {
  0, 0, (const double*)Answers2, (const double*)Answers3, (const double*)Answers4, (const double*)Answers5
};

template<typename T, int N> void VECTOR_N_TEST(TESTDEF::RETPAIR& __testret)
{
  T ans[NUMANSWERS][N];
  for(size_t i = 0; i < NUMANSWERS; ++i)
    for(int j = 0; j < N; ++j)
      ans[i][j] = (T)VAnswers[N][i * N + j];

  T at[N];
  T bt[N];
  TESTVALUES_To<T, N>(at, ans[0]);
  TESTVALUES_To<T, N>(bt, ans[1]);
  Vector<T, N> a(at);
  Vector<T, N> b(bt);
  Vector<T, N> c(7);
  Vector<T, N> d(b);
  Vector<T, N> e;
  TESTVECTOR(a, ans[0], __testret);
  TESTVECTOR(b, ans[1], __testret);
  TESTVECTOR(c, ans[2], __testret);
  TESTVECTOR(d, ans[1], __testret);
  e = a + b;
  TESTVECTOR(e, ans[3], __testret);
  e = c + a;
  TESTVECTOR(e, ans[4], __testret);
  e = a - b;
  TESTVECTOR(e, ans[5], __testret);
  e = b - a;
  TESTVECTOR(e, ans[6], __testret);
  e = a * b;
  TESTVECTOR(e, ans[7], __testret);
  e = c * a;
  TESTVECTOR(e, ans[8], __testret);
  e = a / b;
  TESTVECTOR(e, ans[9], __testret);
  e = c / a;
  TESTVECTOR(e, ans[10], __testret);
  a += b;
  TESTVECTOR(a, ans[11], __testret);
  c -= a;
  TESTVECTOR(c, ans[12], __testret);
  b *= a;
  TESTVECTOR(b, ans[13], __testret);
  b /= c;
  if(!std::is_unsigned<T>::value)
    TESTVECTOR(b, ans[14], __testret);
  e = (c - (a * ((T)2))).Abs();
  TESTVECTOR(e, ans[15], __testret);
  e = (c - (((T)2) * a)).Abs();
  TESTVECTOR(e, ans[15], __testret);
  T l = c.DistanceSq(a);
  TEST(l == ans[16][0]);
  l = c.Distance(a);
  TEST(l == FastSqrt<T>(ans[16][0]));
  l = c.Dot(c);
  TEST(l == ans[17][0]);
  l = c.Length();
  TEST(l == FastSqrt<T>(ans[17][0]));
  if(!std::is_integral<T>::value)
  {
    e = c.Normalize();
    for(size_t i = 0; i < N; ++i)
    {
      TEST(e.v[i] == (ans[12][i] / l));
    }
  }
}
template<typename T> void VECTOR2_CROSS_TEST(TESTDEF::RETPAIR& __testret)
{
  Vector<T, 2> v(4, 3);
  Vector<T, 2> v2(1, 2);
  T a = v.Cross(v2);
  TEST(a == (T)5);
}
template<typename T> void VECTOR3_CROSS_TEST(TESTDEF::RETPAIR& __testret)
{
  Vector<T, 3> v(4, 5, 6);
  Vector<T, 3> v2(1, 2, 3);
  Vector<T, 3> a = v.Cross(v2);
  T ans[3]       = { (T)3, (T)-6, (T)3 };
  for(size_t i = 0; i < 3; ++i)
  {
    TEST(a.v[i] == ans[i]);
  }
}

template<typename T, int M, int N, T (*op)(const T& l, const T& r)>
void MATRIX_COMPONENT_OP(const T (&l)[M][N], const T (&r)[M][N], T (&out)[M][N])
{
  for(size_t i = 0; i < M; ++i)
    for(int j = 0; j < N; ++j)
      out[i][j] = op(l[i][j], r[i][j]);
}

template<typename T> T MATRIX_OP_ADD(const T& l, const T& r) { return l + r; }
template<typename T> T MATRIX_OP_SUB(const T& l, const T& r) { return l - r; }
template<typename T> T MATRIX_OP_MUL(const T& l, const T& r) { return l * r; }
template<typename T> T MATRIX_OP_DIV(const T& l, const T& r) { return l / r; }

template<typename T, int M, int N, int P> void MATRIX_MULTIPLY(const T (&l)[M][N], const T (&r)[N][P], T (&out)[M][P])
{
  T a[M][N];
  T b[N][P];
  memcpy(a, l, sizeof(T) * M * N); // ensure if we pass in multiple non-distinct matrices we don't overwrite them.
  memcpy(b, r, sizeof(T) * N * P);
  bun_Fill(out);
  for(size_t i = 0; i < M; ++i)
  {
    for(int j = 0; j < P; ++j)
    {
      for(int k = 0; k < N; ++k)
        out[i][j] += a[i][k] * b[k][j];
    }
  }
}

template<typename T> bool MAT_OP_COMPFUNC(const T& l, const T& r) { return l == r; }
template<> bool MAT_OP_COMPFUNC<float>(const float& l, const float& r) { return fCompareSmall(l, r); }
template<> bool MAT_OP_COMPFUNC<double>(const double& l, const double& r) { return fCompareSmall(l, r); }

template<typename T, int M, int N, T (*op)(const T& l, const T& r)>
bool MATRIX_OP_TEST(const T (&x)[M][N], const T (&l)[M][N], const T (&r)[M][N])
{
  T out[M][N];
  MATRIX_COMPONENT_OP<T, M, N, op>(l, r, out);

  for(size_t i = 0; i < M; ++i)
    for(int j = 0; j < N; ++j)
      if(!MAT_OP_COMPFUNC(out[i][j], x[i][j]))
        return false;

  return true;
}

template<typename T, int M, int N, T (*op)(const T& l, const T& r)>
bool MATRIX_OP_TEST(const T (&x)[M][N], const T (&l)[M][N], const T scalar)
{
  T out[M][N];
  T* set = (T*)out;
  for(size_t i = 0; i < M * N; ++i)
    set[i] = scalar;
  return MATRIX_OP_TEST<T, M, N, op>(x, l, out);
}

template<typename T, int M, int N> BUN_FORCEINLINE static void MATRIX_DIAGONAL(const T (&v)[M < N ? M : N], T (&out)[M][N])
{
  bun_Fill(out);
  for(size_t i = 0; i < M && i < N; ++i)
    out[i][i] = v[i];
}

template<typename T, int M, int N> bool MATRIX_COMPARE(const T (&l)[M][N], const T (&r)[M][N], int diff = 1)
{
  for(size_t i = 0; i < M; ++i)
    for(int j = 0; j < N; ++j)
      if(!fCompareSmall(l[i][j], r[i][j], diff))
        return false;
  return true;
}

template<typename T, int M, int N> void MATRIX_M_N_TEST(TESTDEF::RETPAIR& __testret)
{
  Matrix<T, M, N> a;
  Matrix<T, M, N> b;
  Matrix<T, M, N> c;
  Matrix<T, M, N> d;
  Matrix<T, M, N> e;

  for(size_t i = 0; i < M; ++i)
    for(int j = 0; j < N; ++j)
      a.v[i][j] = (T)((i * M) + j + 1);
  for(size_t i = 0; i < M; ++i)
    for(int j = 0; j < N; ++j)
      b.v[i][j] = (T)((i * M) + j + (M * N) + 1);
  for(size_t i = 0; i < M; ++i)
    for(int j = 0; j < N; ++j)
      c.v[i][j] = (T)7;

  // Component operation tests
  e = a + b;
  TEST((MATRIX_OP_TEST<T, M, N, MATRIX_OP_ADD>(e.v, a.v, b.v)));
  e = b + a;
  TEST((MATRIX_OP_TEST<T, M, N, MATRIX_OP_ADD>(e.v, b.v, a.v)));
  e = a - b;
  TEST((MATRIX_OP_TEST<T, M, N, MATRIX_OP_SUB>(e.v, a.v, b.v)));
  e = b - c;
  TEST((MATRIX_OP_TEST<T, M, N, MATRIX_OP_SUB>(e.v, b.v, c.v)));
  e = b ^ a;
  TEST((MATRIX_OP_TEST<T, M, N, MATRIX_OP_MUL>(e.v, b.v, a.v)));
  e = c ^ b;
  TEST((MATRIX_OP_TEST<T, M, N, MATRIX_OP_MUL>(e.v, c.v, b.v)));
  e = a / b;
  TEST((MATRIX_OP_TEST<T, M, N, MATRIX_OP_DIV>(e.v, a.v, b.v)));
  e = b / a;
  TEST((MATRIX_OP_TEST<T, M, N, MATRIX_OP_DIV>(e.v, b.v, a.v)));
  e = a * (T)2;
  TEST(e == (a + a));
  e = (T)2 * a;
  TEST(e == (a + a));
  e = a + (T)2;
  TEST((MATRIX_OP_TEST<T, M, N, MATRIX_OP_ADD>(e.v, a.v, (T)2)));
  e = (T)2 + a;
  TEST((MATRIX_OP_TEST<T, M, N, MATRIX_OP_ADD>(e.v, a.v, (T)2)));
  e = a - (T)2;
  TEST((MATRIX_OP_TEST<T, M, N, MATRIX_OP_SUB>(e.v, a.v, (T)2)));
  e = (T)2 - a;
  TEST(e == -(a - (T)2));
  e = a / (T)2;
  TEST((MATRIX_OP_TEST<T, M, N, MATRIX_OP_DIV>(e.v, a.v, (T)2)));
  e = a + b;
  e -= b;
  TEST(e == a);
  e += a;
  TEST(e == (a * (T)2));
  e *= (T)-1;
  TEST(e == (a * ((T)-2)));
  e /= (T)2;
  TEST(e == (-a));
  d = a;
  TEST(d == a);

  // Function tests
  Matrix<T, N, M> t1;
  Matrix<T, N, M> t2;
  a.Transpose(t1.v);
  Matrix<T, M, N>::Transpose(a.v, t2.v);
  TEST(t1 == t2);
  Vector<T, (M < N ? M : N)> diag;
  for(size_t i = 0; i < (M < N ? M : N); ++i)
    diag.v[i] = (T)(i + 1);
  MATRIX_DIAGONAL<T, M, N>(diag.v, d.v);
  Matrix<T, M, N>::Diagonal(diag.v, e.v);
  if(d != e)
  {
    MATRIX_DIAGONAL<T, M, N>(diag.v, d.v);
    Matrix<T, M, N>::Diagonal(diag.v, e.v);
  }
  TEST(d == e);

  diag = Vector<T, (M < N ? M : N)>(1);
  MATRIX_DIAGONAL<T, M, N>(diag.v, d.v);
  Matrix<T, M, N>::Identity(e.v);
  TEST(d == e);

  // Matrix Multiplication tests
  Vector<T, N> r1;
  Vector<T, M> rr1;
  Vector<T, M> rrr1;
  for(size_t i = 0; i < N; ++i)
    r1.v[i] = (T)(i + 1);
  Vector<T, M> r2;
  Vector<T, N> rr2;
  Vector<T, N> rrr2;
  for(size_t i = 0; i < M; ++i)
    r2.v[i] = (T)(i + 1);

  Matrix<T, N, N> s1;
  Matrix<T, N, N> ss1;
  Matrix<T, M, M> s2;
  Matrix<T, M, M> ss2;

  s2 = a * t2;
  MATRIX_MULTIPLY<T, M, N, M>(a.v, t2.v, ss2.v);
  TEST(ss2 == s2);
  s1 = t1 * a;
  MATRIX_MULTIPLY<T, N, M, N>(t1.v, a.v, ss1.v);
  TEST(ss1 == s1);
  rr2 = r2 * b;
  MATRIX_MULTIPLY<T, 1, M, N>(r2.v_row, b.v, rrr2.v_row);
  TEST(rr2 == rrr2);
  rr1 = b * r1;
  MATRIX_MULTIPLY<T, M, N, 1>(b.v, r1.v_column, rrr1.v_column);
  TEST(rr1 == rrr1);
  MatrixMultiply<T, M, M, M>(ss2.v, ss2.v, s2.v);
  MatrixMultiply<T, M, M, M>(ss2.v, ss2.v, ss2.v);
  TEST(ss2 == s2);
  MATRIX_MULTIPLY<T, M, M, M>(ss2.v, ss2.v, s2.v);
  MATRIX_MULTIPLY<T, M, M, M>(ss2.v, ss2.v, ss2.v);
  TEST(ss2 == s2);
}

template<int M, int N> struct VECTOR_DUPLICATE
{
  static void test(TESTDEF::RETPAIR& __testret)
  {
    MATRIX_M_N_TEST<float, M, N>(__testret);
    MATRIX_M_N_TEST<double, M, N>(__testret);
    MATRIX_M_N_TEST<int, M, N>(__testret);
    MATRIX_M_N_TEST<int64_t, M, N>(__testret);

    VECTOR_DUPLICATE<M, N - 1>::test(__testret);
    VECTOR_DUPLICATE<M - 1, N - 1>::test(__testret);
  }
};
template<int M> struct VECTOR_DUPLICATE<M, 0>
{
  static void test(TESTDEF::RETPAIR& __testret) {}
};
template<int N> struct VECTOR_DUPLICATE<0, N>
{
  static void test(TESTDEF::RETPAIR& __testret) {}
};
template<> struct VECTOR_DUPLICATE<0, 0>
{
  static void test(TESTDEF::RETPAIR& __testret) {}
};

TESTDEF::RETPAIR test_VECTOR()
{
  BEGINTEST;
  VECTOR_N_TEST<float, 2>(__testret);
  VECTOR_N_TEST<double, 2>(__testret);
  VECTOR_N_TEST<int, 2>(__testret);
  VECTOR_N_TEST<uint32_t, 2>(__testret);
  VECTOR_N_TEST<int64_t, 2>(__testret);
  VECTOR_N_TEST<float, 3>(__testret);
  VECTOR_N_TEST<double, 3>(__testret);
  VECTOR_N_TEST<int, 3>(__testret);
  VECTOR_N_TEST<uint32_t, 3>(__testret);
  VECTOR_N_TEST<int64_t, 3>(__testret);
  VECTOR_N_TEST<float, 4>(__testret);
  VECTOR_N_TEST<double, 4>(__testret);
  VECTOR_N_TEST<int, 4>(__testret);
  VECTOR_N_TEST<uint32_t, 4>(__testret);
  VECTOR_N_TEST<int64_t, 4>(__testret);
  VECTOR_N_TEST<float, 5>(__testret);
  VECTOR_N_TEST<double, 5>(__testret);
  VECTOR_N_TEST<int, 5>(__testret);
  VECTOR_N_TEST<uint32_t, 5>(__testret);
  VECTOR_N_TEST<int64_t, 5>(__testret);

  VECTOR2_CROSS_TEST<float>(__testret);
  VECTOR2_CROSS_TEST<double>(__testret);
  VECTOR2_CROSS_TEST<int>(__testret);
  VECTOR2_CROSS_TEST<uint32_t>(__testret);
  VECTOR2_CROSS_TEST<int64_t>(__testret);
  VECTOR3_CROSS_TEST<float>(__testret);
  VECTOR3_CROSS_TEST<double>(__testret);
  VECTOR3_CROSS_TEST<int>(__testret);
  VECTOR3_CROSS_TEST<uint32_t>(__testret);
  VECTOR3_CROSS_TEST<int64_t>(__testret);

  { // Verify the validity of our testing functions
    Matrix<int, 2, 3> u = { 1, 2, 3, 4, 5, 6 };
    Matrix<int, 3, 2> v = { 1, 2, 3, 4, 5, 6 };
    Matrix<int, 2, 3> w = { 12, 11, 10, 9, 8, 7 };
    Matrix<int, 2, 2> uv;
    int uv_ans[4] = { 22, 28, 49, 64 };
    Matrix<int, 3, 3> vu;
    int vu_ans[9] = { 9, 12, 15, 19, 26, 33, 29, 40, 51 };
    Matrix<int, 2, 3> w2;

    MATRIX_MULTIPLY<int, 2, 3, 2>(u.v, v.v, uv.v);
    TESTARRAY(uv_ans, return (((int*)&uv)[i] == uv_ans[i]););
    MATRIX_MULTIPLY<int, 3, 2, 3>(v.v, u.v, vu.v);
    TESTARRAY(vu_ans, return (((int*)&vu)[i] == vu_ans[i]););

    int w_ans[6] = { 11, 9, 7, 5, 3, 1 };
    MATRIX_COMPONENT_OP<int, 2, 3, MATRIX_OP_SUB>(w.v, u.v, w2.v);
    TESTARRAY(w_ans, return (((int*)&w2)[i] == w_ans[i]););
    MATRIX_COMPONENT_OP<int, 2, 3, MATRIX_OP_SUB>(u.v, w.v, w.v);
    TESTARRAY(w_ans, return (((int*)&w)[i] == (-w_ans[i])););
    int w_ans2[6]       = { 1, 0, 0, 0, 1, 0 };
    Vector<int, 2> diag = { 1, 1 };
    MATRIX_DIAGONAL<int, 2, 3>(diag.v, w.v);
    TESTARRAY(w_ans2, return (((int*)&w)[i] == w_ans2[i]););
  }

  // Now that we know our test functions are valid, test all possible matrix configurations up to 4x4
  VECTOR_DUPLICATE<4, 4>::test(__testret);

  // Test matrix transform functions
  {
    Matrix<int64_t, 2, 2> a = { 1, 2, 3, 4 };
    TEST(a.Determinant() == -2);
    Matrix<double, 3, 3> b = { 2, 2, 3, 4, 5, 6, 7, 8, 9 };
    TEST(b.Determinant() == -3.0);
    Matrix<float, 3, 3> bf = b;
    TEST(bf.Determinant() == -3.0f);
    Matrix<int, 4, 4> c = { 2, 2, 3, 4, 5, 0, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
    TEST(c.Determinant() == 24);
    Matrix<float, 4, 4> cf(c);
    TEST(cf.Determinant() == 24.0f);

    Matrix<float, 2, 2> d_ans = { -2, 1, 1.5f, -0.5f };
    Matrix<float, 2, 2> d(a);
    d.Inverse(d.v);
    TEST(MATRIX_COMPARE(d.v, d_ans.v, 1000));
    Matrix<float, 3, 3> e_ans = { 1, -2, 1, -2, 1, 0, 1, 2.0f / 3.0f, -2.0f / 3.0f };
    Matrix<float, 3, 3> e     = b;
    e.Inverse(e.v);
    TEST(MATRIX_COMPARE(e.v, e_ans.v, 1000));
    Matrix<double, 4, 4> g_ans = { 1,  0,       -3,       2,        0, -1.0 / 6, 1.0 / 3,    -1.0 / 6,
                                   -3, 1.0 / 3, 13.0 / 3, -8.0 / 3, 2, -1.0 / 6, -23.0 / 12, 13.0 / 12 };
    Matrix<float, 4, 4> f_ans  = g_ans;
    Matrix<double, 4, 4> g     = c;
    Matrix<float, 4, 4> f      = g;
    g.Inverse(g.v);
    Matrix<float, 4, 4> ff;
    f.Inverse(ff.v);
    TEST(f.Determinant() == 24.0f);
    TEST(MATRIX_COMPARE(ff.v, f_ans.v, 1000));
    f = g;
    TEST(MATRIX_COMPARE(f.v, f_ans.v, 1000));

    Matrix<float, 2, 2> af_ans = { 0, -1, 1, 0 };
    Matrix<float, 2, 2> af     = a;
    Matrix<float, 2, 2>::Rotation(PI_HALFf, d.v);
    Matrix<float, 2, 2>::Rotation_T(PI_HALFf, af);
    af.Transpose(af.v);
    TEST(af == d);
    TEST(MATRIX_COMPARE(af.v, af_ans.v, 1000));

    Matrix<float, 3, 3> f1_ans = { 0, -1, 0, 1, 0, 0, 0, 0, 1 };
    Matrix<float, 3, 3> f1;
    Matrix<float, 3, 3> f2;
    Matrix<float, 3, 3> f3;
    Matrix<float, 3, 3>::AffineRotation(PI_HALFf, f2.v);
    Matrix<float, 3, 3>::AffineRotation_T(PI_HALFf, f3);
    f3.Transpose(f1.v);
    TEST(f2 == f1);
    TEST(MATRIX_COMPARE(f1.v, f1_ans.v, 10));
    Matrix<float, 3, 3>::AffineScaling(2, -1, f1);
    Matrix<float, 3, 3>::Diagonal(2, -1, 1, f2);
    TEST(f2 == f1);
    Matrix<float, 3, 3> f2_ans = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
    Matrix<float, 3, 3>::Identity(f1);
    TEST(f1 == f2_ans);
    Matrix<float, 3, 3> f3_ans = { 1, 0, 2, 0, 1, -3, 0, 0, 1 };
    Matrix<float, 3, 3>::Translation(2, -3, f1.v);
    Matrix<float, 3, 3>::Translation_T(2, -3, f2);
    f2.Transpose(f3.v);
    TEST(f1 == f3);
    TEST(f1 == f3_ans);
    Matrix<float, 3, 3>::AffineRotation(0, f2.v);
    Matrix<float, 3, 3>::AffineRotation_T(0, f3);
    Matrix<float, 3, 3>::Identity(f1);
    TEST(f1 == f2);
    TEST(f1 == f3);

    // Rotate (1,0) 90 degrees, then translate it (3,1), which should yield (3,2)
    // When using the canonical matrix layouts (not the transposed versions), matrix multiplications are reversed:
    // M_T*M_R*V, where V is treated as a column vector and is always on the rightmost side regardless of the operations
    // being performed.
    Matrix<float, 3, 3>::AffineRotation(PI_HALFf, f1); // First rotate
    Matrix<float, 3, 3>::Translation(3, 1, f2);        // Then translate
    Vector<float, 3> V = {
      1, 0, 1
    }; // The third element is the projective dimension, which must be set to 1 for affine transforms.
    Vector<float, 3> V2    = f2 * f1 * V; // Multiply in reverse order of operations
    Vector<float, 3> V_ans = { 3, 2, 1 };
    TEST(V2 == V_ans);

    // When using transposed versions (this is what DirectX does), matrix multiplications are in the order of the
    // operations. This is because (AB)^T = B^T * A^T. So we get: V*M_R*M_T, where V is treated as a ROW vector and is
    // always on the leftmost side.
    Matrix<float, 3, 3>::AffineRotation_T(PI_HALFf, f1); // First rotate
    Matrix<float, 3, 3>::Translation_T(3, 1, f2);        // Then translate
    V  = { 1, 0, 1 }; // The third element is the projective dimension, which must be set to 1 for affine transforms.
    V2 = V * f1 * f2; // Multiply by the order of operations
    TEST(V2 == V_ans);
    f3 = f1 * f2;
    f1 *= f2;
    TEST(MATRIX_COMPARE(f1.v, f3.v, 100));
    V2 = V * f1;
    TEST(V2 == V_ans);

    Matrix<float, 3, 3>::AffineTransform_T(3, 1, PI_HALFf, 0, 0, f3);
    TEST(MATRIX_COMPARE(f1.v, f3.v, 100));

    Matrix<float, 3, 3>::Translation(4, 5, f1);   // M_rc
    Matrix<float, 3, 3>::AffineRotation(3, f2);   // M_r
    Matrix<float, 3, 3>::Translation(1, 2, f3.v); // M_t
    f3 = f3 * f1 * f2 * f1.Inverse();             // (M_rc)^-1 * M_r * M_rc * M_t (reversed)

    Matrix<float, 3, 3>::AffineTransform(1, 2, 3, 4, 5, f1.v);
    TEST(MATRIX_COMPARE(f1.v, f3.v, 100));

    Matrix<float, 3, 3>::Translation_T(4, 5, f1);   // M_rc
    Matrix<float, 3, 3>::AffineRotation_T(3, f2);   // M_r
    Matrix<float, 3, 3>::Translation_T(1, 2, f3.v); // M_t
    f3 = f1.Inverse() * f2 * f1 * f3;               // (M_rc)^-1 * M_r * M_rc * M_t

    Matrix<float, 3, 3>::AffineTransform_T(1, 2, 3, 4, 5, f1.v);
    Matrix<float, 3, 3>::AffineTransform(1, 2, 3, 4, 5, f2.v);
    TEST(MATRIX_COMPARE(f1.v, f3.v, 100));
    f1.Transpose(f1);
    TEST(MATRIX_COMPARE(f1.v, f2.v, 100));
  }

  {
    Matrix<float, 4, 4> m1;
    Matrix<float, 4, 4> m2;
    Matrix<float, 4, 4> m3;
    Matrix<float, 4, 4> m4;
    Matrix<float, 4, 4> m1_ansx = { 1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1 };
    Matrix<float, 4, 4> m1_ansy = { 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 0, 1 };
    Matrix<float, 4, 4> m1_ansz = { 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

    Matrix<float, 4, 4>::AffineRotationZ(PI_HALFf, m2.v);
    Matrix<float, 4, 4>::AffineRotationZ_T(PI_HALFf, m3);
    m3.Transpose(m1.v);
    TEST(m2 == m1);
    TEST(MATRIX_COMPARE(m1.v, m1_ansz.v, 10));
    Matrix<float, 4, 4>::AffineRotationY(PI_HALFf, m2.v);
    Matrix<float, 4, 4>::AffineRotationY_T(PI_HALFf, m3);
    m3.Transpose(m1.v);
    TEST(m2 == m1);
    TEST(MATRIX_COMPARE(m1.v, m1_ansy.v, 10));
    Matrix<float, 4, 4>::AffineRotationX(PI_HALFf, m2.v);
    Matrix<float, 4, 4>::AffineRotationX_T(PI_HALFf, m3);
    m3.Transpose(m1.v);
    TEST(m2 == m1);
    TEST(MATRIX_COMPARE(m1.v, m1_ansx.v, 10));
    Matrix<float, 4, 4>::AffineScaling(2, -1, 3, m1);
    Matrix<float, 4, 4>::Diagonal(2, -1, 3, 1, m2);
    TEST(m2 == m1);
    Matrix<float, 4, 4> m2_ans = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
    Matrix<float, 4, 4>::Identity(m1);
    TEST(m1 == m2_ans);
    Matrix<float, 4, 4> m3_ans = { 1, 0, 0, 2, 0, 1, 0, -3, 0, 0, 1, 4, 0, 0, 0, 1 };
    Matrix<float, 4, 4>::Translation(2, -3, 4, m1.v);
    Matrix<float, 4, 4>::Translation_T(2, -3, 4, m2);
    m2.Transpose(m3.v);
    TEST(m1 == m3);
    TEST(m1 == m3_ans);
    Matrix<float, 4, 4>::AffineRotationZ(0, m2.v);
    Matrix<float, 4, 4>::AffineRotationZ_T(0, m3);
    Matrix<float, 4, 4>::Identity(m1);
    TEST(m1 == m2);
    TEST(m1 == m3);

    // Rotate (1,0) 90 degrees, then translate it (3,1), which should yield (3,2)
    // When using the canonical matrix layouts (not the transposed versions), matrix multiplications are reversed:
    // M_T*M_R*V, where V is treated as a column vector and is always on the rightmost side regardless of the operations
    // being performed.
    Matrix<float, 4, 4>::AffineRotationZ(PI_HALFf, m1); // First rotate
    Matrix<float, 4, 4>::Translation(3, 1, 4, m2);      // Then translate
    Vector<float, 4> V = {
      1, 0, 0, 1
    }; // The third element is the projective dimension, which must be set to 1 for affine transforms.
    Vector<float, 4> V2    = m2 * m1 * V; // Multiply in reverse order of operations
    Vector<float, 4> V_ans = { 3, 2, 4, 1 };
    TEST(V2 == V_ans);

    // When using transposed versions (this is what DirectX does), matrix multiplications are in the order of the
    // operations. This is because (AB)^T = B^T * A^T. So we get: V*M_R*M_T, where V is treated as a ROW vector and is
    // always on the leftmost side.
    Matrix<float, 4, 4>::AffineRotationZ_T(PI_HALFf, m1); // First rotate
    Matrix<float, 4, 4>::Translation_T(3, 1, 4, m2);      // Then translate
    V  = { 1, 0, 0, 1 }; // The third element is the projective dimension, which must be set to 1 for affine transforms.
    V2 = V * m1 * m2;    // Multiply by the order of operations
    TEST(V2 == V_ans);
    m3 = m1 * m2;
    m1 *= m2;
    TEST(MATRIX_COMPARE(m1.v, m3.v, 100));
    V2 = V * m1;
    TEST(V2 == V_ans);

    Matrix<float, 4, 4>::AffineTransform_T(3, 1, 4, PI_HALFf, 0, 0, m3);
    TEST(MATRIX_COMPARE(m1.v, m3.v, 100));

    Matrix<float, 4, 4>::Translation(5, 6, 0, m1);   // M_rc
    Matrix<float, 4, 4>::AffineRotationZ(4, m2);     // M_r
    Matrix<float, 4, 4>::Translation(1, 2, 3, m3.v); // M_t
    m1.Inverse(m4);
    m3 = m3 * m1 * m2 * m4; // (M_rc)^-1 * M_r * M_rc * M_t (reversed)

    Matrix<float, 4, 4>::AffineTransform(1, 2, 3, 4, 5, 6, m1.v);
    TEST(MATRIX_COMPARE(m1.v, m3.v, 100));

    Matrix<float, 4, 4>::Translation_T(5, 6, 0, m1);   // M_rc
    Matrix<float, 4, 4>::AffineRotationZ_T(4, m2);     // M_r
    Matrix<float, 4, 4>::Translation_T(1, 2, 3, m3.v); // M_t
    m4 = m1.Inverse();
    m3 = m4 * m2 * m1 * m3; // (M_rc)^-1 * M_r * M_rc * M_t

    Matrix<float, 4, 4>::AffineTransform_T(1, 2, 3, 4, 5, 6, m1.v);
    Matrix<float, 4, 4>::AffineTransform(1, 2, 3, 4, 5, 6, m2.v);
    TEST(MATRIX_COMPARE(m1.v, m3.v, 100));
    m1.Transpose(m1);
    TEST(MATRIX_COMPARE(m1.v, m2.v, 100));
  }

  // Test n-dimensional vector functions
  alignas(16) float v[4] = { 2, -3, 4, -5 };
  alignas(16) float u[4] = { 1, -2, -1, 2 };
  alignas(16) float w[4] = { 0, 0, 0, 0 };
  TEST(fCompare(NVectorDistanceSq(v, w), 54.0f));
  TEST(fCompare(NVectorDistance(v, w), 7.34846922835f));
  TEST(fCompare(NVectorDistance(u, v), NVectorDistance(v, u)));
  TEST(fCompare(NVectorDistance(u, v), 8.717797887f));
  TEST(fCompare(NTriangleArea(v, u, w), 11.22497216f, 100));
  TEST(fCompare(NTriangleArea(u, v, w), 11.22497216f, 100));
  TEST(fSmall(NVectorDot(u, w)));
  TEST(fCompare(NVectorDot(u, v), -6.0f));
  // NVectAdd(v, w, w);
  // TESTFOUR(w, v[0], v[1], v[2], v[3]);
  // NVectAdd(v, 3.0f, w);
  // TESTFOUR(w, 5, 0, 7, -2);
  // NVectAdd(v, u, w);
  // TESTRELFOUR(w, 3, -5, 3, -3);
  // NVectSub(v, 3.0f, w);
  // TESTRELFOUR(w, -1, -6, 1, -8);
  // NVectSub(v, u, w);
  // TESTRELFOUR(w, 1, -1, 5, -7);
  // NVectMul(v, 2.5f, w);
  // TESTRELFOUR(w, 5, -7.5f, 10, -12.5f);
  // NVectMul(v, u, w);
  // TESTRELFOUR(w, 2, 6, -4, -10);
  // NVectDiv(v, 2.5f, w);
  // TESTRELFOUR(w, 0.8f, -1.2f, 1.6f, -2);
  // NVectDiv(v, u, w);
  // TESTRELFOUR(w, 2, 1.5f, -4, -2.5f);

  // alignas(16) float m1[4][4]={ { 1, 2, 4, 8 }, { 16, 32, 64, 128 }, { -1, -2, -4, -8 }, { -16, -32, -64, -128 } };
  // alignas(16) float m2[4][4]={ { 8, 4, 2, 1 }, { 16, 32, 64, 128 }, { -1, -32, -4, -16 }, { -8, -2, -64, -128 } };
  // alignas(16) float m3[4][4]={ 0 };
  // alignas(16) float m4[4][4]={ 0 };
  // alignas(16) float m5[4][4]={ { -28, -76, -398, -831 }, { -448, -1216, -6368, -13296 }, { 28, 76, 398, 831 }, { 448,
  // 1216, 6368, 13296 } }; alignas(16) float m6[4][4]={ { 54, 108, 216, 432 }, { -1584, -3168, -6336, -12672 }, { -253,
  // -506, -1012, -2024 }, { 2072, 4144, 8288, 16576 } }; Mult4x4(m3, m1, m2); Mult4x4(m4, m2, m1); TEST(!memcmp(m3, m5,
  // sizeof(float)*4*4)); TEST(!memcmp(m4, m6, sizeof(float)*4*4));
  ENDTEST;
}