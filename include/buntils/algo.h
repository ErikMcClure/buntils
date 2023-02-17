// Copyright �2018 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BUN_ALGO_H__
#define __BUN_ALGO_H__

#include "compare.h"
#include "sseVec.h"
#include "XorshiftEngine.h"
#include "Array.h"
#include <algorithm>
#include <utility>
#include <random>
#include <array>
#ifdef BUN_COMPILER_GCC
#include <alloca.h>
#endif

namespace bun {
  namespace internal {
  // A generalization of the binary search that allows for auxiliary arguments to be passed into the comparison function
    template<typename T, typename D, typename CT_, bool(*CompEQ)(const char&, const char&), char CompValue, typename... Args>
    struct binsearch_aux_t
    {
      template<char(*CompF)(const D&, const T&, Args...)>
      inline static CT_ BinarySearchNear(const T* arr, const D& data, CT_ first, CT_ last, Args... args)
      {
        typename std::make_signed<CT_>::type c = last - first; // Must be a signed version of whatever CT_ is
        CT_ c2; //No possible operation can make this negative so we leave it as possibly unsigned.
        CT_ m;

        while(c > 0)
        {
          c2 = (c >> 1); // >> 1 is valid here because we check to see that c > 0 beforehand.
          m = first + c2;

          //if(!(_Val < *_Mid))
          if((*CompEQ)((*CompF)(data, arr[m], args...), CompValue))
          {	// try top half
            first = m + 1;
            c -= c2 + 1;
          }
          else
            c = c2;
        }

        return first;
      }
    };
  }

  // Performs a binary search on "arr" between first and last. if CompEQ=NEQ and char CompValue=-1, uses an upper bound, otherwise uses lower bound.
  template<typename T, typename D, typename CT_, char(*CompF)(const D&, const T&), bool(*CompEQ)(const char&, const char&), char CompValue>
  BUN_FORCEINLINE CT_ BinarySearchNear(const T* arr, const D& data, CT_ first, CT_ last) { return internal::binsearch_aux_t<T, D, CT_, CompEQ, CompValue>::template BinarySearchNear<CompF>(arr, data, first, last); }

  // Either gets the element that matches the value in question or one immediately before the closest match. Could return an invalid -1 value.
  template<typename T, typename CT_, char(*CompF)(const T&, const T&)>
  BUN_FORCEINLINE CT_ BinarySearchBefore(const T* arr, const T& data, CT_ first, CT_ last) { return BinarySearchNear<T, T, CT_, CompF, CompT_NEQ<char>, -1>(arr, data, first, last) - 1; }

  template<typename T, typename CT_, char(*CompF)(const T&, const T&)>
  BUN_FORCEINLINE CT_ BinarySearchBefore(const T* arr, CT_ length, const T& data) { return BinarySearchNear<T, T, CT_, CompF, CompT_NEQ<char>, -1>(arr, data, 0, length) - 1; }

  template<typename T, typename CT_, char(*CompF)(const T&, const T&), CT_ I>
  BUN_FORCEINLINE CT_ BinarySearchBefore(const T(&arr)[I], const T& data) { return BinarySearchBefore<T, CT_, CompF>(arr, I, data); }

  template<typename T, typename CT_, char(*CompF)(const T&, const T&), CT_ I>
  BUN_FORCEINLINE CT_ BinarySearchBefore(const std::array<T, I>& arr, const T& data) { return BinarySearchBefore<T, CT_, CompF>(arr, I, data); }

  // Either gets the element that matches the value in question or one immediately after the closest match.
  template<typename T, typename CT_, char(*CompF)(const T&, const T&)>
  BUN_FORCEINLINE CT_ BinarySearchAfter(const T* arr, const T& data, CT_ first, CT_ last) { return BinarySearchNear<T, T, CT_, CompF, CompT_EQ<char>, 1>(arr, data, first, last); }

  template<typename T, typename CT_, char(*CompF)(const T&, const T&)>
  BUN_FORCEINLINE CT_ BinarySearchAfter(const T* arr, CT_ length, const T& data) { return BinarySearchNear<T, T, CT_, CompF, CompT_EQ<char>, 1>(arr, data, 0, length); }

