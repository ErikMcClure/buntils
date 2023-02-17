// Copyright �2018 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BUN_VECTOR_H__
#define __BUN_VECTOR_H__

#include "buntils.h"
#include "sseVec.h"
#include <initializer_list>
#include <array>

namespace bun {
  template<class Engine>
  class Serializer;

  // Find the dot product of two n-dimensional vectors
  template<typename T, int N>
  [[nodiscard]] BUN_FORCEINLINE T NVectorDot(const T(&l)[N], const T(&r)[N])
  {
    T ret = 0;
    for(int i = 0; i < N; ++i)
      ret += (l[i] * r[i]);
    return ret;
  }

  template<>
  [[nodiscard]] BUN_FORCEINLINE float NVectorDot<float, 4>(const float(&l)[4], const float(&r)[4])
  {
    return (sseVec(l)*sseVec(r)).Sum();
  }

  template<>
  [[nodiscard]] BUN_FORCEINLINE int NVectorDot<int, 4>(const int(&l)[4], const int(&r)[4])
  {
    return (sseVeci(l)*sseVeci(r)).Sum();
  }

  // Get the squared distance between two n-dimensional vectors
  template<typename T, int N>
  [[nodiscard]] inline T NVectorDistanceSq(const T(&l)[N], const T(&r)[N])
  {
    T tp = r[0] - l[0];
    T ret = tp*tp;
    for(int i = 1; i < N; ++i)
    {
      tp = r[i] - l[i];
      ret += tp*tp;
    }
    return ret;
  }
  template<typename T, int N>
  [[nodiscard]] BUN_FORCEINLINE T NVectorDistance(const T(&l)[N], const T(&r)[N]) { return FastSqrt<T>(NVectorDistanceSq<T, N>(l, r)); }

  template<typename T, int N>
  BUN_FORCEINLINE void NVectorNormalize(const T(&v)[N], T(&out)[N])
  {
    T invlength = 1 / FastSqrt<T>(NVectorDot<T, N>(v, v));
    assert(invlength != 0);
    for(int i = 0; i < N; ++i)
      out[i] = v[i] * invlength;
  }

  template<>
  BUN_FORCEINLINE void NVectorNormalize<float, 4>(const float(&v)[4], float(&out)[4])
  {
    float l = FastSqrt<float>(NVectorDot<float, 4>(v, v));
    (sseVec(v) / sseVec(l, l, l, l)).Set(out);
  }

  template<typename T>
  [[nodiscard]] BUN_FORCEINLINE T NVectorAbsCall(const T v)
  {
    return abs((typename std::conditional<std::is_integral<T>::value, typename std::make_signed<typename std::conditional<std::is_integral<T>::value, T, int>::type>::type, T>::type)v);
  }

  template<typename T, int N>
  BUN_FORCEINLINE void NVectorAbs(const T(&v)[N], T(&out)[N])
  {
    for(int i = 0; i < N; ++i)
      out[i] = NVectorAbsCall(v[i]);
  }

  // Find the area of an n-dimensional triangle using Heron's formula
  template<typename T, int N>
  [[nodiscard]] inline T NTriangleArea(const T(&x1)[N], const T(&x2)[N], const T(&x3)[N])
  {
    T a = NVectorDistance(x1, x2);
    T b = NVectorDistance(x1, x3);
    T c = NVectorDistance(x2, x3);
    T s = (a + b + c) / ((T)2);
    return FastSqrt<T>(s*(s - a)*(s - b)*(s - c));
  }

  // Applies an operator to two vectors
  /*template<typename T, int N, T(*F)(T, T), sseVecT<T>(*sseF)(const sseVecT<T>&, const sseVecT<T>&)>
  BUN_FORCEINLINE void NVectOp(const T(&x1)[N], const T(&x2)[N], T(&out)[N])
  {
  assert(((size_t)x1)%16==0);
  assert(((size_t)x2)%16==0);
  assert(((size_t)out)%16==0);
  int low=(N/4)*4; // Gets number of SSE iterations we can do
  int i;
  for(i=0; i < low; i+=4)
  BUN_SSE_STORE_APS(out+i, sseF(sseVecT<T>(BUN_SSE_LOAD_APS(x1+i)), sseVecT<T>(BUN_SSE_LOAD_APS(x2+i))));
  for(; i<N; ++i)
  out[i]=F(x1[i], x2[i]);
  }

  // Applies an operator to a vector and a scalar
  template<typename T, int N, T(*F)(T, T), sseVecT<T>(*sseF)(const sseVecT<T>&, const sseVecT<T>&)>
  BUN_FORCEINLINE void NVectOp(const T(&x1)[N], T x2, T(&out)[N])
  {
  assert(((size_t)x1)%16==0);
  assert(((size_t)out)%16==0);
  int low=(N/4)*4; // Gets number of SSE iterations we can do
  int i;
  for(i=0; i < low; i+=4)
  BUN_SSE_STORE_APS(out+i, sseF(sseVecT<T>(BUN_SSE_LOAD_APS(x1+i)), sseVecT<T>(x2)));
  for(; i<N; ++i)
  out[i]=F(x1[i], x2);
  }

  template<typename T, typename R> BUN_FORCEINLINE R NVectFAdd(T a, T b) { return a+b; }
  template<typename T, typename R> BUN_FORCEINLINE R NVectFSub(T a, T b) { return a-b; }
  template<typename T, typename R> BUN_FORCEINLINE R NVectFMul(T a, T b) { return a*b; }
  template<typename T, typename R> BUN_FORCEINLINE R NVectFDiv(T a, T b) { return a/b; }
  template<typename T, int N> BUN_FORCEINLINE void NVectAdd(const T(&x1)[N], const T(&x2)[N], T(&out)[N])
  { return NVectOp<T, N, NVectFAdd<T, T>, NVectFAdd<const sseVecT<T>&, sseVecT<T>>>(x1, x2, out); }
  template<typename T, int N> BUN_FORCEINLINE void NVectAdd(const T(&x1)[N], T x2, T(&out)[N])
  { return NVectOp<T, N, NVectFAdd<T, T>, NVectFAdd<const sseVecT<T>&, sseVecT<T>>>(x1, x2, out); }
  template<typename T, int N> BUN_FORCEINLINE void NVectSub(const T(&x1)[N], const T(&x2)[N], T(&out)[N])
  { return NVectOp<T, N, NVectFSub<T, T>, NVectFSub<const sseVecT<T>&, sseVecT<T>>>(x1, x2, out); }
  template<typename T, int N> BUN_FORCEINLINE void NVectSub(const T(&x1)[N], T x2, T(&out)[N])
  { return NVectOp<T, N, NVectFSub<T, T>, NVectFSub<const sseVecT<T>&, sseVecT<T>>>(x1, x2, out); }
  template<typename T, int N> BUN_FORCEINLINE void NVectMul(const T(&x1)[N], const T(&x2)[N], T(&out)[N])
  { return NVectOp<T, N, NVectFMul<T, T>, NVectFMul<const sseVecT<T>&, sseVecT<T>>>(x1, x2, out); }
  template<typename T, int N> BUN_FORCEINLINE void NVectMul(const T(&x1)[N], T x2, T(&out)[N])
  { return NVectOp<T, N, NVectFMul<T, T>, NVectFMul<const sseVecT<T>&, sseVecT<T>>>(x1, x2, out); }
  template<typename T, int N> BUN_FORCEINLINE void NVectDiv(const T(&x1)[N], const T(&x2)[N], T(&out)[N])
  { return NVectOp<T, N, NVectFDiv<T, T>, NVectFDiv<const sseVecT<T>&, sseVecT<T>>>(x1, x2, out); }
  template<typename T, int N> BUN_FORCEINLINE void NVectDiv(const T(&x1)[N], T x2, T(&out)[N])
  { return NVectOp<T, N, NVectFDiv<T, T>, NVectFDiv<const sseVecT<T>&, sseVecT<T>>>(x1, x2, out); }*/

  template<typename T, int M, int N, bool B = false>
  struct SetSubMatrix
  {
    static BUN_FORCEINLINE void M2x2(T(&out)[N][M], T m11, T m12, T m21, T m22)
    {
      static_assert(N >= 2 && M >= 2, "The NxM matrix must be at least 2x2");
      out[0][0] = m11; out[0][1] = m12;
      out[1][0] = m21; out[1][1] = m22;
    }
    static BUN_FORCEINLINE void M3x3(T(&out)[N][M], T m11, T m12, T m13, T m21, T m22, T m23, T m31, T m32, T m33)
    {
      static_assert(N >= 3 && M >= 3, "The NxM matrix must be at least 3x3");
      out[0][0] = m11; out[0][1] = m12; out[0][2] = m13;
      out[1][0] = m21; out[1][1] = m22; out[1][2] = m23;
      out[2][0] = m31; out[2][1] = m32; out[2][2] = m33;
    }
    static BUN_FORCEINLINE void M4x4(T(&out)[N][M], T m11, T m12, T m13, T m14, T m21, T m22, T m23, T m24, T m31, T m32, T m33, T m34, T m41, T m42, T m43, T m44)
    {
      static_assert(N >= 4 && M >= 4, "The NxM matrix must be at least 4x4");
      out[0][0] = m11; out[0][1] = m12; out[0][2] = m13; out[0][3] = m14;
      out[1][0] = m21; out[1][1] = m22; out[1][2] = m23; out[1][3] = m24;
      out[2][0] = m31; out[2][1] = m32; out[2][2] = m33; out[2][3] = m34;
      out[3][0] = m41; out[3][1] = m42; out[3][2] = m43; out[3][3] = m44;
    }
  };

  template<typename T, int M, int N>
  struct SetSubMatrix<T, M, N, true>
  {
    static BUN_FORCEINLINE void M2x2(T(&out)[N][M], T m11, T m12, T m21, T m22)
    {
      SetSubMatrix<T, M, N, false>::M2x2(out, m11, m21, m12, m22);
    }
    static BUN_FORCEINLINE void M3x3(T(&out)[N][M], T m11, T m12, T m13, T m21, T m22, T m23, T m31, T m32, T m33)
    {
      SetSubMatrix<T, M, N, false>::M3x3(out, m11, m21, m31, m12, m22, m32, m13, m23, m33);
    }
    static BUN_FORCEINLINE void M4x4(T(&out)[N][M], T m11, T m12, T m13, T m14, T m21, T m22, T m23, T m24, T m31, T m32, T m33, T m34, T m41, T m42, T m43, T m44)
    {
      SetSubMatrix<T, M, N, false>::M4x4(out, m11, m21, m31, m41, m12, m22, m32, m42, m13, m23, m33, m43, m14, m24, m34, m44);
    }
  };

