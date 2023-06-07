// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/algo.h"
#include "buntils/vector.h"
#include <functional>

using namespace bun;

TESTDEF::RETPAIR test_algo()
{
  BEGINTEST;
  {
    int a[] = { -5,-1,0,1,1,1,1,6,8,8,9,26,26,26,35 };
    bool test;
    for(int i = -10; i < 40; ++i)
    {
      test = (BinarySearchBefore<int, uint32_t, CompT<int>, 15>(a, i) == (uint32_t)((std::upper_bound(std::begin(a), std::end(a), i) - a) - 1));
      TEST(test);
      TEST((BinarySearchAfter<int, uint32_t, CompT<int>, 15>(a, i) == (std::lower_bound(std::begin(a), std::end(a), i) - a)));
    }

    int b[2] = { 2,3 };
    int d[1] = { 1 };
    TEST((BinarySearchExact<int, uint32_t, CompT<int>, 2>(b, 1) == -1));
    TEST((BinarySearchExact<int, uint32_t, CompT<int>, 2>(b, 2) == 0));
    TEST((BinarySearchExact<int, uint32_t, CompT<int>, 2>(b, 3) == 1));
    TEST((BinarySearchExact<int, uint32_t, CompT<int>, 2>(b, 4) == -1));
    TEST((BinarySearchExact<int, int, uint32_t, CompT<int>>(0, 0, 0, 0) == -1));
    TEST((BinarySearchExact<int, int, uint32_t, CompT<int>>(0, -1, 0, 0) == -1));
    TEST((BinarySearchExact<int, int, uint32_t, CompT<int>>(0, 1, 0, 0) == -1));
    TEST((BinarySearchExact<int, uint32_t, CompT<int>, 1>(d, -1) == -1));
    TEST((BinarySearchExact<int, uint32_t, CompT<int>, 1>(d, 1) == 0));
    TEST((BinarySearchExact<int, int, uint32_t, CompT<int>>(d, 1, 1, 1) == -1));
    TEST((BinarySearchExact<int, uint32_t, CompT<int>, 1>(d, 2) == -1));
  }

  {
    NormalZig<128> zig; // TAKE OFF EVERY ZIG!
    float rect[4] = { 0,100,1000,2000 };
    StochasticSubdivider<float>(rect,
      [](size_t d, const float(&r)[4]) -> double { return 1.0 - 10.0 / (d + 10.0) + 0.5 / (((r[2] - r[0])*(r[3] - r[1])) + 0.5); },
      [](const float(&r)[4]) {},
      [&](size_t d, const float(&r)[4]) -> double { return zig(); });

    //TEST(QuadraticBSpline<double,double>(1.0,2.0,4.0,8.0)==4.0);
    double res = CubicBSpline<double, double>(0.0, 2.0, 4.0, 8.0, 16.0);
    TEST(res == 4.0);
    res = CubicBSpline<double, double>(1.0, 2.0, 4.0, 8.0, 16.0);
    TEST(res == 8.0);

    uint64_t state[17];
    GenXor1024Seed(9, state);
    TEST(xorshift1024star(state) == 8818441658795715037ULL);
    TEST(xorshift1024star(state) == 1327952074883251404ULL);
    TEST(xorshift1024star(state) == 15247190286870378554ULL);
    TEST(xorshift1024star(state) == 7401833066192044893ULL);
    TEST(xorshift1024star(state) == 17572132563546066374ULL);
    TEST(xorshift1024star(state) == 11245070064711388831ULL);

    XorshiftRand(234);
  }

  {
    const char b64test[] = "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.";
    const char b64out[] = "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4";

    Str b64s;
    b64s.resize(Base64Encode((uint8_t*)b64test, sizeof(b64test) - 1, 0));
    Base64Encode((uint8_t*)b64test, sizeof(b64test) - 1, b64s.UnsafeString());
    TEST(!memcmp(b64out, b64s.c_str(), sizeof(b64out) - 1));
    Str b64sd;
    b64sd.resize(Base64Decode(b64s.UnsafeString(), b64s.size(), 0));
    Base64Decode(b64s.UnsafeString(), b64s.size(), (uint8_t*)b64sd.UnsafeString());
    TEST(!memcmp(b64test, b64sd.c_str(), sizeof(b64test) - 1));

    uint8_t b64[256];
    uint8_t bout[256];
    for(size_t max = 256; max > 1; --max)
    {
      for(size_t i = 0; i < max; ++i) b64[i] = (uint8_t)i;
      Str str;
      str.resize(Base64Encode(b64, max, 0));
      Base64Encode(b64, max, str.UnsafeString());
      TEST(Base64Decode(str.c_str(), str.size(), 0) == max);
      Base64Decode(str.c_str(), str.size(), bout);
      for(size_t i = 0; i < max; ++i) TEST(bout[i] == i);
    }
  }

  {
    auto fn = [](double x) -> double { return x*x*x - 2 * x*x - 11 * x + 12; };
    auto dfn = [](double x) -> double { return 3 * x*x - 4 * x - 11; };
    double r = 0.0; // Test boundary basin behavior of newton raphson to ensure it's actually doing what we expect. Because this is an edge case, we require a lot more iterations than normal
    NewtonRaphson<double>(r, 2.35287527, fn, dfn, DBL_EPS, 50);
    TEST(r == 4.0);
    NewtonRaphson<double>(r, 2.35284172, fn, dfn, DBL_EPS, 50);
    TEST(r == -3.0);
    NewtonRaphson<double>(r, 2.35283735, fn, dfn, DBL_EPS, 50);
    TEST(r == 4.0);
    NewtonRaphson<double>(r, 2.352836327, fn, dfn, DBL_EPS, 50);
    TEST(r == -3.0);
    NewtonRaphson<double>(r, 2.352836323, fn, dfn, DBL_EPS, 50);
    TEST(r == 1.0);

    r = NewtonRaphsonBisection<double>(2.35287527, -10, 10, fn, dfn, DBL_EPS, 50);
    TEST(r == 4.0); // These all converge on 4 because the bisection introduces stability to the algorithm.
    r = NewtonRaphsonBisection<double>(2.35284172, -10, 10, fn, dfn, DBL_EPS, 50);
    TEST(r == 4.0);
    r = NewtonRaphsonBisection<double>(2.35283735, -10, 10, fn, dfn, DBL_EPS, 50);
    TEST(r == 4.0);
    r = NewtonRaphsonBisection<double>(2.352836327, -10, 10, fn, dfn, DBL_EPS, 50);
    TEST(r == 4.0);
    r = NewtonRaphsonBisection<double>(2.352836323, -10, 10, fn, dfn, DBL_EPS, 50);
    TEST(r == 4.0);
  }

  {
    Vector<float, 2> p0 = { 0,0 };
    Vector<float, 2> p1 = { 1,0 };
    Vector<float, 2> p2 = { 1,1 };
    Vector<float, 2> A = p1 - p0;
    Vector<float, 2> B = p0 - (2.0f * p1) + p2;
    Vector<float, 2> M = p0 - Vector<float, 2>(1, 0);

    float a = B.Dot(B);
    float b = 3 * A.Dot(B);
    float c = 2 * A.Dot(A) + M.Dot(B);
    float d = M.Dot(A);

    float r[3];
    int n = solveCubic<float>(a, b, c, d, r);
    TEST(r[0] == 0.5);
  }

  uint32_t nsrc[] = { 0,1,2,3,4,5,10,13,21,2873,3829847,2654435766 };
  uint32_t num[] = { 1,2,4,5,7,8,17,21,34,4647,6193581,4292720341 };
  std::transform(std::begin(nsrc), std::end(nsrc), nsrc, &fbnext<uint32_t>);
  TESTARRAY(nsrc, return nsrc[i] == num[i];)

  //int testmap = { 1, 2, 3, 4, 5, 6, 7, 8, 9, -1 };
  //DynArray<int> array;
  //filter(std::begin(testmap), std::end(testmap), array);
  //DynArray<int*> array2;
  //filter(std::begin(testmap), std::end(testmap), array2);

  // This can be used to test the distribution of a random engine to verify it is uniform
  //XorshiftEngine<double> dtest;
  /*double vals[10000];
  for(size_t i = 0; i < sizeof(vals) / sizeof(double); ++i)
    vals[i] = bun_RandReal(0.0, 1.0);
    //vals[i] = dtest();

  double vmin = vals[0];
  double vmax = vals[0];
  double vavg = vals[0];
  for(size_t i = 1; i < sizeof(vals) / sizeof(double); ++i)
  {
    vmin = std::min(vmin, vals[i]);
    vmax = std::max(vmax, vals[i]);
    vavg = bun_Avg(vavg, vals[i], i);
  }

  double chunk = vmax / 25.0 - vmin / 25.0;
  int buckets[25] = { 0 };

  for(size_t i = 0; i < sizeof(vals) / sizeof(double); ++i)
  {
    int k = (int)floor((vals[i] / chunk) - (vmin / chunk));
    if(k < 0) k = 0;
    if(k > 24) k = 24;
    buckets[k]++;
  }

  std::cout << std::endl;
  for(size_t i = 0; i < sizeof(buckets) / sizeof(int); ++i)
    std::cout << buckets[i] << std::endl;*/

  ENDTEST;
}