  template<typename T, typename CT_, char(*CompF)(const T&, const T&), CT_ I>
  BUN_FORCEINLINE CT_ BinarySearchAfter(const T(&arr)[I], const T& data) { return BinarySearchAfter<T, CT_, CompF>(arr, I, data); }
  template<typename T, typename CT_, char(*CompF)(const T&, const T&), CT_ I>
  BUN_FORCEINLINE CT_ BinarySearchAfter(const std::array<T, I>& arr, const T& data) { return BinarySearchAfter<T, CT_, CompF>(arr, I, data); }

  // Returns index of the item, if it exists, or -1
  template<typename T, typename D, typename CT_, char(*CompF)(const T&, const D&)>
  inline CT_ BinarySearchExact(const T* arr, const D& data, typename std::make_signed<CT_>::type f, typename std::make_signed<CT_>::type l)
  {
    --l; // Done so l can be an exclusive size parameter even though the algorithm is inclusive.
    CT_ m; // While f and l must be signed ints or the algorithm breaks, m does not.
    char r;

    while(l >= f) // This only works when l is an inclusive max indice
    {
      m = f + ((l - f) >> 1); // Done to avoid overflow on large numbers. >> 1 is valid because l must be >= f so this can't be negative

      if((r = (*CompF)(arr[m], data)) < 0) // This is faster than a switch statement
        f = m + 1;
      else if(r > 0)
        l = m - 1;
      else
        return m;
    }

    return (CT_)-1;
  }

  template<typename T, typename CT_, char(*CompF)(const T&, const T&), CT_ I>
  BUN_FORCEINLINE CT_ BinarySearchExact(const T(&arr)[I], const T& data) { return BinarySearchExact<T, T, CT_, CompF>(arr, data, 0, I); }

  // Generates a canonical function for the given real type with the maximum number of bits.
  template<typename T, typename ENGINE>
  BUN_FORCEINLINE T bunGenCanonical(ENGINE& e)
  {
    return std::generate_canonical<T, std::numeric_limits<T>::digits, ENGINE>(e);
  }

  inline XorshiftEngine64& bun_getdefaultengine()
  {
    static XorshiftEngine64 e;
    return e;
  }

  // Generates a number in the range [min,max) using the given engine.
  template<typename T, typename ENGINE = XorshiftEngine<uint64_t>>
  inline T bun_rand(T min, T max, ENGINE& e = bun_getdefaultengine())
  {
    return min + static_cast<T>(bunGenCanonical<double, ENGINE>(e)*static_cast<double>(max - min));
  }
  inline double bun_RandReal(double min, double max) { return bun_rand<double>(min, max); }
  inline int64_t bun_RandInt(int64_t min, int64_t max) { return bun_rand<int64_t>(min, max); }
  inline void bun_RandSeed(uint64_t s) { bun_getdefaultengine().seed(s); }

  // Shuffler using Fisher-Yates/Knuth Shuffle algorithm based on Durstenfeld's implementation.
  template<typename T, typename CT, typename ENGINE>
  inline void Shuffle(T* p, CT size, ENGINE& e)
  {
    for(CT i = size; i > 0; --i)
      std::swap(p[i - 1], p[bun_rand<CT, ENGINE>(0, i, e)]);
  }
  template<typename T, typename CT, typename ENGINE, CT size>
  inline void Shuffle(T(&p)[size], ENGINE& e) { Shuffle<T, CT, ENGINE>(p, size, e); }

  /* Shuffler using default random number generator.*/
  template<typename T>
  BUN_FORCEINLINE void Shuffle(T* p, int size)
  {
    Shuffle<T, int, XorshiftEngine<uint64_t>>(p, size, bun_getdefaultengine());
  }
  template<typename T, int size>
  BUN_FORCEINLINE void Shuffle(T(&p)[size]) { Shuffle<T>(p, size); }

  static const double ZIGNOR_R = 3.442619855899;

  // Instance for generating random samples from a normal distribution.
  template<int ZIGNOR_C = 128, typename T = double, typename ENGINE = XorshiftEngine<uint64_t>>
  struct NormalZig
  {
    T s_adZigX[ZIGNOR_C + 1], s_adZigR[ZIGNOR_C];