  template<typename T>
  BUN_FORCEINLINE void MatrixInvert4x4(const T* mat, T* dst) noexcept
  {
    T tmp[12]; /* temp array for pairs */
    T src[16]; /* array of transpose source matrix */
    T det;  /* determinant */
    /* transpose matrix */
    for(int i = 0; i < 4; i++)
    { // because of this operation, mat and dst can be the same
      src[i] = mat[i * 4];
      src[i + 4] = mat[i * 4 + 1];
      src[i + 8] = mat[i * 4 + 2];
      src[i + 12] = mat[i * 4 + 3];
    }
    /* calculate pairs for first 8 elements (cofactors) */
    tmp[0] = src[10] * src[15];
    tmp[1] = src[11] * src[14];
    tmp[2] = src[9] * src[15];
    tmp[3] = src[11] * src[13];
    tmp[4] = src[9] * src[14];
    tmp[5] = src[10] * src[13];
    tmp[6] = src[8] * src[15];
    tmp[7] = src[11] * src[12];
    tmp[8] = src[8] * src[14];
    tmp[9] = src[10] * src[12];
    tmp[10] = src[8] * src[13];
    tmp[11] = src[9] * src[12];
    /* calculate first 8 elements (cofactors) */
    dst[0] = tmp[0] * src[5] + tmp[3] * src[6] + tmp[4] * src[7];
    dst[0] -= tmp[1] * src[5] + tmp[2] * src[6] + tmp[5] * src[7];
    dst[1] = tmp[1] * src[4] + tmp[6] * src[6] + tmp[9] * src[7];
    dst[1] -= tmp[0] * src[4] + tmp[7] * src[6] + tmp[8] * src[7];
    dst[2] = tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7];
    dst[2] -= tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7];
    dst[3] = tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6];
    dst[3] -= tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6];
    dst[4] = tmp[1] * src[1] + tmp[2] * src[2] + tmp[5] * src[3];
    dst[4] -= tmp[0] * src[1] + tmp[3] * src[2] + tmp[4] * src[3];
    dst[5] = tmp[0] * src[0] + tmp[7] * src[2] + tmp[8] * src[3];
    dst[5] -= tmp[1] * src[0] + tmp[6] * src[2] + tmp[9] * src[3];
    dst[6] = tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3];
    dst[6] -= tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3];
    dst[7] = tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2];
    dst[7] -= tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2];
    /* calculate pairs for second 8 elements (cofactors) */
    tmp[0] = src[2] * src[7];
    tmp[1] = src[3] * src[6];
    tmp[2] = src[1] * src[7];
    tmp[3] = src[3] * src[5];
    tmp[4] = src[1] * src[6];
    tmp[5] = src[2] * src[5];
    tmp[6] = src[0] * src[7];
    tmp[7] = src[3] * src[4];
    tmp[8] = src[0] * src[6];
    tmp[9] = src[2] * src[4];
    tmp[10] = src[0] * src[5];
    tmp[11] = src[1] * src[4];
    /* calculate second 8 elements (cofactors) */
    dst[8] = tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15];
    dst[8] -= tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15];
    dst[9] = tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15];
    dst[9] -= tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15];
    dst[10] = tmp[2] * src[12] + tmp[7] * src[13] + tmp[10] * src[15];
    dst[10] -= tmp[3] * src[12] + tmp[6] * src[13] + tmp[11] * src[15];
    dst[11] = tmp[5] * src[12] + tmp[8] * src[13] + tmp[11] * src[14];
    dst[11] -= tmp[4] * src[12] + tmp[9] * src[13] + tmp[10] * src[14];
    dst[12] = tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9];
    dst[12] -= tmp[4] * src[11] + tmp[0] * src[9] + tmp[3] * src[10];
    dst[13] = tmp[8] * src[11] + tmp[0] * src[8] + tmp[7] * src[10];
    dst[13] -= tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8];
    dst[14] = tmp[6] * src[9] + tmp[11] * src[11] + tmp[3] * src[8];
    dst[14] -= tmp[10] * src[11] + tmp[2] * src[8] + tmp[7] * src[9];
    dst[15] = tmp[10] * src[10] + tmp[4] * src[8] + tmp[9] * src[9];
    dst[15] -= tmp[8] * src[9] + tmp[11] * src[10] + tmp[5] * src[8];
    /* calculate determinant */
    det = src[0] * dst[0] + src[1] * dst[1] + src[2] * dst[2] + src[3] * dst[3];
    /* calculate matrix inverse */
    det = 1 / det;
    for(int j = 0; j < 16; j++)
      dst[j] *= det;
  }

#ifdef BUN_SSE_ENABLED
  // Standard 4x4 Matrix inversion using SSE, adapted from intel's implementation.
  template<>
  BUN_FORCEINLINE void MatrixInvert4x4<float>(const float* src, float* dest) noexcept // no point in using __restrict because we load everything into SSE registers anyway
  {
    //   Copyright (c) 2001 Intel Corporation.
    //
    // Permition is granted to use, copy, distribute and prepare derivative works 
    // of this library for any purpose and without fee, provided, that the above 
    // copyright notice and this statement appear in all copies.  
    // Intel makes no representations about the suitability of this software for 
    // any purpose, and specifically disclaims all warranties. 
    static const BUN_ALIGN(16) uint32_t _Sign_PNNP[4] = { 0x00000000, 0x80000000, 0x80000000, 0x00000000 };

    // The inverse is calculated using "Divide and Conquer" technique. The 
    // original matrix is divide into four 2x2 sub-matrices. Since each 
    // register holds four matrix element, the smaller matrices are 
    // represented as a registers. Hence we get a better locality of the 
    // calculations.
    sseVec _L1(BUN_SSE_LOAD_APS(src));
    sseVec _L2(BUN_SSE_LOAD_APS(src + 4));
    sseVec _L3(BUN_SSE_LOAD_APS(src + 8));
    sseVec _L4(BUN_SSE_LOAD_APS(src + 12));

    sseVec A = _mm_movelh_ps(_L1, _L2),    // the four sub-matrices 
      B = _mm_movehl_ps(_L2, _L1),
      C = _mm_movelh_ps(_L3, _L4),
      D = _mm_movehl_ps(_L4, _L3);
    __m128 det, d, d1, d2;

    //  AB = A# * B
    sseVec AB = _mm_mul_ps(_mm_shuffle_ps(A, A, 0x0F), B);
    AB -= (sseVec)_mm_mul_ps(_mm_shuffle_ps(A, A, 0xA5), _mm_shuffle_ps(B, B, 0x4E));
    //  DC = D# * C
    sseVec DC = _mm_mul_ps(_mm_shuffle_ps(D, D, 0x0F), C);
    DC -= (sseVec)_mm_mul_ps(_mm_shuffle_ps(D, D, 0xA5), _mm_shuffle_ps(C, C, 0x4E));

    //  dA = |A|
    __m128 dA = _mm_mul_ps(_mm_shuffle_ps(A, A, 0x5F), A);
    dA = _mm_sub_ss(dA, _mm_movehl_ps(dA, dA));
    //  dB = |B|
    __m128 dB = _mm_mul_ps(_mm_shuffle_ps(B, B, 0x5F), B);
    dB = _mm_sub_ss(dB, _mm_movehl_ps(dB, dB));

    //  dC = |C|
    __m128 dC = _mm_mul_ps(_mm_shuffle_ps(C, C, 0x5F), C);
    dC = _mm_sub_ss(dC, _mm_movehl_ps(dC, dC));
    //  dD = |D|
    __m128 dD = _mm_mul_ps(_mm_shuffle_ps(D, D, 0x5F), D);
    dD = _mm_sub_ss(dD, _mm_movehl_ps(dD, dD));

    //  d = trace(AB*DC) = trace(A#*B*D#*C)
    d = _mm_mul_ps(_mm_shuffle_ps(DC, DC, 0xD8), AB);

    //  iD = C*A#*B
    sseVec iD = _mm_mul_ps(_mm_shuffle_ps(C, C, 0xA0), _mm_movelh_ps(AB, AB));
    iD += (sseVec)_mm_mul_ps(_mm_shuffle_ps(C, C, 0xF5), _mm_movehl_ps(AB, AB));
    //  iA = B*D#*C
    sseVec iA = _mm_mul_ps(_mm_shuffle_ps(B, B, 0xA0), _mm_movelh_ps(DC, DC));
    iA += (sseVec)_mm_mul_ps(_mm_shuffle_ps(B, B, 0xF5), _mm_movehl_ps(DC, DC));

    //  d = trace(AB*DC) = trace(A#*B*D#*C) [continue]
    d = _mm_add_ps(d, _mm_movehl_ps(d, d));
    d = _mm_add_ss(d, _mm_shuffle_ps(d, d, 1));
    d1 = _mm_mul_ss(dA, dD);
    d2 = _mm_mul_ss(dB, dC);

    //  iD = D*|A| - C*A#*B
    iD = D*sseVec(_mm_shuffle_ps(dA, dA, 0)) - iD;

    //  iA = A*|D| - B*D#*C;
    iA = A*sseVec(_mm_shuffle_ps(dD, dD, 0)) - iA;

    //  det = |A|*|D| + |B|*|C| - trace(A#*B*D#*C)
    det = _mm_sub_ss(_mm_add_ss(d1, d2), d);
    sseVec rd = _mm_div_ss(_mm_set_ss(1.0f), det);
    // rd = _mm_and_ps(_mm_cmpneq_ss(det, _mm_setzero_ps()), rd); // This would set the matrix to zero if it wasn't invertible.


    //  iB = D * (A#B)# = D*B#*A
    sseVec iB = _mm_mul_ps(D, _mm_shuffle_ps(AB, AB, 0x33));
    iB -= (sseVec)_mm_mul_ps(_mm_shuffle_ps(D, D, 0xB1), _mm_shuffle_ps(AB, AB, 0x66));
    //  iC = A * (D#C)# = A*C#*D
    sseVec iC = _mm_mul_ps(A, _mm_shuffle_ps(DC, DC, 0x33));
    iC -= (sseVec)_mm_mul_ps(_mm_shuffle_ps(A, A, 0xB1), _mm_shuffle_ps(DC, DC, 0x66));

    rd = _mm_shuffle_ps(rd, rd, 0);
    rd = _mm_castsi128_ps(BUN_SSE_XOR(_mm_castps_si128(rd), BUN_SSE_LOAD_ASI128((const __m128i*)_Sign_PNNP)));

    //  iB = C*|B| - D*B#*A
    iB = C*sseVec(_mm_shuffle_ps(dB, dB, 0)) - iB;

    //  iC = B*|C| - A*C#*D;
    iC = B*sseVec(_mm_shuffle_ps(dC, dC, 0)) - iC;

    //  iX = iX / det
    iA *= rd;
    iB *= rd;
    iC *= rd;
    iD *= rd;

    BUN_SSE_STORE_APS(dest, _mm_shuffle_ps(iA, iB, 0x77));
    BUN_SSE_STORE_APS(dest + 4, _mm_shuffle_ps(iA, iB, 0x22));
    BUN_SSE_STORE_APS(dest + 8, _mm_shuffle_ps(iC, iD, 0x77));
    BUN_SSE_STORE_APS(dest + 12, _mm_shuffle_ps(iC, iD, 0x22));
  }