    NormalZig(int iC = ZIGNOR_C, T dV = 9.91256303526217e-3, T dR = ZIGNOR_R, ENGINE& e = bun_getdefaultengine()) :  // (R * phi(R) + Pr(X>=R)) * sqrt(2\pi)
      _e(e), _dist(0, 0x7F), _rdist(0, 1.0)
    {
      T f = exp(T(-0.5) * (dR * dR));
      s_adZigX[0] = dV / f; /* [0] is bottom block: V / f(R) */
      s_adZigX[1] = dR;
      s_adZigX[iC] = 0;

      for(int i = 2; i < iC; ++i)
      {
        s_adZigX[i] = sqrt(-2 * log(dV / s_adZigX[i - 1] + f));
        f = exp(T(-0.5) * (s_adZigX[i] * s_adZigX[i]));
      }

      for(int i = 0; i < iC; ++i)
        s_adZigR[i] = s_adZigX[i + 1] / s_adZigX[i];
    }

    T DRanNormalTail(T dMin, int iNegative)
    {
      T x, y;

      do
      {
        x = log(_rdist(_e)) / dMin;
        y = log(_rdist(_e));
      } while(-2 * y < x * x);

      return iNegative ? x - dMin : dMin - x;
    }

    // Returns a random normally distributed number using the ziggurat method
    T Get()
    {
      for(;;)
      {
        T u = 2 * _rdist(_e) - 1;
        uint32_t i = _dist(_e); // get random number between 0 and 0x7F
        /* first try the rectangular boxes */
        if(fabs(u) < s_adZigR[i])
          return u * s_adZigX[i];
        /* bottom box: sample from the tail */
        if(i == 0)
          return DRanNormalTail(ZIGNOR_R, u < 0);
        /* is this a sample from the wedges? */
        T x = u * s_adZigX[i];
        T f0 = exp(T(-0.5) * (s_adZigX[i] * s_adZigX[i] - x * x));
        T f1 = exp(T(-0.5) * (s_adZigX[i + 1] * s_adZigX[i + 1] - x * x));
        if(f1 + _rdist(_e) * (f0 - f1) < 1.0)
          return x;
      }
    }

    T operator()() { return Get(); }
    ENGINE& _e;
    std::uniform_int_distribution<uint32_t> _dist;
    std::uniform_real_distribution<double> _rdist;
  };

  // Randomly subdivides a rectangular area into smaller rects of varying size. F1 takes (depth,rect) and returns how likely it is that a branch will terminate.
  template<typename T, typename F1, typename F2, typename F3> // F2 takes (const float (&rect)[4]) and is called when a branch terminates on a rect.
  inline void StochasticSubdivider(const T(&rect)[4], const F1& f1, const F2& f2, const F3& f3, size_t depth = 0) // F3 returns a random number from [0,1]
  {
    if(bun_RandReal(0, 1.0) < f1(depth, rect))
    {
      f2(rect);
      return;
    }

    uint8_t axis = depth % 2;
    T div = lerp(rect[axis], rect[2 + axis], f3(depth, rect));
    T r1[4] = { rect[0], rect[1], rect[2], rect[3] };
    T r2[4] = { rect[0], rect[1], rect[2], rect[3] };
    r1[axis] = div;
    r2[2 + axis] = div;
    StochasticSubdivider(r1, f1, f2, f3, ++depth);
    StochasticSubdivider(r2, f1, f2, f3, depth);
  }

  // Implementation of a uniform quadratic B-spline interpolation
  template<typename T, typename D>
  inline T UniformQuadraticBSpline(D t, const T& p1, const T& p2, const T& p3)
  {
    D t2 = t*t;
    return (p1*(1 - 2 * t + t2) + p2*(1 + 2 * t - 2 * t2) + p3*t2) / ((D)2.0);
  }

  /* Implementation of a uniform cubic B-spline interpolation. A uniform cubic B-spline matrix is:
                  / -1  3 -3  1 \         / p1 \
   [t^3,t�,t,1] * |  3 -6  3  0 | * 1/6 * | p2 |
                  | -3  0  3  0 |         | p3 |
                  \  1  4  1  0 /         \ p4 / */
  template<typename T, typename D>
  inline T UniformCubicBSpline(D t, const T& p1, const T& p2, const T& p3, const T& p4)
  {
    D t2 = t*t;
    D t3 = t2*t;
    return (p1*(1 - 3 * t + 3 * t2 - t3) + p2*(4 - 6 * t2 + 3 * t3) + p3*(1 + 3 * t + 3 * t2 - 3 * t3) + (p4*t3)) / ((D)6.0);
  }

  /* Implementation of a basic cubic interpolation. The B-spline matrix for this is
                  / -1  3 -3  1 \       / p1 \
   [t^3,t�,t,1] * |  2 -5  4 -1 | * � * | p2 |
                  | -1  0  1  0 |       | p3 |
                  \  0  2  0  0 /       \ p4 /  */
  template<typename T, typename D>
  inline T CubicBSpline(D t, const T& p1, const T& p2, const T& p3, const T& p4)
  {
    D t2 = t*t;
    D t3 = t2*t;
    return (p1*(-t3 + 2 * t2 - t) + p2*(3 * t3 - 5 * t2 + 2) + p3*(-3 * t3 + 4 * t2 + t) + p4*(t3 - t2)) / ((D)2.0);
  }

  // This implements all possible B-spline functions, but does it statically without optimizations (so it can be used with any type)
  template<typename T, typename D, const T(&m)[4][4]>
  BUN_FORCEINLINE T StaticGenericSpline(D t, const T(&p)[4])
  {
    D t2 = t*t;
    D t3 = t2*t;
    return p[0] * (m[0][0] * t3 + m[1][0] * t2 + m[2][0] * t + m[3][0]) +
      p[1] * (m[0][1] * t3 + m[1][1] * t2 + m[2][1] * t + m[3][1]) +
      p[2] * (m[0][2] * t3 + m[1][2] * t2 + m[2][2] * t + m[3][2]) +
      p[3] * (m[0][3] * t3 + m[1][3] * t2 + m[2][3] * t + m[3][3]);
  }

  /* Implementation of a bezier curve. The B-spline matrix for this is
                  / -1  3 -3  1 \   / p1 \
   [t^3,t�,t,1] * |  3 -6  3  0 | * | p2 |
                  | -3  3  0  0 |   | p3 |
                  \  1  0  0  0 /   \ p4 /  */
  template<typename T, typename D>
  inline T BezierCurve(D t, const T& p1, const T& p2, const T& p3, const T& p4)
  {
    static constexpr float m[4][4] = { -1.0f, 3.0f, -3.0f, 1.0f, 3.0f, -6.0f, 3.0f, 0, -3.0f, 3, 0, 0, 1.0f, 0, 0, 0 };
    const float p[4] = { p1, p2, p3, p4 };
    return StaticGenericSpline<T, D, m>(t, p);
  }

  // Generic breadth-first search for a binary tree. Don't use this on a min/maxheap - it's internal array already IS in breadth-first order.
  template<typename T, bool(*FACTION)(T*), T* (*LCHILD)(T*), T* (*RCHILD)(T*)> // return true to quit
  inline void BreadthFirstTree(T* root, size_t n)
  {
    if(!n)
      return;

    if(n == 1)
    {
      FACTION(root);
      return;
    }

    n = (n / 2) + 1;
    VARARRAY(T*, queue, n);
    queue[0] = root;
    size_t l = 1;

    for(size_t i = 0; i != l; i = (i + 1) % n)
    {
      if(FACTION(queue[i]))
        return;

      queue[l] = LCHILD(queue[i]); //Enqueue the children
      l = (l + (size_t)(queue[l] != 0)) % n;
      queue[l] = RCHILD(queue[i]);
      l = (l + (size_t)(queue[l] != 0)) % n;
    }
  }

  template<typename T, typename D>
  inline T QuadraticBezierCurve(D t, const T& p0, const T& p1, const T& p2)
  {
    D inv = D(1) - t;
    return (inv*inv*p0) + (D(2) * inv*t*p1) + (t*t*p2);
  }

  // Find a quadratic curve (A,B,C) that passes through 3 points
  template<typename T>
  inline T QuadraticFit(T t, T x1, T y1, T x2, T y2, T x3, T y3)
  {
    T x1_2 = x1*x1;
    T x2_2 = x2*x2;
    T x3_2 = x3*x3;
    T A = ((y2 - y1)(x1 - x3) + (y3 - y1)(x2 - x1)) / ((x1 - x3)(x2_2 - x1_2) + (x2 - x1)(x3_2 - x1_2));
    T B = ((y2 - y1) - A(x2_2 - x1_2)) / (x2 - x1);
    T C = y1 - A*x1_2 - B*x1;
    return A*t*t + B*t + C;
  }