#endif

  // Multiply a 1x4 vector on the left with a 4x4 matrix on the right, resulting in a 1x4 vector held in an sseVec.
  template<typename T>
  [[nodiscard]] BUN_FORCEINLINE sseVecT<T> MatrixMultiply1x4(const T(&l)[4], const T(&r)[4][4])
  {
    return (sseVecT<T>(r[0])*sseVecT<T>(l[0])) + (sseVecT<T>(r[1])*sseVecT<T>(l[1])) + (sseVecT<T>(r[2])*sseVecT<T>(l[2])) + (sseVecT<T>(r[3])*sseVecT<T>(l[3]));
  }

  template<typename T, int M, int N, int P>
  BUN_FORCEINLINE void MatrixMultiply(const T(&l)[M][N], const T(&r)[N][P], T(&out)[M][P])
  {
    if constexpr(N == 4 && P == 4 && (std::is_same_v<T, float> || std::is_same_v<T, int32_t>))
    {
      using SSE = sseVecT<T>;
      assert(!(((size_t)l) % 16));
      assert(!(((size_t)r) % 16));
      assert(!(((size_t)out) % 16));
      SSE a(r[0]);
      SSE b(r[1]);
      SSE c(r[2]);
      SSE d(r[3]);

      if constexpr(M == 1)
        ((a*SSE(l[0][0])) + (b*SSE(l[0][1])) + (c*SSE(l[0][2])) + (d*SSE(l[0][3]))).Set(out[0]);
      else if constexpr(M == 4)
      {
        ((a*SSE(l[0][0])) + (b*SSE(l[0][1])) + (c*SSE(l[0][2])) + (d*SSE(l[0][3]))).Set(out[0]);
        ((a*SSE(l[1][0])) + (b*SSE(l[1][1])) + (c*SSE(l[1][2])) + (d*SSE(l[1][3]))).Set(out[1]);
        ((a*SSE(l[2][0])) + (b*SSE(l[2][1])) + (c*SSE(l[2][2])) + (d*SSE(l[2][3]))).Set(out[2]);
        ((a*SSE(l[3][0])) + (b*SSE(l[3][1])) + (c*SSE(l[3][2])) + (d*SSE(l[3][3]))).Set(out[3]);
      }
      else
      {
        for(int i = 0; i < M; ++i) // Note: It's ok if l, r, and out are all the same matrix because of the order they're accessed in.
          ((a*SSE(l[i][0])) + (b*SSE(l[i][1])) + (c*SSE(l[i][2])) + (d*SSE(l[i][3]))).Set(out[i]);
      }
    }
    else
    {
      T m[M][P];

      for(int i = 0; i < M; ++i)
      {
        for(int j = 0; j < P; ++j)
        {
          m[i][j] = l[i][0] * r[0][j];
          for(int k = 1; k < N; ++k)
            m[i][j] += l[i][k] * r[k][j];
        }
      }

      for(int i = 0; i < M; ++i) // Done so the compiler can get rid of it if it isn't necessary
        for(int j = 0; j < P; ++j)
          out[i][j] = m[i][j];
    }
  }

  namespace internal {
    template<typename T, int N>
    struct BUN_COMPILER_DLLEXPORT __MatrixDeterminant { };

    template<typename T>
    struct BUN_COMPILER_DLLEXPORT __MatrixDeterminant<T, 2>
    {
      [[nodiscard]] BUN_FORCEINLINE static T MD(const T(&x)[2][2]) { return D(x[0][0], x[0][1], x[1][0], x[1][1]); }
      [[nodiscard]] BUN_FORCEINLINE static T D(T a, T b, T c, T d) { return (a*d) - (b*c); }
    };

    template<typename T>
    struct BUN_COMPILER_DLLEXPORT __MatrixDeterminant<T, 3>
    {
      [[nodiscard]] BUN_FORCEINLINE static T MD(const T(&x)[3][3]) { return D(x[0][0], x[0][1], x[0][2], x[1][0], x[1][1], x[1][2], x[2][0], x[2][1], x[2][2]); }
      [[nodiscard]] BUN_FORCEINLINE static T D(T a, T b, T c, T d, T e, T f, T g, T h, T i) { return a*(e*i - f*h) - b*(i*d - f*g) + c*(d*h - e*g); }
    };

    template<>
    struct BUN_COMPILER_DLLEXPORT __MatrixDeterminant<float, 3>
    {
      [[nodiscard]] BUN_FORCEINLINE static float MD(const float(&x)[3][3]) { return D(x[0][0], x[0][1], x[0][2], x[1][0], x[1][1], x[1][2], x[2][0], x[2][1], x[2][2]); }
      [[nodiscard]] BUN_FORCEINLINE static float D(float a, float b, float c, float d, float e, float f, float g, float h, float i)
      {
        sseVec u(a, b, c, 0);
        sseVec v(e, i, d, f);
        sseVec w(i, d, h, g);

        sseVec r = u*(v*w - v.Shuffle<3, 3, 0, 0>()*w.Shuffle<2, 3, 3, 0>());
        float out;
        (r - r.Shuffle<1, 1, 1, 1>() + r.Shuffle<2, 2, 2, 2>()).Set(out);
        return out;
      }
    };

    template<typename T>
    struct BUN_COMPILER_DLLEXPORT __MatrixDeterminant<T, 4>
    {
      [[nodiscard]] BUN_FORCEINLINE static T MD(const T(&x)[4][4])
      {
        return x[0][0] * __MatrixDeterminant<T, 3>::D(x[1][1], x[1][2], x[1][3],
          x[2][1], x[2][2], x[2][3],
          x[3][1], x[3][2], x[3][3]) -
          x[0][1] * __MatrixDeterminant<T, 3>::D(x[1][0], x[1][2], x[1][3],
            x[2][0], x[2][2], x[2][3],
            x[3][0], x[3][2], x[3][3]) +
          x[0][2] * __MatrixDeterminant<T, 3>::D(x[1][0], x[1][1], x[1][3],
            x[2][0], x[2][1], x[2][3],
            x[3][0], x[3][1], x[3][3]) -
          x[0][3] * __MatrixDeterminant<T, 3>::D(x[1][0], x[1][1], x[1][2],
            x[2][0], x[2][1], x[2][2],
            x[3][0], x[3][1], x[3][2]);
      }
    };

    template<>
    struct BUN_COMPILER_DLLEXPORT __MatrixDeterminant<float, 4>
    {
      template<uint8_t I>
      [[nodiscard]] BUN_FORCEINLINE static BUN_SSE_M128 _mm_ror_ps(BUN_SSE_M128 vec) { return (((I) % 4) ? (BUN_SSE_SHUFFLE_PS(vec, vec, _MM_SHUFFLE((uint8_t)(I + 3) % 4, (uint8_t)(I + 2) % 4, (uint8_t)(I + 1) % 4, (uint8_t)(I + 0) % 4))) : vec); }
      [[nodiscard]] BUN_FORCEINLINE static float MD(const float(&x)[4][4])
      {
        //   Copyright (c) 2001 Intel Corporation.
        //
        // Permition is granted to use, copy, distribute and prepare derivative works 
        // of this library for any purpose and without fee, provided, that the above 
        // copyright notice and this statement appear in all copies.  
        // Intel makes no representations about the suitability of this software for 
        // any purpose, and specifically disclaims all warranties. 

        BUN_SSE_M128 Va, Vb, Vc;
        BUN_SSE_M128 r1, r2, r3, t1, t2, sum;
        sseVec _L1(x[0]);
        sseVec _L2(x[1]);
        sseVec _L3(x[2]);
        sseVec _L4(x[3]);

        // First, Let's calculate the first four minterms of the first line
        t1 = _L4; t2 = _mm_ror_ps<1>(_L3);
        Vc = BUN_SSE_MUL_PS(t2, _mm_ror_ps<0>(t1));                   // V3'�V4
        Va = BUN_SSE_MUL_PS(t2, _mm_ror_ps<2>(t1));                   // V3'�V4"
        Vb = BUN_SSE_MUL_PS(t2, _mm_ror_ps<3>(t1));                   // V3'�V4^

        r1 = BUN_SSE_SUB_PS(_mm_ror_ps<1>(Va), _mm_ror_ps<2>(Vc));     // V3"�V4^ - V3^�V4"
        r2 = BUN_SSE_SUB_PS(_mm_ror_ps<2>(Vb), _mm_ror_ps<0>(Vb));     // V3^�V4' - V3'�V4^
        r3 = BUN_SSE_SUB_PS(_mm_ror_ps<0>(Va), _mm_ror_ps<1>(Vc));     // V3'�V4" - V3"�V4'

        Va = _mm_ror_ps<1>(_L2);     sum = BUN_SSE_MUL_PS(Va, r1);
        Vb = _mm_ror_ps<1>(Va);      sum = BUN_SSE_ADD_PS(sum, BUN_SSE_MUL_PS(Vb, r2));
        Vc = _mm_ror_ps<1>(Vb);      sum = BUN_SSE_ADD_PS(sum, BUN_SSE_MUL_PS(Vc, r3));

        // Now we can calculate the determinant:
        BUN_SSE_M128 Det = BUN_SSE_MUL_PS(sum, _L1);
        Det = BUN_SSE_ADD_PS(Det, BUN_SSE_MOVEHL_PS(Det, Det));
        Det = BUN_SSE_SUB_SS(Det, BUN_SSE_SHUFFLE_PS(Det, Det, 1));
        return BUN_SSE_SS_F32(Det);
      }
    };
  }

  template<typename T, int N>
  [[nodiscard]] BUN_FORCEINLINE T MatrixDeterminant(const T(&x)[N][N]) { return internal::__MatrixDeterminant<T, N>::MD(x); }

  template<typename T, int N>
  BUN_FORCEINLINE void FromQuaternion(T(&q)[4], T(&out)[N][N])
  {
    T w = q[3], x = q[0], y = q[1], z = q[2];
    T xx = x*x, xy = x*y, xz = x*z, xw = x*w;
    T yy = y*y, yz = y*z, yw = y*w;
    T zz = z*z, zw = z*w;

    SetSubMatrix<T, 3, 3>::MM(out,
      1 - 2 * (yy + zz), 2 * (xy - zw), 2 * (xz + yw),
      2 * (xy + zw), 1 - 2 * (xx + zz), 2 * (yz - xw),
      2 * (xz - yw), 2 * (yz + xw), 1 - 2 * (xx + yy));
  }

  template<>
  BUN_FORCEINLINE void FromQuaternion<float, 4>(float(&q)[4], float(&out)[4][4])
  {
    sseVec x(q[0]);
    sseVec y(q[1]);
    sseVec z(q[2]);
    sseVec yxw(q[1], q[0], q[3], 0);
    sseVec zwx(q[2], q[3], q[0], 0);
    sseVec wzy(q[3], q[2], q[1], 0);
    const sseVec n010(1, -1, 1, 0);
    const sseVec n100(-1, 1, 1, 0);
    const sseVec n001(1, 1, -1, 0);

    (y*yxw + z*zwx*n010).Set(out[0]);
    (x*yxw + z*wzy*n001).Set(out[1]);
    (x*zwx + y*wzy*n100).Set(out[2]);
    sseVec::ZeroVector().Set(out[3]);
    out[3][3] = 1;
    out[1][2] = -out[1][2];

    /*
    y y + z z
    x y - z w
    w y + z x

    x y + z w
    x x + z z
    -(x w - z y)

    x z - y w
    x w + y z
    x x + y y*/
  }

  template<typename T, int M, int N>
  struct Matrix;

  // N-element vector
  template<typename T, int N>
  struct BUN_COMPILER_DLLEXPORT Vector
  {

    template<typename U>
    inline constexpr Vector(const Vector<U, N>& copy) { for(int i = 0; i < N; ++i) v[i] = (T)copy.v[i]; }
    inline explicit constexpr Vector(T scalar) { for(int i = 0; i < N; ++i) v[i] = scalar; }
    inline explicit constexpr Vector(const T(&e)[N]) { for(int i = 0; i < N; ++i) v[i] = e[i]; }
    inline explicit constexpr Vector(const std::array<T, N>& e) { for(int i = 0; i < N; ++i) v[i] = e[i]; }
    inline Vector(const std::initializer_list<T>& e) { int k = 0; for(const T* i = e.begin(); i != e.end() && k < N; ++i) v[k++] = *i; }
    inline constexpr Vector() {}
    [[nodiscard]] inline T Length() const { return FastSqrt<T>(Dot(*this)); }
    [[nodiscard]] inline Vector<T, N> Normalize() const { Vector<T, N> ret(*this); NVectorNormalize(v, ret.v); return ret; }
    [[nodiscard]] inline Vector<T, N> Abs() const { Vector<T, N> ret(*this); NVectorAbs(v, ret.v); return ret; }
    [[nodiscard]] inline T Dot(const Vector<T, N>& r) const { return NVectorDot<T, N>(v, r.v); }
    [[nodiscard]] inline T Distance(const Vector<T, N>& r) const { return FastSqrt<T>(DistanceSq(r)); }
    [[nodiscard]] inline T DistanceSq(const Vector<T, N>& r) const { return NVectorDistanceSq<T, N>(v, r.v); }
    template<int K> inline void Outer(const Vector<T, K>& r, T(&out)[N][K]) const { MatrixMultiply<T, N, 1, K>(v_column, r.v_row, out); }

    inline Vector<T, N>& operator=(const Vector<T, N>& r) { for(int i = 0; i < N; ++i) v[i] = r.v[i]; return *this; }
    template<typename U> inline Vector<T, N>& operator=(const Vector<U, N>& r) { for(int i = 0; i < N; ++i) v[i] = (T)r.v[i]; return *this; }
    template<int... K> BUN_FORCEINLINE Vector<T, sizeof...(K)> Swizzle() const
    {
      static_assert(sizeof...(K) <= N, "Too many swizzles!");
      return std::array<T, sizeof...(K)>{ v[K]... };
    }

    using SerializerArray = T;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { s.EvaluateFixedArray(v, id); }

    union {
      T v[N];
      T v_column[N][1];
      T v_row[1][N];
    };
  };

  template<typename T>
  struct BUN_COMPILER_DLLEXPORT Vector<T, 2>
  {
    template<typename U>
    inline constexpr Vector(const Vector<U, 2>& copy) : x((T)copy.v[0]), y((T)copy.v[1]) {}
    inline explicit constexpr Vector(T scalar) : x(scalar), y(scalar) {}
    inline explicit constexpr Vector(const T(&e)[2]) : x(e[0]), y(e[1]) {}
    inline explicit constexpr Vector(const std::array<T, 2>& e) : x(e[0]), y(e[1]) {}
    inline Vector(const std::initializer_list<T>& e) { int k = 0; for(const T* i = e.begin(); i != e.end() && k < 2; ++i) v[k++] = *i; }
    inline constexpr Vector(T X, T Y) : x(X), y(Y) {}
    inline constexpr Vector() {}
    [[nodiscard]] inline T Length() const { return FastSqrt<T>(Dot(*this)); }
    [[nodiscard]] inline Vector<T, 2> Normalize() const { T l = Length(); return Vector<T, 2>(x / l, y / l); }
    [[nodiscard]] inline Vector<T, 2> Abs() const { return Vector<T, 2>(NVectorAbsCall(x), NVectorAbsCall(y)); }
    [[nodiscard]] inline T Dot(const Vector<T, 2>& r) const { return DotProduct(r.x, r.y, x, y); }
    [[nodiscard]] inline T Distance(const Vector<T, 2>& r) const { return FastSqrt<T>(DistanceSq(r)); }
    [[nodiscard]] inline T DistanceSq(const Vector<T, 2>& r) const { return bun::DistSqr<T>(r.x, r.y, x, y); }
    [[nodiscard]] inline Vector<T, 2> Rotate(T R, const Vector<T, 2>& center) const { return Rotate(R, center.x, center.y); }
    [[nodiscard]] inline Vector<T, 2> Rotate(T R, T X, T Y) const { T tx = x; T ty = y; RotatePoint(tx, ty, R, X, Y); return Vector<T, 2>(tx, ty); }
    [[nodiscard]] inline T Cross(const Vector<T, 2>& r) const { return CrossProduct(r.x, r.y, x, y); }
    [[nodiscard]] inline T Cross(T X, T Y) const { return CrossProduct(X, Y, x, y); }
    template<int K> inline void Outer(const Vector<T, K>& r, T(&out)[2][K]) const { MatrixMultiply<T, 2, 1, K>(v_column, r.v_row, out); }

    template<typename U> inline Vector<T, 2>& operator=(const Vector<U, 2>& r) { x = (T)r.x; y = (T)r.y; return *this; }

    static BUN_FORCEINLINE void RotatePoint(T& x, T& y, T r, T cx, T cy) { T tx = x - cx; T ty = y - cy; T rcos = (T)cos(r); T rsin = (T)sin(r); x = (tx*rcos - ty*rsin) + cx; y = (ty*rcos + tx*rsin) + cy; }
    static BUN_FORCEINLINE T CrossProduct(T X, T Y, T x, T y) { return x*Y - X*y; }
    static BUN_FORCEINLINE T DotProduct(T X, T Y, T x, T y) { return X*x + Y*y; }
    static BUN_FORCEINLINE Vector<T, 2> FromPolar(const Vector<T, 2>& v) { return FromPolar(v.x, v.y); }
    static BUN_FORCEINLINE Vector<T, 2> FromPolar(T r, T angle) { return Vector<T, 2>((T)r*cos(angle), (T)r*sin(angle)); }
    static BUN_FORCEINLINE Vector<T, 2> ToPolar(const Vector<T, 2>& v) { return ToPolar(v.x, v.y); }
    static BUN_FORCEINLINE Vector<T, 2> ToPolar(T x, T y) { return Vector<T, 2>(bun::FastSqrt<T>((x*x) + (y*y)), (T)atan2(y, x)); } //x - r, y - theta

    inline Vector<T, 2> yx() const { return Vector<T, 2>(y, x); }

    using SerializerArray = T;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { s.EvaluateFixedArray(v, id); }

    union {
      T v[2];
      T v_column[2][1];
      T v_row[1][2];
      struct { T x, y; };
    };
  };

  template<typename T>
  struct BUN_COMPILER_DLLEXPORT Vector<T, 3>
  {
    template<typename U>
    inline constexpr Vector(const Vector<U, 3>& copy) : x((T)copy.v[0]), y((T)copy.v[1]), z((T)copy.v[2]) {}
    inline explicit constexpr Vector(T scalar) : x(scalar), y(scalar), z(scalar) {}
    inline explicit constexpr Vector(const T(&e)[3]) : x(e[0]), y(e[1]), z(e[2]) {}
    inline explicit constexpr Vector(const std::array<T, 3>& e) : x(e[0]), y(e[1]), z(e[2]) {}
    inline Vector(const std::initializer_list<T>& e) { int k = 0; for(const T* i = e.begin(); i != e.end() && k < 3; ++i) v[k++] = *i; }
    inline constexpr Vector(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
    inline constexpr Vector() {}
    [[nodiscard]] inline T Length() const { return FastSqrt((x*x) + (y*y) + (z*z)); }
    [[nodiscard]] inline Vector<T, 3> Normalize() const { T l = Length(); return Vector<T, 3>(x / l, y / l, z / l); }
    [[nodiscard]] inline Vector<T, 3> Abs() const { return Vector<T, 3>(NVectorAbsCall(x), NVectorAbsCall(y), NVectorAbsCall(z)); }
    [[nodiscard]] inline T Dot(const Vector<T, 3>& r) const { return (x*r.x) + (y*r.y) + (z*r.z); }
    [[nodiscard]] inline T Distance(const Vector<T, 3>& r) const { return FastSqrt<T>(DistanceSq(r)); }
    [[nodiscard]] inline T DistanceSq(const Vector<T, 3>& r) const { T tz = (r.z - z); T ty = (r.y - y); T tx = (r.x - x); return (T)((tx*tx) + (ty*ty) + (tz*tz)); }
    [[nodiscard]] inline Vector<T, 3> Cross(const Vector<T, 3>& r) const { return Cross(r.x, r.y, r.z); }
    [[nodiscard]] inline Vector<T, 3> Cross(T X, T Y, T Z) const { return Vector<T, 3>(y*Z - z*Y, z*X - x*Z, x*Y - X*y); }
    template<int K> inline void Outer(const Vector<T, K>& r, T(&out)[3][K]) const { MatrixMultiply<T, 3, 1, K>(v_column, r.v_row, out); }

    template<typename U> inline Vector<T, 3>& operator=(const Vector<U, 3>& r) { x = (T)r.x; y = (T)r.y; z = (T)r.z; return *this; }

    using SerializerArray = T;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { s.EvaluateFixedArray(v, id); }

    inline Vector<T, 2> xz() const { return Vector<T, 2>(x, z); }
    inline Vector<T, 2> zx() const { return Vector<T, 2>(z, x); }
    inline Vector<T, 2> zy() const { return Vector<T, 2>(z, y); }
    inline Vector<T, 2> yz() const { return Vector<T, 2>(y, z); }
    inline Vector<T, 2> yx() const { return Vector<T, 2>(y, x); }
    inline Vector<T, 3> xzy() const { return Vector<T, 3>(x, z, y); }
    inline Vector<T, 3> zxy() const { return Vector<T, 3>(z, x, y); }
    inline Vector<T, 3> zyx() const { return Vector<T, 3>(z, y, x); }
    inline Vector<T, 3> yzx() const { return Vector<T, 3>(y, z, x); }
    inline Vector<T, 3> yxz() const { return Vector<T, 3>(y, x, z); }

    union {
      T v[3];
      T v_column[3][1];
      T v_row[1][3];
      struct { T x, y, z; };
      struct { Vector<T, 2> xy; };
    };
  };

  template<typename T>
  struct BUN_COMPILER_DLLEXPORT Vector<T, 4>
  {
    template<typename U>
    inline constexpr Vector(const Vector<U, 4>& copy) : x((T)copy.v[0]), y((T)copy.v[1]), z((T)copy.v[2]), w((T)copy.v[3]) {}
    inline explicit constexpr Vector(T scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
    inline explicit constexpr Vector(const T(&e)[4]) : x(e[0]), y(e[1]), z(e[2]), w(e[3]) {}
    inline explicit constexpr Vector(const std::array<T, 4>& e) : x(e[0]), y(e[1]), z(e[2]), w(e[3]) {}
    inline Vector(const std::initializer_list<T>& e) { int k = 0; for(const T* i = e.begin(); i != e.end() && k < 4; ++i) v[k++] = *i; }
    inline constexpr Vector(T X, T Y, T Z, T W) : x(X), y(Y), z(Z), w(W) {}
    inline constexpr Vector() {}
    [[nodiscard]] inline T Length() const { return FastSqrt<T>(Dot(*this)); }
    [[nodiscard]] inline Vector<T, 4> Normalize() const { Vector<T, 4> ret; NVectorNormalize<T, 4>(v, ret.v); return ret; }
    [[nodiscard]] inline Vector<T, 4> Abs() const { return Vector<T, 4>(NVectorAbsCall(x), NVectorAbsCall(y), NVectorAbsCall(z), NVectorAbsCall(w)); }
    [[nodiscard]] inline T Dot(const Vector<T, 4>& right) const { return NVectorDot<T, 4>(v, right.v); }
    [[nodiscard]] inline T Distance(const Vector<T, 4>& right) const { return FastSqrt<T>(DistanceSq(right)); }
    [[nodiscard]] inline T DistanceSq(const Vector<T, 4>& right) const { T tz = (right.z - z); T ty = (right.y - y); T tx = (right.x - x); T tw = (right.w - w); return (T)((tx*tx) + (ty*ty) + (tz*tz) + (tw*tw)); }
    template<int K> inline void Outer(const Vector<T, K>& right, T(&out)[4][K]) const { MatrixMultiply<T, 4, 1, K>(v_column, right.v_row, out); }

    template<typename U> inline Vector<T, 4>& operator=(const Vector<U, 4>& copy) { x = (T)copy.x; y = (T)copy.y; z = (T)copy.z; w = (T)copy.w; return *this; }

    using SerializerArray = T;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { s.EvaluateFixedArray(v, id); }

    // SWIZZLES!!!!
    inline Vector<T, 2> xz() const { return Vector<T, 2>(x, z); }
    inline Vector<T, 2> xw() const { return Vector<T, 2>(x, w); }
    inline Vector<T, 2> zx() const { return Vector<T, 2>(z, x); }
    inline Vector<T, 2> zy() const { return Vector<T, 2>(z, y); }
    inline Vector<T, 2> zw() const { return Vector<T, 2>(z, w); }
    inline Vector<T, 2> yz() const { return Vector<T, 2>(y, z); }
    inline Vector<T, 2> yx() const { return Vector<T, 2>(y, x); }
    inline Vector<T, 2> yw() const { return Vector<T, 2>(y, w); }
    inline Vector<T, 3> xyw() const { return Vector<T, 3>(x, y, w); }
    inline Vector<T, 3> xzy() const { return Vector<T, 3>(x, z, y); }
    inline Vector<T, 3> xzw() const { return Vector<T, 3>(x, z, w); }
    inline Vector<T, 3> xwy() const { return Vector<T, 3>(x, w, y); }
    inline Vector<T, 3> xwz() const { return Vector<T, 3>(x, w, z); }
    inline Vector<T, 3> zxy() const { return Vector<T, 3>(z, x, y); }
    inline Vector<T, 3> zxw() const { return Vector<T, 3>(z, x, w); }
    inline Vector<T, 3> zyx() const { return Vector<T, 3>(z, y, x); }
    inline Vector<T, 3> zyw() const { return Vector<T, 3>(z, y, w); }
    inline Vector<T, 3> zwx() const { return Vector<T, 3>(z, w, x); }
    inline Vector<T, 3> zwy() const { return Vector<T, 3>(z, w, y); }
    inline Vector<T, 3> wxy() const { return Vector<T, 3>(w, x, y); }
    inline Vector<T, 3> wxz() const { return Vector<T, 3>(w, x, z); }
    inline Vector<T, 3> wyx() const { return Vector<T, 3>(w, y, x); }
    inline Vector<T, 3> wyz() const { return Vector<T, 3>(w, y, z); }
    inline Vector<T, 3> wzx() const { return Vector<T, 3>(w, z, x); }
    inline Vector<T, 3> wzy() const { return Vector<T, 3>(w, z, y); }
    inline Vector<T, 4> xywz() const { return Vector<T, 4>(x, y, w, z); }
    inline Vector<T, 4> xzyw() const { return Vector<T, 4>(x, z, y, w); }
    inline Vector<T, 4> xzwy() const { return Vector<T, 4>(x, z, w, y); }
    inline Vector<T, 4> xwyz() const { return Vector<T, 4>(x, w, y, z); }
    inline Vector<T, 4> xwzy() const { return Vector<T, 4>(x, w, z, y); }
    inline Vector<T, 4> zxyw() const { return Vector<T, 4>(z, x, y, w); }
    inline Vector<T, 4> zxwy() const { return Vector<T, 4>(z, x, w, y); }
    inline Vector<T, 4> zyxw() const { return Vector<T, 4>(z, y, x, w); }
    inline Vector<T, 4> zywx() const { return Vector<T, 4>(z, y, w, x); }
    inline Vector<T, 4> zwxy() const { return Vector<T, 4>(z, w, x, y); }
    inline Vector<T, 4> zwyx() const { return Vector<T, 4>(z, w, y, x); }
    inline Vector<T, 4> wxyz() const { return Vector<T, 4>(w, x, y, z); }
    inline Vector<T, 4> wxzy() const { return Vector<T, 4>(w, x, z, y); }
    inline Vector<T, 4> wyxz() const { return Vector<T, 4>(w, y, x, z); }
    inline Vector<T, 4> wyzx() const { return Vector<T, 4>(w, y, z, x); }
    inline Vector<T, 4> wzxy() const { return Vector<T, 4>(w, z, x, y); }
    inline Vector<T, 4> wzyx() const { return Vector<T, 4>(w, z, y, x); }

    BUN_ALIGNED_UNION(16)
    {
      T v[4];
      T v_column[4][1];
      T v_row[1][4];
      struct { T x, y, z, w; };
      struct { T r, g, b, a; };
      struct { Vector<T, 2> xy; };
      struct { Vector<T, 3> xyz; };
      struct { Vector<T, 3> rgb; };
    };
  };

  // MxN matrix (M rows and N columns)
  template<typename T, int M, int N>
  struct BUN_COMPILER_DLLEXPORT Matrix
  {
    static const int MIN = M < N ? M : N;

    template<typename U>
    inline Matrix(const Matrix<U, M, N>& copy) { T* p = v; U* cp = copy.v; for(int i = 0; i < M*N; ++i) p[i] = (T)cp[i]; }
    inline Matrix(const std::initializer_list<T>& l) { assert(l.size() == (M*N)); T* p = (T*)v; int k = 0; for(const T* i = l.begin(); i != l.end() && k < M*N; ++i) p[k++] = *i; }
    inline explicit Matrix(const T(&m)[M][N]) { memcpy(v, m, sizeof(T)*M*N); }
    inline constexpr Matrix() {}
    inline void Transpose(T(&out)[N][M]) const { Transpose(v, out); }
    inline void Transpose(Matrix& mat) const { Transpose(v, mat.v); }
    [[nodiscard]] inline Matrix<T, M, N> Transpose() const { Matrix<T, M, N> m; Transpose(v, m.v); return m; }

    inline Matrix<T, M, N>& operator=(const Matrix<T, M, N>& r) { memcpy(v, r.v, sizeof(T)*M*N); return *this; }
    template<typename U> inline Matrix<T, M, N>& operator=(const Matrix<U, M, N>& r) { T* p = v; U* cp = r.v; for(int i = 0; i < M*N; ++i) p[i] = (T)cp[i]; return *this; }

    BUN_FORCEINLINE static void Diagonal(const Vector<T, MIN>& d, Matrix& mat) { Diagonal(d.v, mat.v); }
    BUN_FORCEINLINE static void Diagonal(const T(&d)[MIN], Matrix& mat) { Diagonal(d.v, mat.v); }
    BUN_FORCEINLINE static void Diagonal(const Vector<T, MIN>& d, T(&out)[M][N]) { Diagonal(d.v, out); }
    BUN_FORCEINLINE static void Diagonal(const T(&d)[MIN], T(&out)[M][N])
    {
      memset(out, 0, sizeof(T)*M*N);
      for(int i = 0; i < MIN; ++i)
        out[i][i] = d[i];
    }
    BUN_FORCEINLINE static Matrix<T, M, N> Diagonal(const T(&d)[MIN]) { Matrix<T, M, N> m; Diagonal(d, m.v); return m; }

    static inline void Transpose(const T(&in)[M][N], T(&out)[N][M])
    {
      for(int i = 0; i < M; ++i)
        for(int j = 0; j < N; ++j)
          out[j][i] = in[i][j];
    }
    static inline void Identity(Matrix<T, M, N>& m) { Identity(m.v); }
    static inline void Identity(T(&m)[M][N])
    {
      memset(m, 0, sizeof(T)*M*N);
      for(int i = 0; i < M && i < N; ++i)
        m[i][i] = 1;
    }
    static inline Matrix<T, M, N> Identity()
    {
      Matrix<T, M, N> m;
      Identity(m);
      return m;
    }

    using SerializerArray = T;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { s.EvaluateFixedArray(v, id); }

    T v[M][N];
  };

  template<typename T>
  struct BUN_COMPILER_DLLEXPORT Matrix<T, 2, 2>
  {
    template<typename U>
    inline Matrix(const Matrix<U, 2, 2>& copy) : a((T)copy.a), b((T)copy.b), c((T)copy.c), d((T)copy.d) {}
    inline Matrix(const std::initializer_list<T>& l) { assert(l.size() == (2 * 2)); T* p = (T*)v; int k = 0; for(const T* i = l.begin(); i != l.end() && k < 2 * 2; ++i) p[k++] = *i; }
    inline explicit Matrix(const T(&m)[2][2]) : a(m[0][0]), b(m[0][1]), c(m[1][0]), d(m[1][1]) {}
    inline constexpr Matrix() {}
    inline void Transpose(Matrix& mat) const { Transpose(mat.v); }
    inline void Transpose(T(&out)[2][2]) const { SetSubMatrix<T, 2, 2, true>::M2x2(out, a, b, c, d); }
    [[nodiscard]] inline Matrix Transpose() const { Matrix m; Transpose(v, m.v); return m; }
    [[nodiscard]] inline T Determinant() const { return MatrixDeterminant<T, 2>(v); }
    inline void Inverse(Matrix& mat) const { Inverse(mat.v); }
    inline void Inverse(T(&out)[2][2]) const { T invd = ((T)1) / Determinant(); SetSubMatrix<T, 2, 2>::M2x2(out, d*invd, -b*invd, -c*invd, a*invd); }
    [[nodiscard]] inline Matrix Inverse() const { Matrix m; Inverse(m.v); return m; }

    inline Matrix<T, 2, 2>& operator=(const Matrix<T, 2, 2>& r) { a = r.a; b = r.b; c = r.c; d = r.d; return *this; }
    template<typename U> inline Matrix<T, 2, 2>& operator=(const Matrix<U, 2, 2>& r) { a = (T)r.a; b = (T)r.b; c = (T)r.c; d = (T)r.d; return *this; }

    template<bool Tp>
    BUN_FORCEINLINE static void __Rotation(T r, T(&out)[2][2]) { T c = cos(r); T s = sin(r); SetSubMatrix<T, 2, 2, Tp>::M2x2(out, c, -s, s, c); }
    BUN_FORCEINLINE static void Rotation(T r, T(&out)[2][2]) { __Rotation<false>(r, out); }
    BUN_FORCEINLINE static void Rotation_T(T r, T(&out)[2][2]) { __Rotation<true>(r, out); }
    BUN_FORCEINLINE static void Rotation(T r, Matrix& mat) { __Rotation<false>(r, mat.v); }
    BUN_FORCEINLINE static void Rotation_T(T r, Matrix& mat) { __Rotation<true>(r, mat.v); }
    BUN_FORCEINLINE static void Diagonal(T x, T y, Matrix& mat) { Diagonal(x, y, mat.v); }
    BUN_FORCEINLINE static void Diagonal(const Vector<T, 2>& d, Matrix& mat) { Diagonal(d, mat.v); }
    BUN_FORCEINLINE static void Diagonal(T x, T y, T(&out)[2][2]) { SetSubMatrix<T, 2, 2>::M2x2(out, x, 0, 0, y); }
    BUN_FORCEINLINE static void Diagonal(const Vector<T, 2>& d, T(&out)[2][2]) { Diagonal(d.v[0], d.v[1], out); }
    BUN_FORCEINLINE static void Diagonal(const T(&d)[2], T(&out)[2][2]) { Diagonal(d[0], d[1], out); }
    BUN_FORCEINLINE static Matrix Diagonal(const T(&d)[2]) { Matrix m; Diagonal(d, m.v); return m; }
    static inline void Transpose(const T(&in)[2][2], T(&out)[2][2]) { SetSubMatrix<T, 2, 2, true>::M2x2(out, in[0][0], in[0][1], in[1][0], in[1][1]); }

    BUN_FORCEINLINE static void Identity(Matrix<T, 2, 2>& n) { Identity(n.v); }
    BUN_FORCEINLINE static void Identity(T(&n)[2][2]) { Diagonal(1, 1, n); }
    BUN_FORCEINLINE static Matrix Identity() { Matrix m; Identity(m); return m; }

    using SerializerArray = T;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { s.EvaluateFixedArray(v, id); }

    union {
      T v[2][2];
      struct { T a, b, c, d; };
    };
  };

  template<typename T>
  struct BUN_COMPILER_DLLEXPORT Matrix<T, 3, 3>
  {
    template<typename U>
    inline Matrix(const Matrix<U, 3, 3>& copy) : a((T)copy.a), b((T)copy.b), c((T)copy.c), d((T)copy.d), e((T)copy.e), f((T)copy.f), g((T)copy.g), h((T)copy.h), i((T)copy.i) {}
    inline Matrix(const std::initializer_list<T>& l) { assert(l.size() == (3 * 3)); T* p = (T*)v; int k = 0; for(const T* j = l.begin(); j != l.end() && k < 3 * 3; ++j) p[k++] = *j; }
    inline explicit Matrix(const T(&m)[3][3]) : a(m[0][0]), b(m[0][1]), c(m[0][2]), d(m[1][0]), e(m[1][1]), f(m[1][2]), g(m[2][0]), h(m[2][1]), i(m[2][2]) {}
    inline constexpr Matrix() {}
    inline void Transpose(Matrix& mat) const { Transpose(mat.v); }
    inline void Transpose(T(&out)[3][3]) const { SetSubMatrix<T, 3, 3, true>::M3x3(out, a, b, c, d, e, f, g, h, i); }
    [[nodiscard]] inline Matrix Transpose() const { Matrix m; Transpose(v, m.v); return m; }
    [[nodiscard]] inline T Determinant() const { return MatrixDeterminant<T, 3>(v); }
    inline void Inverse(Matrix& mat) const { Inverse(mat.v); }
    inline void Inverse(T(&out)[3][3]) const
    {
      T invd = ((T)1) / Determinant();
      SetSubMatrix<T, 3, 3>::M3x3(out,
        invd*(e*i - f*h), invd*(c*h - b*i), invd*(b*f - c*e),
        invd*(f*g - d*i), invd*(a*i - c*g), invd*(c*d - a*f),
        invd*(d*h - e*g), invd*(b*g - a*h), invd*(a*e - b*d));
    }
    inline Matrix Inverse() const { Matrix m; Inverse(m.v); return m; }

    inline Matrix<T, 3, 3>& operator=(const Matrix<T, 3, 3>& r) { a = r.a; b = r.b; c = r.c; d = r.d; e = r.e; f = r.f; g = r.g; h = r.h; i = r.i; return *this; }
    template<typename U> inline Matrix<T, 3, 3>& operator=(const Matrix<U, 3, 3>& r) { a = (T)r.a; b = (T)r.b; c = (T)r.c; d = (T)r.d; e = (T)r.e; f = (T)r.f; g = (T)r.g; h = (T)r.h; i = (T)r.i; return *this; }

    static BUN_FORCEINLINE void AffineRotation(T r, T(&out)[3][3]) { RotationZ(r, out); }
    static BUN_FORCEINLINE void AffineRotation_T(T r, T(&out)[3][3]) { RotationZ_T(r, out); }
    static BUN_FORCEINLINE void AffineScaling(T x, T y, T(&out)[3][3]) { Diagonal(x, y, 1, out); }
    static BUN_FORCEINLINE void AffineRotation(T r, Matrix& mat) { RotationZ(r, mat.v); }
    static BUN_FORCEINLINE void AffineRotation_T(T r, Matrix& mat) { RotationZ_T(r, mat.v); }
    static BUN_FORCEINLINE void AffineScaling(T x, T y, Matrix& mat) { Diagonal(x, y, 1, mat.v); }
    template<bool Tp> // Equivilent to (M_rc)^-1 * M_r * M_rc * M_t
    static inline void __AffineTransform(T x, T y, T r, T cx, T cy, T(&out)[3][3])
    {
      T c = cos(r);
      T s = sin(r);
      SetSubMatrix<T, 3, 3, Tp>::M3x3(out, c, -s, x - c*cx + cx + cy*s, s, c, y - c*cy + cy - cx*s, 0, 0, 1);
    }
    static BUN_FORCEINLINE void AffineTransform(T x, T y, T r, T cx, T cy, T(&out)[3][3]) { __AffineTransform<false>(x, y, r, cx, cy, out); }
    static BUN_FORCEINLINE void AffineTransform_T(T x, T y, T r, T cx, T cy, T(&out)[3][3]) { __AffineTransform<true>(x, y, r, cx, cy, out); }
    static BUN_FORCEINLINE void AffineTransform(T x, T y, T r, T cx, T cy, Matrix& mat) { __AffineTransform<false>(x, y, r, cx, cy, mat.v); }
    static BUN_FORCEINLINE void AffineTransform_T(T x, T y, T r, T cx, T cy, Matrix& mat) { __AffineTransform<true>(x, y, r, cx, cy, mat.v); }
    static BUN_FORCEINLINE void Translation(T x, T y, T(&out)[3][3]) { SetSubMatrix<T, 3, 3>::M3x3(out, 1, 0, x, 0, 1, y, 0, 0, 1); }
    static BUN_FORCEINLINE void Translation_T(T x, T y, T(&out)[3][3]) { SetSubMatrix<T, 3, 3, true>::M3x3(out, 1, 0, x, 0, 1, y, 0, 0, 1); }
    static BUN_FORCEINLINE void Translation(T x, T y, Matrix& mat) { Translation(x, y, mat.v); }
    static BUN_FORCEINLINE void Translation_T(T x, T y, Matrix& mat) { Translation_T(x, y, mat.v); }
    template<bool Tp> static BUN_FORCEINLINE void __RotationX(T r, T(&out)[3][3]) { T c = cos(r); T s = sin(r); SetSubMatrix<T, 3, 3, Tp>::M3x3(out, 1, 0, 0, 0, c, -s, 0, s, c); }
    template<bool Tp> static BUN_FORCEINLINE void __RotationY(T r, T(&out)[3][3]) { T c = cos(r); T s = sin(r); SetSubMatrix<T, 3, 3, Tp>::M3x3(out, c, 0, s, 0, 1, 0, -s, 0, c); }
    template<bool Tp> static BUN_FORCEINLINE void __RotationZ(T r, T(&out)[3][3]) { T c = cos(r); T s = sin(r); SetSubMatrix<T, 3, 3, Tp>::M3x3(out, c, -s, 0, s, c, 0, 0, 0, 1); }
    static BUN_FORCEINLINE void RotationX(T r, T(&out)[3][3]) { __RotationX<false>(r, out); }
    static BUN_FORCEINLINE void RotationX_T(T r, T(&out)[3][3]) { __RotationX<true>(r, out); }
    static BUN_FORCEINLINE void RotationY(T r, T(&out)[3][3]) { __RotationY<false>(r, out); }
    static BUN_FORCEINLINE void RotationY_T(T r, T(&out)[3][3]) { __RotationY<true>(r, out); }
    static BUN_FORCEINLINE void RotationZ(T r, T(&out)[3][3]) { __RotationZ<false>(r, out); }
    static BUN_FORCEINLINE void RotationZ_T(T r, T(&out)[3][3]) { __RotationZ<true>(r, out); }
    static BUN_FORCEINLINE void RotationX(T r, Matrix& mat) { __RotationX<false>(r, mat.v); }
    static BUN_FORCEINLINE void RotationX_T(T r, Matrix& mat) { __RotationX<true>(r, mat.v); }
    static BUN_FORCEINLINE void RotationY(T r, Matrix& mat) { __RotationY<false>(r, mat.v); }
    static BUN_FORCEINLINE void RotationY_T(T r, Matrix& mat) { __RotationY<true>(r, mat.v); }
    static BUN_FORCEINLINE void RotationZ(T r, Matrix& mat) { __RotationZ<false>(r, mat.v); }
    static BUN_FORCEINLINE void RotationZ_T(T r, Matrix& mat) { __RotationZ<true>(r, mat.v); }
    //static BUN_FORCEINLINE void FromQuaternion(T(&q)[4], T(&out)[3][3]) {  }
    static BUN_FORCEINLINE void Diagonal(T x, T y, T z, Matrix& mat) { Diagonal(x, y, z, mat.v); }
    static BUN_FORCEINLINE void Diagonal(const Vector<T, 3>& d, Matrix& mat) { Diagonal(d, mat.v); }
    static BUN_FORCEINLINE void Diagonal(T x, T y, T z, T(&out)[3][3]) { SetSubMatrix<T, 3, 3>::M3x3(out, x, 0, 0, 0, y, 0, 0, 0, z); }
    static BUN_FORCEINLINE void Diagonal(const Vector<T, 3>& d, T(&out)[3][3]) { Diagonal(d.v[0], d.v[1], d.v[2], out); }
    static BUN_FORCEINLINE void Diagonal(const T(&d)[3], T(&out)[3][3]) { Diagonal(d[0], d[1], d[2], out); }
    static BUN_FORCEINLINE Matrix Diagonal(const T(&d)[3]) { Matrix m; Diagonal(d, m.v); return m; }
    static inline void Transpose(const T(&in)[3][3], T(&out)[3][3]) { SetSubMatrix<T, 3, 3, true>::M3x3(out, in[0][0], in[0][1], in[0][2], in[1][0], in[1][1], in[1][2], in[2][0], in[2][1], in[2][2]); }

    static BUN_FORCEINLINE void Identity(Matrix<T, 3, 3>& m) { Identity(m.v); }
    static BUN_FORCEINLINE void Identity(T(&m)[3][3]) { Diagonal(1, 1, 1, m); }
    static BUN_FORCEINLINE Matrix Identity() { Matrix m; Identity(m); return m; }

    using SerializerArray = T;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { s.EvaluateFixedArray(v, id); }

    union {
      T v[3][3];
      struct { T a, b, c, d, e, f, g, h, i; };
    };
  };

  template<typename T>
  struct BUN_COMPILER_DLLEXPORT Matrix<T, 4, 4>
  {
    template<typename U>
    inline Matrix(const Matrix<U, 4, 4>& copy) : a((T)copy.a), b((T)copy.b), c((T)copy.c), d((T)copy.d), e((T)copy.e), f((T)copy.f), g((T)copy.g), h((T)copy.h), i((T)copy.i), j((T)copy.j), k((T)copy.k), l((T)copy.l), m((T)copy.m), n((T)copy.n), o((T)copy.o), p((T)copy.p) {}
    inline Matrix(const std::initializer_list<T>& list) { assert(list.size() == (4 * 4)); T* ptr = (T*)v; int index = 0; for(const T* u = list.begin(); u != list.end() && index < 4 * 4; ++u) ptr[index++] = *u; }
    inline explicit Matrix(const T(&mat)[4][4]) { memcpy(v, mat, sizeof(T) * 4 * 4); }
    inline constexpr Matrix() {}
    inline void Transpose(Matrix& mat) const { Transpose(mat.v); }
    inline void Transpose(T(&out)[4][4]) const { SetSubMatrix<T, 4, 4, true>::M4x4(out, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p); }
    [[nodiscard]] inline Matrix Transpose() const { Matrix mat; Transpose(v, mat.v); return mat; }
    [[nodiscard]] inline T Determinant() const { return MatrixDeterminant<T, 4>(v); }
    inline void Inverse(Matrix& mat) const { Inverse(mat.v); }
    inline void Inverse(T(&out)[4][4]) const { MatrixInvert4x4<T>((T*)v, (T*)out); }
    [[nodiscard]] inline Matrix Inverse() const { Matrix mat; Inverse(mat.v); return mat; }

    // Note: The assignment operator here isn't a memcpy() so the compiler can inline the assignment and then remove it entirely if applicable.
    inline Matrix<T, 4, 4>& operator=(const Matrix<T, 4, 4>& r) { a = r.a; b = r.b; c = r.c; d = r.d; e = r.e; f = r.f; g = r.g; h = r.h; i = r.i; j = r.j; k = r.k; l = r.l; m = r.m; n = r.n; o = r.o; p = r.p; return *this; }
    template<typename U> inline Matrix<T, 4, 4>& operator=(const Matrix<U, 4, 4>& r) { a = (T)r.a; b = (T)r.b; c = (T)r.c; d = (T)r.d; e = (T)r.e; f = (T)r.f; g = (T)r.g; h = (T)r.h; i = (T)r.i; j = (T)r.j; k = (T)r.k; l = (T)r.l; m = (T)r.m; n = (T)r.n; o = (T)r.o; p = (T)r.p; return *this; }

    static inline void AffineScaling(T x, T y, T z, Matrix& mat) { Diagonal(x, y, z, 1, mat.v); }
    static inline void AffineScaling(T x, T y, T z, T(&out)[4][4]) { Diagonal(x, y, z, 1, out); }
    template<bool Tp> // Equivilent to (M_rc)^-1 * M_r * M_rc * M_t
    static void __AffineTransform(T x, T y, T z, T rz, T cx, T cy, T(&out)[4][4])
    {
      T c = cos(rz);
      T s = sin(rz);
      SetSubMatrix<T, 4, 4, Tp>::M4x4(out, c, -s, 0, x - c*cx + cx + cy*s, s, c, 0, y - c*cy + cy - cx*s, 0, 0, 1, z, 0, 0, 0, 1);
    }
    static BUN_FORCEINLINE void AffineTransform(T x, T y, T z, T rz, T cx, T cy, T(&out)[4][4]) { __AffineTransform<false>(x, y, z, rz, cx, cy, out); }
    static BUN_FORCEINLINE void AffineTransform_T(T x, T y, T z, T rz, T cx, T cy, T(&out)[4][4]) { __AffineTransform<true>(x, y, z, rz, cx, cy, out); }
    static BUN_FORCEINLINE void AffineTransform(T x, T y, T z, T rz, T cx, T cy, Matrix& mat) { __AffineTransform<false>(x, y, z, rz, cx, cy, mat.v); }
    static BUN_FORCEINLINE void AffineTransform_T(T x, T y, T z, T rz, T cx, T cy, Matrix& mat) { __AffineTransform<true>(x, y, z, rz, cx, cy, mat.v); }
    template<bool Tp> static BUN_FORCEINLINE void __Translation(T x, T y, T z, T(&out)[4][4]) { SetSubMatrix<T, 4, 4, Tp>::M4x4(out, 1, 0, 0, x, 0, 1, 0, y, 0, 0, 1, z, 0, 0, 0, 1); }
    template<bool Tp> static BUN_FORCEINLINE void __AffineRotationX(T r, T(&out)[4][4]) { T c = cos(r); T s = sin(r); SetSubMatrix<T, 4, 4, Tp>::M4x4(out, 1, 0, 0, 0, 0, c, -s, 0, 0, s, c, 0, 0, 0, 0, 1); }
    template<bool Tp> static BUN_FORCEINLINE void __AffineRotationY(T r, T(&out)[4][4]) { T c = cos(r); T s = sin(r); SetSubMatrix<T, 4, 4, Tp>::M4x4(out, c, 0, s, 0, 0, 1, 0, 0, -s, 0, c, 0, 0, 0, 0, 1); }
    template<bool Tp> static BUN_FORCEINLINE void __AffineRotationZ(T r, T(&out)[4][4]) { T c = cos(r); T s = sin(r); SetSubMatrix<T, 4, 4, Tp>::M4x4(out, c, -s, 0, 0, s, c, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1); }
    static BUN_FORCEINLINE void Translation(T x, T y, T z, T(&out)[4][4]) { __Translation<false>(x, y, z, out); }
    static BUN_FORCEINLINE void Translation_T(T x, T y, T z, T(&out)[4][4]) { __Translation<true>(x, y, z, out); }
    static BUN_FORCEINLINE void AffineRotationX(T r, T(&out)[4][4]) { __AffineRotationX<false>(r, out); }
    static BUN_FORCEINLINE void AffineRotationX_T(T r, T(&out)[4][4]) { __AffineRotationX<true>(r, out); }
    static BUN_FORCEINLINE void AffineRotationY(T r, T(&out)[4][4]) { __AffineRotationY<false>(r, out); }
    static BUN_FORCEINLINE void AffineRotationY_T(T r, T(&out)[4][4]) { __AffineRotationY<true>(r, out); }
    static BUN_FORCEINLINE void AffineRotationZ(T r, T(&out)[4][4]) { __AffineRotationZ<false>(r, out); }
    static BUN_FORCEINLINE void AffineRotationZ_T(T r, T(&out)[4][4]) { __AffineRotationZ<true>(r, out); }
    static BUN_FORCEINLINE void Translation(T x, T y, T z, Matrix& mat) { __Translation<false>(x, y, z, mat.v); }
    static BUN_FORCEINLINE void Translation_T(T x, T y, T z, Matrix& mat) { __Translation<true>(x, y, z, mat.v); }
    static BUN_FORCEINLINE void AffineRotationX(T r, Matrix& mat) { __AffineRotationX<false>(r, mat.v); }
    static BUN_FORCEINLINE void AffineRotationX_T(T r, Matrix& mat) { __AffineRotationX<true>(r, mat.v); }
    static BUN_FORCEINLINE void AffineRotationY(T r, Matrix& mat) { __AffineRotationY<false>(r, mat.v); }
    static BUN_FORCEINLINE void AffineRotationY_T(T r, Matrix& mat) { __AffineRotationY<true>(r, mat.v); }
    static BUN_FORCEINLINE void AffineRotationZ(T r, Matrix& mat) { __AffineRotationZ<false>(r, mat.v); }
    static BUN_FORCEINLINE void AffineRotationZ_T(T r, Matrix& mat) { __AffineRotationZ<true>(r, mat.v); }
    static BUN_FORCEINLINE void Diagonal(T x, T y, T z, T w, Matrix& mat) { Diagonal(x, y, z, w, mat.v); }
    static BUN_FORCEINLINE void Diagonal(const Vector<T, 4>& d, Matrix& mat) { Diagonal(d, mat.v); }
    static BUN_FORCEINLINE void Diagonal(T x, T y, T z, T w, T(&out)[4][4]) { SetSubMatrix<T, 4, 4>::M4x4(out, x, 0, 0, 0, 0, y, 0, 0, 0, 0, z, 0, 0, 0, 0, w); }
    static BUN_FORCEINLINE void Diagonal(const Vector<T, 4>& d, T(&out)[4][4]) { Diagonal(d.v[0], d.v[1], d.v[2], d.v[3], out); }
    static BUN_FORCEINLINE void Diagonal(const T(&d)[4], T(&out)[4][4]) { Diagonal(d[0], d[1], d[2], d[3], out); }
    static BUN_FORCEINLINE Matrix Diagonal(const T(&d)[4]) { Matrix m; Diagonal(d, m.v); return m; }
    static inline void Transpose(const T(&in)[4][4], T(&out)[4][4]) { SetSubMatrix<T, 4, 4, true>::M4x4(out, in[0][0], in[0][1], in[0][2], in[0][3], in[1][0], in[1][1], in[1][2], in[1][3], in[2][0], in[2][1], in[2][2], in[2][3], in[3][0], in[3][1], in[3][2], in[3][3]); }

    static BUN_FORCEINLINE void Identity(Matrix<T, 4, 4>& m) { Identity(m.v); }
    static BUN_FORCEINLINE void Identity(T(&m)[4][4]) { Diagonal(1, 1, 1, 1, m); }
    static BUN_FORCEINLINE Matrix Identity() { Matrix m; Identity(m); return m; }

    using SerializerArray = T;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { s.EvaluateFixedArray(v, id); }

    BUN_ALIGNED_UNION(16)
    {
      T v[4][4];
      struct { T a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p; };
    };
  };

  template<typename T, typename U, int N> inline Vector<T, N> operator +(const Vector<T, N>& l, const Vector<U, N>& r) { Vector<T, N> n(l); n += r; return n; }
  template<typename T, typename U, int N> inline Vector<T, N> operator -(const Vector<T, N>& l, const Vector<U, N>& r) { Vector<T, N> n(l); n -= r; return n; }
  template<typename T, typename U, int N> inline Vector<T, N> operator *(const Vector<T, N>& l, const Vector<U, N>& r) { Vector<T, N> n(l); n *= r; return n; }
  template<typename T, typename U, int N> inline Vector<T, N> operator /(const Vector<T, N>& l, const Vector<U, N>& r) { Vector<T, N> n(l); n /= r; return n; }
  template<typename T, int N> inline Vector<T, N> operator +(const Vector<T, N>& l, const T scalar) { Vector<T, N> n(l); n += scalar; return n; }
  template<typename T, int N> inline Vector<T, N> operator -(const Vector<T, N>& l, const T scalar) { Vector<T, N> n(l); n -= scalar; return n; }
  template<typename T, int N> inline Vector<T, N> operator *(const Vector<T, N>& l, const T scalar) { Vector<T, N> n(l); n *= scalar; return n; }
  template<typename T, int N> inline Vector<T, N> operator /(const Vector<T, N>& l, const T scalar) { Vector<T, N> n(l); n /= scalar; return n; }
  template<typename T, int N> inline Vector<T, N> operator +(const T scalar, const Vector<T, N>& r) { Vector<T, N> n(r); n += scalar; return n; }
  template<typename T, int N> inline Vector<T, N> operator -(const T scalar, const Vector<T, N>& r)
  {
    Vector<T, N> ret(r); for(int i = 0; i < N; ++i) ret.v[i] = scalar - r.v[i]; return ret;
  }
  template<typename T, int N> inline Vector<T, N> operator *(const T scalar, const Vector<T, N>& r) { Vector<T, N> n(r); n *= scalar; return n; }
  template<typename T, int N> inline Vector<T, N> operator /(const T scalar, const Vector<T, N>& r)
  {
    Vector<T, N> ret(r); for(int i = 0; i < N; ++i) ret.v[i] = scalar / r.v[i]; return ret;
  }

  template<typename T, typename U, int N> inline Vector<T, N>& operator +=(Vector<T, N>& l, const Vector<U, N>& r) { for(int i = 0; i < N; ++i) l.v[i] += (T)r.v[i]; return l; }
  template<typename T, typename U, int N> inline Vector<T, N>& operator -=(Vector<T, N>& l, const Vector<U, N>& r) { for(int i = 0; i < N; ++i) l.v[i] -= (T)r.v[i]; return l; }
  template<typename T, typename U, int N> inline Vector<T, N>& operator *=(Vector<T, N>& l, const Vector<U, N>& r) { for(int i = 0; i < N; ++i) l.v[i] *= (T)r.v[i]; return l; }
  template<typename T, typename U, int N> inline Vector<T, N>& operator /=(Vector<T, N>& l, const Vector<U, N>& r) { for(int i = 0; i < N; ++i) l.v[i] /= (T)r.v[i]; return l; }
  template<typename T, int N> inline Vector<T, N>& operator +=(Vector<T, N>& l, const T scalar) { for(int i = 0; i < N; ++i) l.v[i] += scalar; return l; }
  template<typename T, int N> inline Vector<T, N>& operator -=(Vector<T, N>& l, const T scalar) { for(int i = 0; i < N; ++i) l.v[i] -= scalar; return l; }
  template<typename T, int N> inline Vector<T, N>& operator *=(Vector<T, N>& l, const T scalar) { for(int i = 0; i < N; ++i) l.v[i] *= scalar; return l; }
  template<typename T, int N> inline Vector<T, N>& operator /=(Vector<T, N>& l, const T scalar) { for(int i = 0; i < N; ++i) l.v[i] /= scalar; return l; }

  template<typename T, int N> inline Vector<T, N> operator -(const Vector<T, N>& l) { for(int i = 0; i < N; ++i) l.v[i] = -l.v[i]; return l; }

  template<typename T, int N> inline bool operator !=(const Vector<T, N>& l, const Vector<T, N>& r) { bool ret = false; for(int i = 0; i < N; ++i) ret = ret || (l.v[i] != r.v[i]); return ret; }
  template<typename T, int N> inline bool operator ==(const Vector<T, N>& l, const Vector<T, N>& r) { bool ret = true; for(int i = 0; i < N; ++i) ret = ret && (l.v[i] == r.v[i]); return ret; }
  template<typename T, int N> inline bool operator !=(const Vector<T, N>& l, const T scalar) { bool ret = false; for(int i = 0; i < N; ++i) ret = ret || (l.v[i] != scalar); return ret; }
  template<typename T, int N> inline bool operator ==(const Vector<T, N>& l, const T scalar) { bool ret = true; for(int i = 0; i < N; ++i) ret = ret && (l.v[i] == scalar); return ret; }
  template<typename T, int N> inline bool operator >(const Vector<T, N>& l, const Vector<T, N>& r) { char c = 0; for(int i = 0; i < N && !c; ++i) c = SGNCOMPARE(l.v[i], r.v[i]); return c > 0; }
  template<typename T, int N> inline bool operator <(const Vector<T, N>& l, const Vector<T, N>& r) { char c = 0; for(int i = 0; i < N && !c; ++i) c = SGNCOMPARE(l.v[i], r.v[i]); return c < 0; }
  template<typename T, int N> inline bool operator >=(const Vector<T, N>& l, const Vector<T, N>& r) { return !operator<(l, r); }
  template<typename T, int N> inline bool operator <=(const Vector<T, N>& l, const Vector<T, N>& r) { return !operator>(l, r); }
  template<typename T, int N> inline bool operator >(const Vector<T, N>& l, const T scalar) { bool ret = true; for(int i = 0; i < N; ++i) ret = ret && (l.v[i] > scalar); return ret; }
  template<typename T, int N> inline bool operator <(const Vector<T, N>& l, const T scalar) { bool ret = true; for(int i = 0; i < N; ++i) ret = ret && (l.v[i] < scalar); return ret; }
  template<typename T, int N> inline bool operator >=(const Vector<T, N>& l, const T scalar) { bool ret = true; for(int i = 0; i < N; ++i) ret = ret && (l.v[i] >= scalar); return ret; }
  template<typename T, int N> inline bool operator <=(const Vector<T, N>& l, const T scalar) { bool ret = true; for(int i = 0; i < N; ++i) ret = ret && (l.v[i] <= scalar); return ret; }

  // Component-wise matrix operations. Note that component-wise matrix multiplication is ^
  template<typename T, int M, int N> inline Matrix<T, M, N> operator +(const Matrix<T, M, N>& l, const Matrix<T, M, N>& r) { Matrix<T, M, N> n(l); n += r; return n; }
  template<typename T, int M, int N> inline Matrix<T, M, N> operator -(const Matrix<T, M, N>& l, const Matrix<T, M, N>& r) { Matrix<T, M, N> n(l); n -= r; return n; }
  template<typename T, int M, int N> inline Matrix<T, M, N> operator ^(const Matrix<T, M, N>& l, const Matrix<T, M, N>& r) { Matrix<T, M, N> n(l); n ^= r; return n; }
  template<typename T, int M, int N> inline Matrix<T, M, N> operator /(const Matrix<T, M, N>& l, const Matrix<T, M, N>& r) { Matrix<T, M, N> n(l); n /= r; return n; }
  template<typename T, int M, int N> inline Matrix<T, M, N> operator +(const Matrix<T, M, N>& l, const T scalar) { Matrix<T, M, N> n(l); n += scalar; return n; }
  template<typename T, int M, int N> inline Matrix<T, M, N> operator -(const Matrix<T, M, N>& l, const T scalar) { Matrix<T, M, N> n(l); n -= scalar; return n; }
  template<typename T, int M, int N> inline Matrix<T, M, N> operator *(const Matrix<T, M, N>& l, const T scalar) { Matrix<T, M, N> n(l); n *= scalar; return n; }
  template<typename T, int M, int N> inline Matrix<T, M, N> operator /(const Matrix<T, M, N>& l, const T scalar) { Matrix<T, M, N> n(l); n /= scalar; return n; }
  template<typename T, int M, int N> inline Matrix<T, M, N> operator +(const T scalar, const Matrix<T, M, N>& r) { Matrix<T, M, N> n(r); n += scalar; return n; }
  template<typename T, int M, int N> inline Matrix<T, M, N> operator -(const T scalar, const Matrix<T, M, N>& r)
  {
    Matrix<T, M, N> x;
    T* u = (T*)x.v;
    T* v = (T*)r.v;
    for(int i = 0; i < M*N; ++i) u[i] = scalar - v[i];
    return x;
  }
  template<typename T, int M, int N> inline Matrix<T, M, N> operator *(const T scalar, const Matrix<T, M, N>& r) { Matrix<T, M, N> n(r); n *= scalar; return n; }
  template<typename T, int M, int N> inline Matrix<T, M, N> operator /(const T scalar, const Matrix<T, M, N>& r)
  {
    Matrix<T, M, N> x;
    T* u = (T*)x.v;
    T* v = (T*)r.v;
    for(int i = 0; i < M*N; ++i) u[i] = scalar / v[i];
    return x;
  }

  template<typename T, int M, int N> inline Matrix<T, M, N>& operator +=(Matrix<T, M, N>& l, const Matrix<T, M, N>& r) { for(int i = 0; i < M; ++i) for(int j = 0; j < N; ++j) l.v[i][j] += r.v[i][j]; return l; }
  template<typename T, int M, int N> inline Matrix<T, M, N>& operator -=(Matrix<T, M, N>& l, const Matrix<T, M, N>& r) { for(int i = 0; i < M; ++i) for(int j = 0; j < N; ++j) l.v[i][j] -= r.v[i][j]; return l; }
  template<typename T, int M, int N> inline Matrix<T, M, N>& operator ^=(Matrix<T, M, N>& l, const Matrix<T, M, N>& r) { for(int i = 0; i < M; ++i) for(int j = 0; j < N; ++j) l.v[i][j] *= r.v[i][j]; return l; }
  template<typename T, int M, int N> inline Matrix<T, M, N>& operator /=(Matrix<T, M, N>& l, const Matrix<T, M, N>& r) { for(int i = 0; i < M; ++i) for(int j = 0; j < N; ++j) l.v[i][j] /= r.v[i][j]; return l; }
  template<typename T, int M, int N> inline Matrix<T, M, N>& operator +=(Matrix<T, M, N>& l, const T scalar) { for(int i = 0; i < M; ++i) for(int j = 0; j < N; ++j) l.v[i][j] += scalar; return l; }
  template<typename T, int M, int N> inline Matrix<T, M, N>& operator -=(Matrix<T, M, N>& l, const T scalar) { for(int i = 0; i < M; ++i) for(int j = 0; j < N; ++j) l.v[i][j] -= scalar; return l; }
  template<typename T, int M, int N> inline Matrix<T, M, N>& operator *=(Matrix<T, M, N>& l, const T scalar) { for(int i = 0; i < M; ++i) for(int j = 0; j < N; ++j) l.v[i][j] *= scalar; return l; }
  template<typename T, int M, int N> inline Matrix<T, M, N>& operator /=(Matrix<T, M, N>& l, const T scalar) { for(int i = 0; i < M; ++i) for(int j = 0; j < N; ++j) l.v[i][j] /= scalar; return l; }

  template<typename T, int M, int N> inline bool operator ==(const Matrix<T, M, N>& l, const Matrix<T, M, N>& r) { for(int i = 0; i < M; ++i) for(int j = 0; j < N; ++j) if(l.v[i][j] != r.v[i][j]) return false; return true; }
  template<typename T, int M, int N> inline bool operator !=(const Matrix<T, M, N>& l, const Matrix<T, M, N>& r) { return !operator==(l, r); }
  template<typename T, int M, int N> inline bool operator ==(const Matrix<T, M, N>& l, const T scalar) { for(int i = 0; i < M; ++i) for(int j = 0; j < N; ++j) if(l.v[i][j] != scalar) return false; return true; }
  template<typename T, int M, int N> inline bool operator !=(const Matrix<T, M, N>& l, const T scalar) { return !operator==(l, scalar); }

  template<typename T, int M, int N> inline Matrix<T, M, N> operator -(const Matrix<T, M, N>& l) { return l*((T)-1); }
  template<typename T, int M, int N> inline Matrix<T, M, N> operator ~(const Matrix<T, M, N>& l) { Matrix<T, M, N> m; return l.Transpose(m.v); return m; }

  template<typename T, int M, int N>
  inline const Vector<T, N> operator *(const Vector<T, M>& v, const Matrix<T, M, N>& m) // Does v * M and assumes v is a row vector
  {
    Vector<T, N> out;
    MatrixMultiply<T, 1, M, N>(v.v_row, m.v, out.v_row);
    return out;
  }
  template<typename T, int M, int N>
  inline const Vector<T, M> operator *(const Matrix<T, M, N>& m, const Vector<T, N>& v) // Does M * v and assumes v is a column vector
  {
    Vector<T, M> out;
    MatrixMultiply<T, M, N, 1>(m.v, v.v_column, out.v_column);
    return out;
  }
  template<typename T, int N>
  inline Matrix<T, N, N>& operator *=(Vector<T, N>& l, const Matrix<T, N, N>& r) // Does v *= M and assumes v is a row vector
  {
    MatrixMultiply<T, 1, N, N>(l.v_row, r.v, l.v_row);
    return l;
  }
  template<typename T, int M, int N, int P>
  inline const Matrix<T, M, P> operator *(const Matrix<T, M, N>& l, const Matrix<T, N, P>& r) // Does M * M
  {
    Matrix<T, M, P> out;
    MatrixMultiply<T, M, N, P>(l.v, r.v, out.v);
    return out;
  }
  template<typename T, int M, int N>
  inline Matrix<T, M, N>& operator *=(Matrix<T, M, N>& l, const Matrix<T, N, N>& r) // Does M *= M
  {
    MatrixMultiply<T, M, N, N>(l.v, r.v, l.v);
    return l;
  }

  template<> inline Vector<float, 4>& operator +=<float, float, 4>(Vector<float, 4>& l, const Vector<float, 4>& r) { (sseVec(l.v) + sseVec(r.v)).Set(l.v); return l; }
  template<> inline Vector<float, 4>& operator -=<float, float, 4>(Vector<float, 4>& l, const Vector<float, 4>& r) { (sseVec(l.v) - sseVec(r.v)).Set(l.v); return l; }
  template<> inline Vector<float, 4>& operator *=<float, float, 4>(Vector<float, 4>& l, const Vector<float, 4>& r) { (sseVec(l.v) * sseVec(r.v)).Set(l.v); return l; }
  template<> inline Vector<float, 4>& operator /=<float, float, 4>(Vector<float, 4>& l, const Vector<float, 4>& r) { (sseVec(l.v) / sseVec(r.v)).Set(l.v); return l; }
  template<> inline Vector<float, 4>& operator +=<float, 4>(Vector<float, 4>& l, const float r) { (sseVec(l.v) + sseVec(r, r, r, r)).Set(l.v); return l; }
  template<> inline Vector<float, 4>& operator -=<float, 4>(Vector<float, 4>& l, const float r) { (sseVec(l.v) - sseVec(r, r, r, r)).Set(l.v); return l; }
  template<> inline Vector<float, 4>& operator *=<float, 4>(Vector<float, 4>& l, const float r) { (sseVec(l.v) * sseVec(r, r, r, r)).Set(l.v); return l; }
  template<> inline Vector<float, 4>& operator /=<float, 4>(Vector<float, 4>& l, const float r) { (sseVec(l.v) / sseVec(r, r, r, r)).Set(l.v); return l; }

  // This implements all possible B-spline functions using a given matrix m, optimized for floats
  inline float GenericBSpline(float t, const float(&p)[4], const float(&m)[4][4])
  {
    float a[4] = { t*t*t, t*t, t, 1 };
    sseVec r = MatrixMultiply1x4<float>(a, m);
    r *= sseVec(p);
    sseVec r2 = r;
    sseVec::Shuffle<0xB1>(r2);
    r += r2;
    sseVec::Shuffle<0x1B>(r2);
    return BUN_SSE_SS_F32(r + r2);
  }
}


#endif