  // Find a quadratic curve (A,B,C) that passes through the points (x1, 0), (x2, 0.5), (x3, 1)
  template<typename T>
  inline T QuadraticCanonicalFit(T t, T x1, T x2, T x3)
  {
    t = (t - x1) / (x3 - x1); // We transform all points with (x - x1) / x3, which yields:
    // x1 = (x1 - x1) / (x3 - x1) = 0
    T x = (x2 - x1) / (x3 - x1);
    // x3 = (x3 - x1) / (x3 - x1) = 1
    //T x1_2 = 0;
    //T x2_2 = x2*x2;
    //T x3_2 = 1;
    T H = ((T)1 / (T)2);
    //T A = ((H - 0)(0 - 1) + (1 - 0)(x2 - 0)) / ((0 - 1)(x2_2 - 0) + (x2 - 0)(1 - 0));
    T A = (x - H) / (x - (x*x));
    //T B = ((H - 0) - A(x2_2 - 0)) / (x2 - 0);
    T B = (H / x) - (A * x);
    //T C = 0 - A*0 - B*0;

    return (A*t*t + B*t)*(x3 - x1) + x1; // reverse our transformation
  }

  // Splits a cubic (P0,P1,P2,P3) into two cubics: (P0, N1, N2, N3) and (N3, R1, R2, P3) using De Casteljau's Algorithm
  template<typename T, int I>
  inline void SplitCubic(T t, const T(&P0)[I], const T(&P1)[I], const T(&P2)[I], const T(&P3)[I], T(&N1)[I], T(&N2)[I], T(&N3)[I], T(&R1)[I], T(&R2)[I])
  {
    T F[I]; // A = P0, B = P1, C = P2, D = P3, E = N1, F, G = R2, H = N2, J = R1, K = N3

    for(int i = 0; i < I; ++i)
    {
      N1[i] = lerp<T>(P0[i], P1[i], t); // lerp(A+B)
      F[i] = lerp<T>(P1[i], P2[i], t); // lerp(B+C)
      R2[i] = lerp<T>(P2[i], P3[i], t); // lerp(C+D)
      N2[i] = lerp<T>(N1[i], F[i], t); // lerp(E+F)
      R1[i] = lerp<T>(F[i], R2[i], t); // lerp(F+G)
      N3[i] = lerp<T>(N2[i], R1[i], t); // lerp(H+J)
    }
  }

  // Splits a quadratic (P0, P1, P2) into two quadratics: (P0, N1, N2) and (N2, R1, P2) using De Casteljau's Algorithm
  template<typename T, int I>
  inline void SplitQuadratic(T t, const T(&P0)[I], const T(&P1)[I], const T(&P2)[I], T(&N1)[I], T(&N2)[I], T(&R1)[I])
  {
    for(int i = 0; i < I; ++i)
    {
      N1[i] = lerp<T>(P0[i], P1[i], t);
      R1[i] = lerp<T>(P1[i], P2[i], t);
      N2[i] = lerp<T>(N1[i], R1[i], t);
    }
  }

  // Solves a quadratic equation of the form at� + bt + c
  template<typename T>
  inline void SolveQuadratic(T a, T b, T c, T(&r)[2])
  {
    T d = FastSqrt<T>(b*b - 4 * a*c);
    r[0] = (-b - d) / (2 * a);
    r[1] = (-b + d) / (2 * a);
  }

  // See: http://www.caffeineowl.com/graphics/2d/vectorial/cubic-inflexion.html
  template<typename T>
  inline void CubicInflectionPoints(const T(&P0)[2], const T(&P1)[2], const T(&P2)[2], const T(&P3)[2], T(&r)[2])
  {
    T a[2];
    T b[2];
    T c[2];

    for(int i = 0; i < 2; ++i)
    {
      a[i] = P1[i] - P0[i];
      b[i] = P2[i] - P1[i] - a[i];
      c[i] = P3[i] - P2[i] - a[i] - 2 * b[i];
    }

    SolveQuadratic<T>(b[0] * c[1] - b[1] * c[0], a[0] * c[1] - a[1] * c[0], a[0] * b[1] - a[1] * b[0], r);
  }

  // Uses modified formulas from: http://www.caffeineowl.com/graphics/2d/vectorial/cubic2quad01.html
  template<typename T, int I>
  inline T ApproxCubicError(const T(&P0)[I], const T(&P1)[I], const T(&P2)[I], const T(&P3)[I])
  {
    // The error for a quadratic approximating a cubic (sharing the anchor points) is: t�(1 - t)�|2�C - 3�C1 + P1 + 3�t�(P2 - 3�C2 + 3�C1 - P1)|    where |v| is the modulus: sqrt(v[0]� + v[1]�)
    // If we choose C = (3�C2 - P2 + 3�C1 - P1)/4 we get f(t) = t�(1 - t)��(6�t - 1)|(3�C_1 - 3�C_2 - P_1 + P_2)|
    // f'(t) = -�(2�t(9�t - 7) + 1) |(3�C_1 - 3�C_2 - P_1 + P_2)| = 0 -> -�(2�t(9�t - 7) + 1) = 0 -> 2�t(9�t - 7) = -1
    // Solving the derivative for 0 to maximize the error value, we get t = (1/18)(7�sqrt(31)). Only the + result is inside [0,1], so we plug that into t�(1 - t)��(6�t - 1) = 77/486+(31 sqrt(31))/972 ~ 0.336008945728118
    const T term = (T)0.336008945728118;

    T r = 0;
    T M;

    for(int i = 0; i < I; ++i) // 3�C_1 - 3�C_2 - P_1 + P_2
    {
      M = T(3) * P1[i] - T(3) * P2[i] - P0[i] + P3[i];
      r += M*M;
    }

    return term * FastSqrt<T>(r);
  }

  template<typename T, typename FN>
  inline void ApproxCubicR(T(&t)[3], const T(&P0)[2], const T(&P1)[2], const T(&P2)[2], const T(&P3)[2], FN fn, T maxerror)
  {
    T N1[2];
    T N2[2];
    T N3[2];
    T R1[2];
    T R2[2];

    SplitCubic(t[1], P0, P1, P2, P3, N1, N2, N3, R1, R2);

    // Check first section: P0, N1, N2, N3
    if(ApproxCubicError<T, 2>(P0, N1, N2, N3) > maxerror)
    {
      T tfirst[3] = { t[0], (t[1] + t[0]) / 2, t[1] };
      ApproxCubicR<T, FN>(tfirst, P0, N1, N2, N3, fn, maxerror);
    }
    else
    {
      T C[2]; // (3�(P2 + P1) - P3 - P0) / 4
      for(int i = 0; i < 2; ++i)
        C[i] = (3 * (N2[i] + N1[i]) - N3[i] - P0[i]) / 4;
      fn(P0, C, N3);
    }

    // Check second section: N3, R1, R2, P3
    if(ApproxCubicError<T, 2>(N3, R1, R2, P3) > maxerror)
    {
      T tsecond[3] = { t[1], (t[2] + t[1]) / 2, t[2] };
      ApproxCubicR<T, FN>(tsecond, N3, R1, R2, P3, fn, maxerror);
    }
    else
    {
      T C[2]; // (3�(P2 + P1) - P3 - P0) / 4
      for(int i = 0; i < 2; ++i)
        C[i] = (3 * (R2[i] + R1[i]) - P3[i] - N3[i]) / 4;
      fn(N3, C, P3);
    }
  }

  template<typename T, typename FN>
  inline void ApproxCubic(const T(&P0)[2], const T(&P1)[2], const T(&P2)[2], const T(&P3)[2], FN fn, T maxerror = FLT_EPSILON)
  {
    T r[2];
    CubicInflectionPoints<T>(P0, P1, P2, P3, r);
    T t[3] = { (T)0.0, (r[0] >= (T)0.0 && r[0] <= (T)1.0) ? r[0] : ((r[1] >= (T)0.0 && r[1] <= (T)1.0) ? r[1] : (T)0.5), (T)1.0 };
    ApproxCubicR<T, FN>(t, P0, P1, P2, P3, fn, maxerror); // Call recursive function that does the actual splitting
  }

  // Solves a cubic equation of the form at^3 + bt� + ct + d by normalizing it (dividing everything by a).
  template<typename T>
  inline int solveCubic(T at, T bt, T ct, T dt, T(&r)[3])
  {
    T a = bt / at;
    T b = ct / at;
    T c = dt / at;
    T p = b - a*a / 3;
    T q = a * (2 * a*a - 9 * b) / 27 + c;
    T p3 = p*p*p;
    T d = q*q + 4 * p3 / 27;
    T offset = -a / 3;

    if(d >= 0)
    { // Single solution
      T z = FastSqrt<T>(d);
      T u = (-q + z) / 2;
      T v = (-q - z) / 2;
      u = cbrt(u);
      v = cbrt(v);
      r[0] = offset + u + v;
      return 1;
    }

    T u = FastSqrt<T>(-p / 3);
    T v = acos(-FastSqrt<T>(-27 / p3) * q / 2) / 3;
    T m = cos(v), n = sin(v)*((T)1.732050808);
    r[0] = offset + u * (m + m);
    r[1] = offset - u * (n + m);
    r[2] = offset + u * (n - m);
    return 3;
  }

  // Uses Newton's method to find the root of F given it's derivative dF. Returns false if it fails to converge within the given error range.
  template<typename T, typename TF, typename TDF>
  inline bool NewtonRaphson(T& result, T estimate, TF F, TDF dF, T epsilon, size_t maxiterations = 20)
  {
    T x = estimate;

    for(size_t i = 0; i < maxiterations; ++i)
    {
      T f = F(x);
      if(fSmall(f, epsilon)) // If we're close enough to zero, return our value
      {
        result = x;
        return true;
      }
      x = x - (f / dF(x));
    }

    return false; // We failed to converge to the required error bound within the given number of iterations
  }

  // A hybrid method that combines both Newton's method and the bisection method, using the bisection method to improve the guess until Newton's method finally starts to converge.
  template<typename T, typename TF, typename TDF>
  inline T NewtonRaphsonBisection(T estimate, T min, T max, TF F, TDF dF, T epsilon, size_t maxiterations = 50)
  {
    T x = estimate;
    for(size_t i = 0; i < maxiterations; ++i)
    {
      T f = F(x);

      if(fSmall(f, epsilon)) // If we're close enough to zero, return x
        return x;

      if(f > 0)
        max = x;
      else
        min = x;

      x = x - f / dF(x);

      if(x <= min || x >= max) // If candidate is out of range, use bisection instead
        x = 0.5*(min + max);
    }
    return x; // Return a good-enough value. Because we're using bisection, it has to be at least reasonably close to the root.
  }

  // Performs Gauss�Legendre quadrature for simple polynomials, using 1-5 sampling points on the interval [a, b] with optional supplemental arguments.
  template<typename T, int N, typename... D>
  inline T GaussianQuadrature(T a, T b, T(*f)(T, D...), D... args)
  {
    static_assert(N <= 5, "Too many points for Guassian Quadrature!");
    static_assert(N < 1, "Too few points for Guassian Quadrature!");

    static const T points[5][5] = {
      0, 0, 0, 0, 0,
      -sqrt(1.0 / 3.0), sqrt(1.0 / 3.0), 0, 0, 0,
      0, -sqrt(3.0 / 5.0), sqrt(3.0 / 5.0), 0, 0,
      -sqrt((3.0 / 7.0) - ((2.0 / 7.0)*sqrt(6.0 / 5.0))), sqrt((3.0 / 7.0) - ((2.0 / 7.0)*sqrt(6.0 / 5.0))), -sqrt((3.0 / 7.0) + ((2.0 / 7.0)*sqrt(6.0 / 5.0))), sqrt((3.0 / 7.0) + ((2.0 / 7.0)*sqrt(6.0 / 5.0))), 0,
      0, -sqrt(5.0 - 2.0*sqrt(10.0 / 7.0)) / 3.0, sqrt(5.0 - 2.0*sqrt(10.0 / 7.0)) / 3.0, -sqrt(5.0 + 2.0*sqrt(10.0 / 7.0)) / 3.0, sqrt(5.0 + 2.0*sqrt(10.0 / 7.0)) / 3.0,
    };

    static const T weights[5][5] = {
      2.0, 0, 0, 0, 0,
      1.0, 1.0, 0, 0, 0,
      8.0 / 9.0, 5.0 / 9.0, 5.0 / 9.0, 0, 0,
      (18 + sqrt(30)) / 36, (18 + sqrt(30)) / 36, (18 - sqrt(30)) / 36, (18 - sqrt(30)) / 36, 0,
      128 / 225, (322 + 13 * sqrt(70)) / 900, (322 + 13 * sqrt(70)) / 900, (322 - 13 * sqrt(70)) / 900, (322 - 13 * sqrt(70)) / 900,
    };

    T scale = (b - a) / 2.0;
    T avg = (a + b) / 2.0;
    T r = weights[N - 1][0] * f(scale * points[N - 1][0] + avg, args...);

    for(int i = 1; i < N; ++i)
      r += weights[N - 1][i] * f(scale * points[N - 1][i] + avg, args...);

    return scale * r;
  }


  inline size_t Base64Encode(const uint8_t* src, size_t cnt, char* out)
  {
    size_t cn = ((cnt / 3) << 2) + (cnt % 3) + (cnt % 3 != 0);

    if(!out)
      return cn;

    /*const uint32_t* ints = (const uint32_t*)src;
    size_t s = (cnt - (cnt % 12))/4;
    size_t c = 0;
    size_t i;
    for(i = 0; i < s; i += 3)
    {
      sseVeci x(ints[i], ints[i], ints[i + 1], ints[i + 2]);
      sseVeci y(ints[i], ints[i + 1], ints[i + 2], ints[i + 2]);
      sseVeci a(0b00111111001111110011111100111111, 0b11000000110000001100000011000000, 0b11110000111100001111000011110000, 0b11111100111111001111110011111100);
      sseVeci b(0b00000000000000000000000000000000, 0b00001111000011110000111100001111, 0b00000011000000110000001100000011, 0b00000000000000000000000000000000);
      sseVeci n(0, 6, 4, 2);
      sseVeci m(0, 2, 4, 0);

      sseVeci res = (sseVeci(BUN_SSE_SR_EPI32((x&a), n)) | ((y&b) << m));
      res += sseVeci('A')&(res < sseVeci(26));
      res += sseVeci('a' - 26)&(res < sseVeci(52));
      res += sseVeci('0' - 52)&(res < sseVeci(62)); // this works because up until this point, all previous blocks were moved past the 62 mark
      res += sseVeci('-' - 62)&(res == sseVeci(62));
      res += sseVeci('_' - 63)&(res == sseVeci(63));

      BUN_SSE_STORE_USI128((BUN_SSE_M128i16*)(out + c), res);
      c += 16;
    }*/

    static const char* code = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    size_t c = 0;
    size_t s = cnt - (cnt % 3);
    size_t i;

    for(i = 0; i < s; i += 3)
    {
      out[c++] = code[((src[i + 0] & 0b11111100) >> 2)];
      out[c++] = code[((src[i + 0] & 0b00000011) << 4) | ((src[i + 1] & 0b11110000) >> 4)];
      out[c++] = code[((src[i + 1] & 0b00001111) << 2) | ((src[i + 2] & 0b11000000) >> 6)];
      out[c++] = code[((src[i + 2] & 0b00111111))];
      //out[c++] = code[src[i] & 0b00111111];
      //out[c++] = code[((src[i] & 0b11000000) >> 6) | ((src[i + 1] & 0b00001111) << 2)];
      //out[c++] = code[((src[i + 1] & 0b11110000) >> 4) | ((src[i + 2] & 0b00000011) << 4)];
      //out[c++] = code[((src[i + 2] & 0b11111100) >> 2)];
    }

    if(i < cnt)
    {
      out[c++] = code[((src[i] & 0b11111100) >> 2)];

      if((i + 1) >= cnt)
        out[c++] = code[((src[i + 0] & 0b00000011) << 4)];
      else
      {
        out[c++] = code[((src[i + 0] & 0b00000011) << 4) | ((src[i + 1] & 0b11110000) >> 4)];
        out[c++] = code[((src[i + 1] & 0b00001111) << 2)];
      }
    }
    return c;
  }

  inline size_t Base64Decode(const char* src, size_t cnt, uint8_t* out)
  {
    if((cnt & 0b11) == 1)
      return 0; // You cannot have a legal base64 encode of this length.

    size_t cn = ((cnt >> 2) * 3) + (cnt & 0b11) - ((cnt & 0b11) != 0);
    if(!out)
      return cn;

    size_t i = 0;
    size_t c = 0;
    uint8_t map[78] = { 62,0,0,52,53,54,55,56,57,58,59,60,61,0,0,0,0,0,0,0,
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,0,0,0,0,63,0,
    26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51 };
    size_t s = cnt & (~0b11);

    for(; i < s; i += 4)
    {
      out[c++] = (map[src[i + 0] - '-'] << 2) | (map[src[i + 1] - '-'] >> 4);
      out[c++] = (map[src[i + 1] - '-'] << 4) | (map[src[i + 2] - '-'] >> 2);
      out[c++] = (map[src[i + 2] - '-'] << 6) | (map[src[i + 3] - '-']);
    }

    if(++i < cnt) // we do ++i here even though i was already valid because the first character requires TWO source characters, not 1.
      out[c++] = (map[src[i - 1] - '-'] << 2) | (map[src[i] - '-'] >> 4); 
    if(++i < cnt)
      out[c++] = (map[src[i - 1] - '-'] << 4) | (map[src[i] - '-'] >> 2);
    return c;
  }
}


#endif
