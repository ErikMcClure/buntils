// Copyright ©2014 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_VECTOR_H__
#define __BSS_VECTOR_H__

#include "bss_util.h"
#include "bss_sse.h"
#include <initializer_list>

namespace bss_util {
  // Find the dot product of two n-dimensional vectors
  template<typename T, int N>
  inline static T BSS_FASTCALL NVector_Dot(const T(&l)[N], const T(&r)[N])
  {
    T ret=0;
    for(int i=0; i<N; ++i)
      ret+=(l[i]*r[i]);
    return ret;
  }

  // Get the squared distance between two n-dimensional vectors
  template<typename T, int N>
  inline static T BSS_FASTCALL NVector_DistanceSq(const T(&l)[N], const T(&r)[N])
  {
    T tp = r[0]-l[0];
    T ret = tp*tp;
    for(int i = 1; i < N; ++i)
    {
      tp = r[i]-l[i];
      ret += tp*tp;
    }
    return ret;
  }
  template<typename T, int N>
  BSS_FORCEINLINE static T BSS_FASTCALL NVector_Distance(const T(&l)[N], const T(&r)[N]) { return FastSqrt<T>(NVector_DistanceSq<T, N>(l, r)); }

  template<typename T, int N>
  BSS_FORCEINLINE static void NVector_Normalize(const T(&v)[N], T(&out)[N])
  {
    T invlength = 1/FastSqrt<T>(NVector_Dot<T, N>(v, v));
    assert(invlength != 0);
    for(int i = 0; i < N; ++i)
      out[i] = v[i]*invlength;
  }

  template<typename T>
  BSS_FORCEINLINE static T NVector_AbsCall(const T v)
  {
      return abs((std::conditional<std::is_integral<T>::value, std::make_signed<std::conditional<std::is_integral<T>::value, T, int>::type>::type, T>::type)v);
  }

  template<typename T, int N>
  BSS_FORCEINLINE static void NVector_Abs(const T(&v)[N], T(&out)[N])
  {
    for(int i = 0; i < N; ++i)
      out[i] = NVector_AbsCall(v[i]);
  }

  // Find the area of an n-dimensional triangle using Heron's formula
  template<typename T, int N>
  inline static T NTriangleArea(const T(&x1)[N], const T(&x2)[N], const T(&x3)[N])
  {
    T a = NVectDist(x1, x2);
    T b = NVectDist(x1, x3);
    T c = NVectDist(x2, x3);
    T s = (a+b+c)/((T)2);
    return FastSqrt<T>(s*(s-a)*(s-b)*(s-c));
  }

  // Applies an operator to two vectors
  /*template<typename T, int N, T(BSS_FASTCALL *F)(T, T), sseVecT<T>(BSS_FASTCALL *sseF)(const sseVecT<T>&, const sseVecT<T>&)>
  BSS_FORCEINLINE static void BSS_FASTCALL NVectOp(const T(&x1)[N], const T(&x2)[N], T(&out)[N])
  {
  assert(((size_t)x1)%16==0);
  assert(((size_t)x2)%16==0);
  assert(((size_t)out)%16==0);
  int low=(N/4)*4; // Gets number of SSE iterations we can do
  int i;
  for(i=0; i < low; i+=4)
  BSS_SSE_STORE_APS(out+i, sseF(sseVecT<T>(BSS_SSE_LOAD_APS(x1+i)), sseVecT<T>(BSS_SSE_LOAD_APS(x2+i))));
  for(; i<N; ++i)
  out[i]=F(x1[i], x2[i]);
  }

  // Applies an operator to a vector and a scalar
  template<typename T, int N, T(BSS_FASTCALL *F)(T, T), sseVecT<T>(BSS_FASTCALL *sseF)(const sseVecT<T>&, const sseVecT<T>&)>
  BSS_FORCEINLINE static void BSS_FASTCALL NVectOp(const T(&x1)[N], T x2, T(&out)[N])
  {
  assert(((size_t)x1)%16==0);
  assert(((size_t)out)%16==0);
  int low=(N/4)*4; // Gets number of SSE iterations we can do
  int i;
  for(i=0; i < low; i+=4)
  BSS_SSE_STORE_APS(out+i, sseF(sseVecT<T>(BSS_SSE_LOAD_APS(x1+i)), sseVecT<T>(x2)));
  for(; i<N; ++i)
  out[i]=F(x1[i], x2);
  }

  template<typename T, typename R> BSS_FORCEINLINE static R BSS_FASTCALL NVectFAdd(T a, T b) { return a+b; }
  template<typename T, typename R> BSS_FORCEINLINE static R BSS_FASTCALL NVectFSub(T a, T b) { return a-b; }
  template<typename T, typename R> BSS_FORCEINLINE static R BSS_FASTCALL NVectFMul(T a, T b) { return a*b; }
  template<typename T, typename R> BSS_FORCEINLINE static R BSS_FASTCALL NVectFDiv(T a, T b) { return a/b; }
  template<typename T, int N> BSS_FORCEINLINE static void BSS_FASTCALL NVectAdd(const T(&x1)[N], const T(&x2)[N], T(&out)[N])
  { return NVectOp<T, N, NVectFAdd<T, T>, NVectFAdd<const sseVecT<T>&, sseVecT<T>>>(x1, x2, out); }
  template<typename T, int N> BSS_FORCEINLINE static void BSS_FASTCALL NVectAdd(const T(&x1)[N], T x2, T(&out)[N])
  { return NVectOp<T, N, NVectFAdd<T, T>, NVectFAdd<const sseVecT<T>&, sseVecT<T>>>(x1, x2, out); }
  template<typename T, int N> BSS_FORCEINLINE static void BSS_FASTCALL NVectSub(const T(&x1)[N], const T(&x2)[N], T(&out)[N])
  { return NVectOp<T, N, NVectFSub<T, T>, NVectFSub<const sseVecT<T>&, sseVecT<T>>>(x1, x2, out); }
  template<typename T, int N> BSS_FORCEINLINE static void BSS_FASTCALL NVectSub(const T(&x1)[N], T x2, T(&out)[N])
  { return NVectOp<T, N, NVectFSub<T, T>, NVectFSub<const sseVecT<T>&, sseVecT<T>>>(x1, x2, out); }
  template<typename T, int N> BSS_FORCEINLINE static void BSS_FASTCALL NVectMul(const T(&x1)[N], const T(&x2)[N], T(&out)[N])
  { return NVectOp<T, N, NVectFMul<T, T>, NVectFMul<const sseVecT<T>&, sseVecT<T>>>(x1, x2, out); }
  template<typename T, int N> BSS_FORCEINLINE static void BSS_FASTCALL NVectMul(const T(&x1)[N], T x2, T(&out)[N])
  { return NVectOp<T, N, NVectFMul<T, T>, NVectFMul<const sseVecT<T>&, sseVecT<T>>>(x1, x2, out); }
  template<typename T, int N> BSS_FORCEINLINE static void BSS_FASTCALL NVectDiv(const T(&x1)[N], const T(&x2)[N], T(&out)[N])
  { return NVectOp<T, N, NVectFDiv<T, T>, NVectFDiv<const sseVecT<T>&, sseVecT<T>>>(x1, x2, out); }
  template<typename T, int N> BSS_FORCEINLINE static void BSS_FASTCALL NVectDiv(const T(&x1)[N], T x2, T(&out)[N])
  { return NVectOp<T, N, NVectFDiv<T, T>, NVectFDiv<const sseVecT<T>&, sseVecT<T>>>(x1, x2, out); }*/

  template<typename T, int M, int N, bool B=false>
  class SetSubMatrix
  {
    static BSS_FORCEINLINE void M2x2(T(&out)[N][M], T m11, T m12, T m21, T m22)
    {
      static_assert(N>=2 && M>=2, "The NxM matrix must be at least 2x2");
      out[0][0] = m11; out[0][1] = m12;
      out[1][0] = m21; out[1][1] = m22;
    }
    static BSS_FORCEINLINE void M3x3(T(&out)[N][M], T m11, T m12, T m13, T m21, T m22, T m23, T m31, T m32, T m33)
    {
      static_assert(N>=3 && M>=3, "The NxM matrix must be at least 3x3");
      out[0][0] = m11; out[0][1] = m12; out[0][2] = m13;
      out[1][0] = m21; out[1][1] = m22; out[1][2] = m23;
      out[2][0] = m31; out[2][1] = m32; out[2][2] = m33;
    }
    static BSS_FORCEINLINE void M4x4(T(&out)[N][M], T m11, T m12, T m13, T m14, T m21, T m22, T m23, T m24, T m31, T m32, T m33, T m34, T m41, T m42, T m43, T m44)
    {
      static_assert(N>=4 && M>=4, "The NxM matrix must be at least 4x4");
      out[0][0] = m11; out[0][1] = m12; out[0][2] = m13; out[0][3] = m41;
      out[1][0] = m21; out[1][1] = m22; out[1][2] = m23; out[1][3] = m42;
      out[2][0] = m31; out[2][1] = m32; out[2][2] = m33; out[2][3] = m43;
      out[2][0] = m41; out[2][1] = m42; out[2][2] = m43; out[3][3] = m44;
    }
  };

  template<typename T, int M, int N>
  class SetSubMatrix<T, M, N, true>
  {
    static BSS_FORCEINLINE void M2x2(T(&out)[N][M], T m11, T m12, T m21, T m22) { 
      SetSubMatrix<T, M, N, false>::M2x2(out, m11, m21, m12, m22);
    }
    static BSS_FORCEINLINE void M3x3(T(&out)[N][M], T m11, T m12, T m13, T m21, T m22, T m23, T m31, T m32, T m33) {
      SetSubMatrix<T, M, N, false>::M3x3(out, m11, m21, m31, m12, m22, m32, m31, m32, m33);
    }
    static BSS_FORCEINLINE void M4x4(T(&out)[N][M], T m11, T m12, T m13, T m14, T m21, T m22, T m23, T m24, T m31, T m32, T m33, T m34, T m41, T m42, T m43, T m44) {
      SetSubMatrix<T, M, N, false>::M4x4(out, m11, m21, m31, m41, m12, m22, m32, m42, m13, m23, m33, m43, m14, m24, m34, m44);
    }
  };

  template<typename T>
  BSS_FORCEINLINE void MatrixInvert4x4(T* mat, T* dest)
  {
    T tmp[12]; /* temp array for pairs */
    T src[16]; /* array of transpose source matrix */
    T det;  /* determinant */
    /* transpose matrix */
    for(int i = 0; i < 4; i++) {
      src[i] = mat[i*4];
      src[i + 4] = mat[i*4+ 1];
      src[i + 8] = mat[i*4+ 2];
      src[i + 12] = mat[i*4+ 3];
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
    dst[0] = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7];
    dst[0] -= tmp[1]*src[5] + tmp[2]*src[6] + tmp[5]*src[7];
    dst[1] = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7];
    dst[1] -= tmp[0]*src[4] + tmp[7]*src[6] + tmp[8]*src[7];
    dst[2] = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7];
    dst[2] -= tmp[3]*src[4] + tmp[6]*src[5] + tmp[11]*src[7];
    dst[3] = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6];
    dst[3] -= tmp[4]*src[4] + tmp[9]*src[5] + tmp[10]*src[6];
    dst[4] = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3];
    dst[4] -= tmp[0]*src[1] + tmp[3]*src[2] + tmp[4]*src[3];
    dst[5] = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3];
    dst[5] -= tmp[1]*src[0] + tmp[6]*src[2] + tmp[9]*src[3];
    dst[6] = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3];
    dst[6] -= tmp[2]*src[0] + tmp[7]*src[1] + tmp[10]*src[3];
    dst[7] = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2];
    dst[7] -= tmp[5]*src[0] + tmp[8]*src[1] + tmp[11]*src[2];
    /* calculate pairs for second 8 elements (cofactors) */
    tmp[0] = src[2]*src[7];
    tmp[1] = src[3]*src[6];
    tmp[2] = src[1]*src[7];
    tmp[3] = src[3]*src[5];
    tmp[4] = src[1]*src[6];
    tmp[5] = src[2]*src[5];
    tmp[6] = src[0]*src[7];
    tmp[7] = src[3]*src[4];
    tmp[8] = src[0]*src[6];
    tmp[9] = src[2]*src[4];
    tmp[10] = src[0]*src[5];
    tmp[11] = src[1]*src[4];
    /* calculate second 8 elements (cofactors) */
    dst[8] = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15];
    dst[8] -= tmp[1]*src[13] + tmp[2]*src[14] + tmp[5]*src[15];
    dst[9] = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15];
    dst[9] -= tmp[0]*src[12] + tmp[7]*src[14] + tmp[8]*src[15];
    dst[10] = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15];
    dst[10]-= tmp[3]*src[12] + tmp[6]*src[13] + tmp[11]*src[15];
    dst[11] = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14];
    dst[11]-= tmp[4]*src[12] + tmp[9]*src[13] + tmp[10]*src[14];
    dst[12] = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9];
    dst[12]-= tmp[4]*src[11] + tmp[0]*src[9] + tmp[3]*src[10];
    dst[13] = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10];
    dst[13]-= tmp[6]*src[10] + tmp[9]*src[11] + tmp[1]*src[8];
    dst[14] = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8];
    dst[14]-= tmp[10]*src[11] + tmp[2]*src[8] + tmp[7]*src[9];
    dst[15] = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9];
    dst[15]-= tmp[8]*src[9] + tmp[11]*src[10] + tmp[5]*src[8];
    /* calculate determinant */
    det=src[0]*dst[0]+src[1]*dst[1]+src[2]*dst[2]+src[3]*dst[3];
    /* calculate matrix inverse */
    det = 1/det;
    for(int j = 0; j < 16; j++)
      dst[j] *= det;
  }

  // Standard 4x4 Matrix inversion using SSE, adapted from intel's implementation.
  template<>
  BSS_FORCEINLINE void MatrixInvert4x4<float>(float* src, float* dest)
  {
    __m128 minor0, minor1, minor2, minor3;
    __m128 row0, row1, row2, row3;
    __m128 det, tmp1;
    tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(src)), (__m64*)(src+ 4));
    row1 = _mm_loadh_pi(_mm_loadl_pi(row1, (__m64*)(src+8)), (__m64*)(src+12));
    row0 = _mm_shuffle_ps(tmp1, row1, 0x88);
    row1 = _mm_shuffle_ps(row1, tmp1, 0xDD);
    tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(src+ 2)), (__m64*)(src+ 6));
    row3 = _mm_loadh_pi(_mm_loadl_pi(row3, (__m64*)(src+10)), (__m64*)(src+14));
    row2 = _mm_shuffle_ps(tmp1, row3, 0x88);
    row3 = _mm_shuffle_ps(row3, tmp1, 0xDD);
    tmp1 = _mm_mul_ps(row2, row3);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
    minor0 = _mm_mul_ps(row1, tmp1);
    minor1 = _mm_mul_ps(row0, tmp1);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
    minor0 = _mm_sub_ps(_mm_mul_ps(row1, tmp1), minor0);
    minor1 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor1);
    minor1 = _mm_shuffle_ps(minor1, minor1, 0x4E);
    tmp1 = _mm_mul_ps(row1, row2);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
    minor0 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor0);
    minor3 = _mm_mul_ps(row0, tmp1);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
    minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row3, tmp1));
    minor3 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor3);
    minor3 = _mm_shuffle_ps(minor3, minor3, 0x4E);
    tmp1 = _mm_mul_ps(_mm_shuffle_ps(row1, row1, 0x4E), row3);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
    row2 = _mm_shuffle_ps(row2, row2, 0x4E);
    minor0 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor0);
    minor2 = _mm_mul_ps(row0, tmp1);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
    minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row2, tmp1));
    minor2 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor2);
    minor2 = _mm_shuffle_ps(minor2, minor2, 0x4E);
    tmp1 = _mm_mul_ps(row0, row1);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
    minor2 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor2);
    minor3 = _mm_sub_ps(_mm_mul_ps(row2, tmp1), minor3);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
    minor2 = _mm_sub_ps(_mm_mul_ps(row3, tmp1), minor2);
    minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row2, tmp1));
    tmp1 = _mm_mul_ps(row0, row3);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
    minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row2, tmp1));
    minor2 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor2);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
    minor1 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor1);
    minor2 = _mm_sub_ps(minor2, _mm_mul_ps(row1, tmp1));
    tmp1 = _mm_mul_ps(row0, row2);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
    minor1 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor1);
    minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row1, tmp1));
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
    minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row3, tmp1));
    minor3 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor3);
    det = _mm_mul_ps(row0, minor0);
    det = _mm_add_ps(_mm_shuffle_ps(det, det, 0x4E), det);
    det = _mm_add_ss(_mm_shuffle_ps(det, det, 0xB1), det);
    tmp1 = _mm_rcp_ss(det);
    det = _mm_sub_ss(_mm_add_ss(tmp1, tmp1), _mm_mul_ss(det, _mm_mul_ss(tmp1, tmp1)));
    det = _mm_shuffle_ps(det, det, 0x00);
    minor0 = _mm_mul_ps(det, minor0);
    _mm_storel_pi((__m64*)(src), minor0);
    _mm_storeh_pi((__m64*)(src+2), minor0);
    minor1 = _mm_mul_ps(det, minor1);
    _mm_storel_pi((__m64*)(src+4), minor1);
    _mm_storeh_pi((__m64*)(src+6), minor1);
    minor2 = _mm_mul_ps(det, minor2);
    _mm_storel_pi((__m64*)(src+ 8), minor2);
    _mm_storeh_pi((__m64*)(src+10), minor2);
    minor3 = _mm_mul_ps(det, minor3);
    _mm_storel_pi((__m64*)(src+12), minor3);
    _mm_storeh_pi((__m64*)(src+14), minor3);
  }

  // Multiply an MxN matrix with an NxP matrix to make an MxP matrix.
  template<typename T, int M, int N, int P>
  class __MatrixMultiply
  {
    static BSS_FORCEINLINE void BSS_FASTCALL MM(const T(&l)[M][N], const T(&r)[N][P], T(&out)[M][P])
    {
      for(int i = 0; i < M; ++i)
      {
        for(int j = 0; j < P; ++j)
        {
          out[i][j] = l[i][0]*r[0][j];
          for(int k = 1; k < N; ++k)
            out[i][j] += l[i][k]*r[k][j];
        }
      }
    }
  };

  // Standard 4x4 matrix multiplication using SSE2. Intended for row-major matrices, so you'll end up with r*l instead of l*r if your matrices are column major instead.
  template<>
  class __MatrixMultiply<float, 4, 4, 4>
  {
    static BSS_FORCEINLINE void BSS_FASTCALL MM(const float(&l)[4][4], const float(&r)[4][4], float(&out)[4][4])
    {
      sseVec a(r[0]);
      sseVec b(r[1]);
      sseVec c(r[2]);
      sseVec d(r[3]);

      (a*sseVec(l[0][0]))+(b*sseVec(l[0][1]))+(c*sseVec(l[0][2]))+(d*sseVec(l[0][3])) >> out[0];
      (a*sseVec(l[1][0]))+(b*sseVec(l[1][1]))+(c*sseVec(l[1][2]))+(d*sseVec(l[1][3])) >> out[1];
      (a*sseVec(l[2][0]))+(b*sseVec(l[2][1]))+(c*sseVec(l[2][2]))+(d*sseVec(l[2][3])) >> out[2];
      (a*sseVec(l[3][0]))+(b*sseVec(l[3][1]))+(c*sseVec(l[3][2]))+(d*sseVec(l[3][3])) >> out[3];
    }
  };

  // It turns out you can efficiently do SSE optimization when multiplying any Mx4 matrix with a 4x4 matrix to yield an Mx4 out matrix.
  template<int M>
  class __MatrixMultiply<float, M, 4, 4>
  {
    static BSS_FORCEINLINE void BSS_FASTCALL MM(const float(&l)[M][4], const float(&r)[4][4], float(&out)[M][4])
    {
      sseVec a(r[0]);
      sseVec b(r[1]);
      sseVec c(r[2]);
      sseVec d(r[3]);

      for(int i = 0; i < M; ++i)
        (a*sseVec(l[i][0]))+(b*sseVec(l[i][1]))+(c*sseVec(l[i][2]))+(d*sseVec(l[i][3])) >> out[i];
    }
  };

  template<typename T, int M>
  class __MatrixMultiply<T, M, 4, 4>
  {
    static BSS_FORCEINLINE void BSS_FASTCALL MM(const T(&l)[M][4], const T(&r)[4][4], T(&out)[M][4])
    {
      sseVecT<T> a(r[0]);
      sseVecT<T> b(r[1]);
      sseVecT<T> c(r[2]);
      sseVecT<T> d(r[3]);

      for(int i = 0; i < M; ++i)
        (a*sseVecT<T>(l[i][0]))+(b*sseVecT<T>(l[i][1]))+(c*sseVecT<T>(l[i][2]))+(d*sseVecT<T>(l[i][3])) >> out[i];
    }
  };

  template<typename T>
  class __MatrixMultiply<T, 1, 4, 4>
  {
    static BSS_FORCEINLINE void BSS_FASTCALL MM(const T(&l)[1][4], const T(&r)[4][4], T(&out)[1][4])
    {
      (sseVecT<T>(r[0])*sseVecT<T>(l[0][0]))+(sseVecT<T>(r[1])*sseVecT<T>(l[0][1]))+(sseVecT<T>(r[2])*sseVecT<T>(l[0][2]))+(sseVecT<T>(r[3])*sseVecT<T>(l[0][3])) >> out[0];
    }
  };

  // Multiply a 1x4 vector on the left with a 4x4 matrix on the right, resulting in a 1x4 vector held in an sseVec.
  template<typename T>
  BSS_FORCEINLINE sseVecT<T> BSS_FASTCALL MatrixMultiply1x4(const T(&l)[4], const T(&r)[4][4])
  {
    return (sseVecT<T>(r[0])*sseVecT<T>(l[0]))+(sseVecT<T>(r[1])*sseVecT<T>(l[1]))+(sseVecT<T>(r[2])*sseVecT<T>(l[2]))+(sseVecT<T>(r[3])*sseVecT<T>(l[3]));
  }

  template<typename T, int M, int N, int P>
  BSS_FORCEINLINE static void BSS_FASTCALL MatrixMultiply(const T(&l)[M][N], const T(&r)[N][P], T(&out)[M][P])
  {
    __MatrixMultiply<T, M, N, P>::MM(l, r, out);
  }

  template<typename T, int N>
  static BSS_FORCEINLINE void BSS_FASTCALL FromQuaternion(T(&q)[4], T(&out)[N][N])
  {
    T w=q[3], x=q[0], y=q[1], z=q[2];
    T xx = x*x, xy = x*y, xz = x*z, xw = x*w;
    T yy = y*y, yz = y*z, yw = y*w;
    T zz = z*z, zw = z*w;

    SetSubMatrix<T, 3, 3>::MM(out,
      1 - 2 * (yy + zz), 2 * (xy - zw), 2 * (xz + yw),
      2 * (xy + zw), 1 - 2 * (xx + zz), 2 * (yz - xw),
      2 * (xz - yw), 2 * (yz + xw), 1 - 2 * (xx + yy));
  }

  template<>
  static BSS_FORCEINLINE void BSS_FASTCALL FromQuaternion<float, 4>(float(&q)[4], float(&out)[4][4])
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

    y*yxw + z*zwx*n010 >> out[0];
    x*yxw + z*wzy*n001 >> out[1];
    x*zwx + y*wzy*n100 >> out[2];
    sseVec::ZeroVector() >> out[3];
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
  struct Vector
  {
    template<typename U>
    inline Vector(const Vector<U, N>& copy) { for(int i = 0; i < N; ++i) v[i] = (T)copy.v[i]; }
    inline Vector(T scalar) { for(int i = 0; i < N; ++i) v[i] = scalar; }
    inline Vector(const T(&e)[N]) { for(int i = 0; i < N; ++i) v[i] = e[i]; }
    inline Vector(std::initializer_list<T> e) { int k = 0; for(const T* i = e.begin(); i != eend() && k < N; ++i) v[k++] = *i; }
    inline Vector() { }
    inline T Length() const { return FastSqrt<T>(Dot(*this)); }
    inline Vector<T, N> Normalize() const { Vector<T, N> ret(*this); NVector_Normalize(v, ret.v); return ret; }
    inline Vector<T, N> Abs() const { Vector<T, N> ret(*this); NVector_Abs(v, ret.v); return ret; }
    inline T BSS_FASTCALL Dot(const Vector<T, N>& r) const { return NVector_Dot<T, N>(v, r.v); }
    inline T BSS_FASTCALL Distance(const Vector<T, N>& r) const { return FastSqrt<T>(DistanceSq(r)); }
    inline T BSS_FASTCALL DistanceSq(const Vector<T, N>& r) const { return NVector_DistanceSq<T, N>(v, r.v); }
    template<int K> inline void Outer(const Vector<T, K>& r, T(&out)[N][K]) const { MatrixMultiply<T, N, 1, K>(v_column, r.v_row, out); }

    inline Vector<T, N>& BSS_FASTCALL operator=(const Vector<T, N>& r) { for(int i = 0; i < N; ++i) v[i] = r.v[i]; return *this; }
    template<typename U> inline Vector<T, N>& BSS_FASTCALL operator=(const Vector<U, N>& r) { for(int i = 0; i < N; ++i) v[i] = (T)r.v[i]; return *this; }

    union {
      T v[N];
      T v_column[N][1];
      T v_row[1][N];
    };
  };

  template<typename T>
  struct Vector<T, 2>
  {
    template<typename U>
    inline Vector(const Vector<U, 2>& copy) : x((U)copy.v[0]), y((U)copy.v[1]) { }
    inline Vector(T scalar) : x(scalar), y(scalar) { }
    inline Vector(const T(&e)[2]) : x(e[0]), y(e[1]) { }
    inline Vector(std::initializer_list<T> e) { int k = 0; for(const T* i = e.begin(); i != e.end() && k < 2; ++i) v[k++] = *i; }
    inline Vector(T X, T Y) : x(X), y(Y) { }
    inline Vector() { }
    inline T Length() const { return FastSqrt<T>(Dot(*this)); }
    inline Vector<T, 2> Normalize() const { T l = Length(); return Vector<T, 2>(x/l, y/l); }
    inline Vector<T, 2> Abs() const { return Vector<T, 2>(NVector_AbsCall(x), NVector_AbsCall(y)); }
    inline T BSS_FASTCALL Dot(const Vector<T, 2>& r) const { return DotProduct(r.x, r.y, x, y); }
    inline T BSS_FASTCALL Distance(const Vector<T, 2>& r) const { return FastSqrt<T>(DistanceSq(r)); }
    inline T BSS_FASTCALL DistanceSq(const Vector<T, 2>& r) const { return bss_util::distsqr<T>(r.x, r.y, x, y); }
    inline Vector<T, 2> BSS_FASTCALL Rotate(T R, const Vector<T, 2>& center) const { return Rotate(R, center.x, center.y); }
    inline Vector<T, 2> BSS_FASTCALL Rotate(T R, T X, T Y) const { T tx=x; T ty=y; RotatePoint(tx, ty, R, X, Y); return Vector<T, 2>(tx, ty); }
    inline float BSS_FASTCALL CrossProduct(const Vector<T, 2>& r) const { return CrossProduct(r.x, r.y, x, y); }
    inline float BSS_FASTCALL CrossProduct(T X, T Y) const { return CrossProduct(X, Y, x, y); }
    template<int K> inline void Outer(const Vector<T, K>& r, T(&out)[2][K]) const { MatrixMultiply<T, 2, 1, K>(v_column, r.v_row, out); }

    inline Vector<T, 2>& BSS_FASTCALL operator=(const Vector<T, 2>& r) { x = r.x; y = r.y; return *this; }
    template<typename U> inline Vector<T, 2>& BSS_FASTCALL operator=(const Vector<U, 2>& r) { x = (T)r.x; y = (T)r.y; return *this; }

    static BSS_FORCEINLINE void BSS_FASTCALL RotatePoint(T& x, T& y, T r, T cx, T cy) { T tx = x-cx; T ty = y-cy; T rcos = (T)cos(r); T rsin = (T)sin(r); x = (tx*rcos - ty*rsin)+cx; y = (ty*rcos + tx*rsin)+cy; }
    static BSS_FORCEINLINE T BSS_FASTCALL CrossProduct(T X, T Y, T x, T y) { return x*Y - X*y; }
    static BSS_FORCEINLINE T BSS_FASTCALL DotProduct(T X, T Y, T x, T y) { return X*x + Y*y; }
    
    inline Vector<T, 2> yx() const { return Vector<T, 2>(y, x); }

    union {
      T v[2];
      T v_column[2][1];
      T v_row[1][2];
      struct { T x, y; };
    };
  };

  template<typename T>
  struct Vector<T, 3>
  {
    template<typename U>
    inline Vector(const Vector<U, 3>& copy) : x((U)copy.v[0]), y((U)copy.v[1]), z((U)copy.v[2]) { }
    inline Vector(T scalar) : x(scalar), y(scalar), z(scalar){ }
    inline Vector(const T(&e)[3]) : x(e[0]), y(e[1]), z(e[2]) { }
    inline Vector(std::initializer_list<T> e) { int k = 0; for(const T* i = e.begin(); i != e.end() && k < 3; ++i) v[k++] = *i; }
    inline Vector(T X, T Y, T Z) : x(X), y(Y), z(Z) { }
    inline Vector() { }
    inline T Length() const { return FastSqrt((x*x)+(y*y)+(z*z)); }
    inline Vector<T, 3> Normalize() const { T l = Length(); return Vector<T, 3>(x/l, y/l, z/l); }
    inline Vector<T, 3> Abs() const { return Vector<T, 3>(NVector_AbsCall(x), NVector_AbsCall(y), NVector_AbsCall(z)); }
    inline T BSS_FASTCALL Dot(const Vector<T, 3>& r) const { return (x*r.x) + (y*r.y) + (z*r.z); }
    inline T BSS_FASTCALL Distance(const Vector<T, 3>& r) const { return FastSqrt<T>(DistanceSq(r)); }
    inline T BSS_FASTCALL DistanceSq(const Vector<T, 3>& r) const { T tz = (r.z - z); T ty = (r.y - y); T tx = (r.x - x); return (T)((tx*tx)+(ty*ty)+(tz*tz)); }
    inline Vector<T, 3> CrossProduct(const Vector<T, 3>& r) const { return CrossProduct(r.x, r.y, r.z); }
    inline Vector<T, 3> CrossProduct(T X, T Y, T Z) const { return Vector<T, 3>(y*Z - z*Y, z*X - x*Z, x*Y - X*y); }
    template<int K> inline void Outer(const Vector<T, K>& r, T(&out)[3][K]) const { MatrixMultiply<T, 3, 1, K>(v_column, r.v_row, out); }

    inline Vector<T, 3>& BSS_FASTCALL operator=(const Vector<T, 3>& r) { x = r.x; y = r.y; z = r.z; return *this; }
    template<typename U> inline Vector<T, 3>& BSS_FASTCALL operator=(const Vector<U, 3>& r) { x = (T)r.x; y = (T)r.y; z = (T)r.z; return *this; }

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
  struct Vector<T, 4>
  {
    template<typename U>
    inline Vector(const Vector<U, 4>& copy) : x((U)copy.v[0]), y((U)copy.v[1]), z((U)copy.v[2]), w((U)copy.v[3]) { }
    inline Vector(T scalar) : x(scalar), y(scalar), z(scalar), w(scalar) { }
    inline Vector(const T(&e)[4]) : x(e[0]), y(e[1]), z(e[2]), w(e[3]) { }
    inline Vector(std::initializer_list<T> e) { int k = 0; for(const T* i = e.begin(); i != e.end() && k < 4; ++i) v[k++] = *i; }
    inline Vector(T X, T Y, T Z, T W) : x(X), y(Y), z(Z), w(W) { }
    inline Vector() { }
    inline T Length() const { return FastSqrt<T>(Dot(*this)); }
    inline Vector<T, 4> Normalize() const { T l = Length(); return Vector<T, 4>(x/l, y/l, z/l, w/l); }
    inline Vector<T, 4> Abs() const { return Vector<T, 3>(NVector_AbsCall(x), NVector_AbsCall(y), NVector_AbsCall(z), NVector_AbsCall(w)); }
    inline T BSS_FASTCALL Dot(const Vector<T, 4>& r) const { return (x*r.x) + (y*r.y) + (z*r.z) + (w*r.w); }
    inline T BSS_FASTCALL Distance(const Vector<T, 4>& r) const { return FastSqrt<T>(DistanceSq(r)); }
    inline T BSS_FASTCALL DistanceSq(const Vector<T, 4>& r) const { T tz = (r.z - z); T ty = (r.y - y); T tx = (r.x - x); T tw = (r.w - w); return (T)((tx*tx)+(ty*ty)+(tz*tz)+(tw*tw)); }
    template<int K> inline void Outer(const Vector<T, K>& r, T(&out)[4][K]) const { MatrixMultiply<T, 4, 1, K>(v_column, r.v_row, out); }

    inline Vector<T, 4>& BSS_FASTCALL operator=(const Vector<T, 4>& r) { x = r.x; y = r.y; z = r.z; w = r.w; return *this; }
    template<typename U> inline Vector<T, 4>& BSS_FASTCALL operator=(const Vector<U, 4>& r) { x = (T)r.x; y = (T)r.y; z = (T)r.z; w = (T)r.w; return *this; }

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

    union {
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
  struct Matrix
  {
    template<typename U>
    inline Matrix(const Matrix<U, M, N>& copy) { T* p = v; U* cp = copy.v; for(int i = 0; i < M*N; ++i) p[i]=(T)cp[i]; }
    inline Matrix(std::initializer_list<T> v) { T* p = v; int k = 0; for(const T* i = v.begin(); i != v.end() && k < M*N; ++i) p[k++] = *i; }
    inline Matrix(const T(&m)[M][N]) { memcpy(v, m, sizeof(T)*M*N); }
    inline Matrix() { }
    inline Matrix<T, N, M> Transpose() const { return Transpose(v); }

    inline Matrix<T, M, N>& BSS_FASTCALL operator=(const Matrix<T, M, N>& r) { memcpy(v, r.v, sizeof(T)*M*N); return *this; }
    template<typename U> inline Matrix<T, M, N>& BSS_FASTCALL operator=(const Matrix<U, M, N>& r) { T* p = v; U* cp = r.v; for(int i = 0; i < M*N; ++i) p[i]=(T)cp[i]; return *this; }

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
        v[i][i] = 1;
    }
    static inline void Inverse();

    T v[M][N];
  };

  template<typename T>
  struct Matrix<T, 2, 2>
  {
    template<typename U>
    inline Matrix(const Matrix<U, 2, 2>& copy) : a(copy.a), b(copy.b), c(copy.c), d(copy.d) { }
    inline Matrix(std::initializer_list<T> v) { T* p = v; int k = 0; for(const T* i = v.begin(); i != v.end() && k < 2*2; ++i) p[k++] = *i; }
    inline Matrix(const T(&m)[2][2]) : a(m[0][0]), b(m[0][1]), c(m[1][0]), d(m[1][1]) { }
    inline Matrix() { }
    inline void Transpose(T(&out)[2][2]) const { SetSubMatrix<T, 2, 2, true>::M2x2(out, a, b, c, d); }
    inline T Determinant() const { return (a*d) - (b*c); }
    inline void Inverse(T(&out)[2][2]) const { T invd = ((T)1)/Determinant(); SetSubMatrix<T, 2, 2>::M2x2(out, d*invd, -b*invd, -c*invd, a*invd); }

    inline Matrix<T, 2, 2>& BSS_FASTCALL operator=(const Matrix<T, 2, 2>& r) { a = r.a; b = r.b; c = r.c; d = r.d; return *this; }
    template<typename U> inline Matrix<T, 2, 2>& BSS_FASTCALL operator=(const Matrix<U, 2, 2>& r) { a = (T)r.a; b = (T)r.b; c = (T)r.c; d = (T)r.d; return *this; }

    template<int M, int N, bool Tp>
    BSS_FORCEINLINE static void __Rotation(T r, T(&out)[2][2]) { T c = cos(r); T s = sin(r); SetSubMatrix<T, 2, 2, Tp>::M2x2(c, -s, s, c); }
    template<int M, int N>
    BSS_FORCEINLINE static void Rotation(T r, T(&out)[2][2]) { __Rotation<2, 2, false>(r, out); }
    template<int M, int N>
    BSS_FORCEINLINE static void Rotation_T(T r, T(&out)[2][2]) { __Rotation<2, 2, true>(r, out); }
    BSS_FORCEINLINE static void Diagonal(T x, T y, T(&out)[2][2]) { SetSubMatrix<T, 2, 2>::M2x2(out, x, 0, 0, y); }
    BSS_FORCEINLINE static void Diagonal(const Vector<T, 2> v, T(&out)[2][2]) { Diagonal(v.v[0], v.v[1], out); }
    BSS_FORCEINLINE static void Diagonal(const T(&v)[2], T(&out)[2][2]) { Diagonal(v[0], v[1], out); }

    BSS_FORCEINLINE static void Identity(Matrix<T, 2, 2>& n) { Identity(n.v); }
    BSS_FORCEINLINE static void Identity(T(&n)[2][2]) { Diagonal(1, 1, n); }

    union {
      T v[2][2];
      struct { T a, b, c, d; };
    };
  };

  template<typename T>
  struct Matrix<T, 3, 3>
  {
    template<typename U>
    inline Matrix(const Matrix<U, 3, 3>& copy) : a(copy.a), b(copy.b), c(copy.c), d(copy.d), e(copy.e), f(copy.f), g(copy.g), h(copy.h), i(copy.i) { }
    inline Matrix(std::initializer_list<T> v) { T* p = v; int k = 0; for(const T* k = v.begin(); k != v.end() && k < 3*3; ++k) p[k++] = *k; }
    inline Matrix(const T(&m)[3][3]) : a(m[0][0]), b(m[0][1]), c(m[0][2]), d(m[1][0]), e(m[1][1]), f(m[1][2]), g(m[2][0]), h(m[2][1]), i(m[2][2]) { }
    inline Matrix() { }
    inline void Transpose(T(&out)[3][3]) const { SetSubMatrix<T, 3, 3, true>::M3x3(out, a, b, c, d, e, f, g, h, i); }
    inline T Determinant() const { return a*(e*i - f*h) - b*(i*d - f*g) + c*(d*h - e*g); }
    inline void Inverse(T(&out)[3][3]) const
    {
      T invd = ((T)1)/Determinant();
      SetSubMatrix3x3(out,
        invd*(e*i - f*h), invd*(c*h - b*i), invd*(b*f - c*e),
        invd*(f*g - d*i), invd*(a*i - c*g), invd*(c*d - a*f),
        invd*(d*h - e*g), invd*(b*g - a*h), invd*(a*e - b*d));
    }

    inline Matrix<T, 3, 3>& BSS_FASTCALL operator=(const Matrix<T, 3, 3>& r) { a = r.a; b = r.b; c = r.c; d = r.d; e = r.e; f = r.f; g = r.g; h = r.h; i = r.i; return *this; }
    template<typename U> inline Matrix<T, 3, 3>& BSS_FASTCALL operator=(const Matrix<U, 3, 3>& r) { a = (T)r.a; b = (T)r.b; c = (T)r.c; d = (T)r.d; e = (T)r.e; f = (T)r.f; g = (T)r.g; h = (T)r.h; i = (T)r.i; return *this; }

    static BSS_FORCEINLINE void AffineRotation(T r, T(&out)[3][3]) { RotationZ(r); }
    static BSS_FORCEINLINE void AffineRotation_T(T r, T(&out)[3][3]) { RotationZ_T(r); }
    static BSS_FORCEINLINE void AffineScaling(T x, T y, T(&out)[3][3]) { Diagonal(x, y, 1); }
    template<bool Tp>
    static inline void __AffineTransform(T x, T y, T r, T cx, T cy, T(&out)[3][3])
    {
      T c = cos(r);
      T s = sin(r);
      SetSubMatrix<T, 3, 3, Tp>::M3x3(out, c, -s, x - c*cx + cx + cy*s, s, c, y - c*cy + cy - cx*s, 0, 0, 1);
    }
    static BSS_FORCEINLINE void AffineTransform(T x, T y, T r, T cx, T cy, T(&out)[3][3]) { __AffineTransform<false>(x, y, r, cx, cy, out); }
    static BSS_FORCEINLINE void AffineTransform_T(T x, T y, T r, T cx, T cy, T(&out)[3][3]) { __AffineTransform<true>(x, y, r, cx, cy, out); }
    static BSS_FORCEINLINE void Translation(T x, T y, T(&out)[3][3]) { SetSubMatrix<T, 3, 3>::M3x3(out, 1, 0, x, 0, 1, y, 0, 0, 1); }
    static BSS_FORCEINLINE void Translation_T(T x, T y, T(&out)[3][3]) { SetSubMatrix<T, 3, 3, true>::M3x3(out, 1, 0, x, 0, 1, y, 0, 0, 1); }
    template<bool Tp> static BSS_FORCEINLINE void __RotationX(T r, T(&out)[3][3]) { T c = cos(r); T s = sin(r); SetSubMatrix<T, 3, 3, Tp>::M3x3(out, 1, 0, 0, 0, c, -s, 0, s, c); }
    template<bool Tp> static BSS_FORCEINLINE void __RotationY(T r, T(&out)[3][3]) { T c = cos(r); T s = sin(r); SetSubMatrix<T, 3, 3, Tp>::M3x3(out, c, 0, s, 0, 1, 0, -s, 0, c); }
    template<bool Tp> static BSS_FORCEINLINE void __RotationZ(T r, T(&out)[3][3]) { T c = cos(r); T s = sin(r); SetSubMatrix<T, 3, 3, Tp>::M3x3(out, c, -s, 0, s, c, 0, 0, 0, 1); }
    static BSS_FORCEINLINE void RotationX(T r, T(&out)[3][3]) { __RotationX<false>(r, out); }
    static BSS_FORCEINLINE void RotationX_T(T r, T(&out)[3][3]) { __RotationX<true>(r, out); }
    static BSS_FORCEINLINE void RotationY(T r, T(&out)[3][3]) { __RotationY<false>(r, out); }
    static BSS_FORCEINLINE void RotationY_T(T r, T(&out)[3][3]) { __RotationY<true>(r, out); }
    static BSS_FORCEINLINE void RotationZ(T r, T(&out)[3][3]) { __RotationZ<false>(r, out); }
    static BSS_FORCEINLINE void RotationZ_T(T r, T(&out)[3][3]) { __RotationZ<true>(r, out); }
    static BSS_FORCEINLINE void FromQuaternion(T(&q)[4], T(&out)[3][3]) {  }
    static BSS_FORCEINLINE void Diagonal(T x, T y, T z, T(&out)[3][3]) { SetSubMatrix<T, 3, 3>::M3x3(out, x, 0, 0, 0, y, 0, 0, 0, z); }
    static BSS_FORCEINLINE void Diagonal(const Vector<T, 3> v, T(&out)[3][3]) { return Diagonal(v.v[0], v.v[1], v.v[2], out); }
    static BSS_FORCEINLINE void Diagonal(const T(&v)[3], T(&out)[3][3]) { return Diagonal(v[0], v[1], v[2], out); }

    static BSS_FORCEINLINE void Identity(Matrix<T, 3, 3>& m) { Identity(m.v); }
    static BSS_FORCEINLINE void Identity(T(&m)[3][3]) { Diagonal(1, 1, 1, m); }

    union {
      T v[3][3];
      struct { T a, b, c, d, e, f, g, h, i; };
    };
  };

  template<typename T>
  struct Matrix<T, 4, 4>
  {
    template<typename U>
    inline Matrix(const Matrix<U, 4, 4>& copy) { T* p = v; U* cp = copy.v; for(int u = 0; u < 4*4; ++u) p[u]=(U)cp[u]; }
    inline Matrix(std::initializer_list<T> v) { T* p = v; int k = 0; for(const T* u = v.begin(); u != v.end() && k < 4*4; ++u) p[k++] = *u; }
    inline Matrix(const T(&m)[4][4]) { memcpy(v, m, sizeof(T)*4*4); }
    inline Matrix() { }
    inline void Transpose(T(&out)[4][4]) const { SetSubMatrix<T, 4, 4, true>::M4x4(out, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p); }
    inline T Determinant() const { return 0; }
    inline void Inverse(T(&out)[4][4]) { MatrixInvert4x4<T>(v, out); }

    inline Matrix<T, 4, 4>& BSS_FASTCALL operator=(const Matrix<T, 4, 4>& r) { a = r.a; b = r.b; c = r.c; d = r.d; e = r.e; f = r.f; g = r.g; h = r.h; i = r.i; j = r.j; k = r.k; l = r.l; m = r.m; n = r.n; o = r.o; p = r.p; return *this; }
    template<typename U> inline Matrix<T, 4, 4>& BSS_FASTCALL operator=(const Matrix<U, 4, 4>& r) { a = (T)r.a; b = (T)r.b; c = (T)r.c; d = (T)r.d; e = (T)r.e; f = (T)r.f; g = (T)r.g; h = (T)r.h; i = (T)r.i; j = (T)r.j; k = (T)r.k; l = (T)r.l; m = (T)r.m; n = (T)r.n; o = (T)r.o; p = (T)r.p; return *this; }

    static inline Matrix<T, 4, 4> AffineScaling(T x, T y, T z) { return Diagonal(x, y, z, 1); }
    template<bool Tp>
    static void __AffineTransform(T x, T y, T z, T rz, T cx, T cy, T(&out)[4][4])
    {
      T c = cos(rz);
      T s = sin(rz);
      SetSubMatrix<T, 4, 4, Tp>::M4x4(out, c, -s, 0, x - c*cx + cx + cy*s, s, c, 0, y - c*cy + cy - cx*s, 0, 0, 1, z, 0, 0, 0, 1);
    }
    static BSS_FORCEINLINE void AffineTransform(T x, T y, T z, T rz, T cx, T cy, T(&out)[4][4]) { __AffineTransform<false>(x, y, z, rz, cx, cy, out); }
    static BSS_FORCEINLINE void AffineTransform_T(T x, T y, T z, T rz, T cx, T cy, T(&out)[4][4]) { __AffineTransform<true>(x, y, z, rz, cx, cy, out); }
    template<bool Tp> static BSS_FORCEINLINE void __Translation(T x, T y, T z, T(&out)[4][4]) { SetSubMatrix<T, 4, 4, Tp>::M4x4(1, 0, 0, x, 0, 1, 0, y, 0, 0, 1, z, 0, 0, 0, 1); }
    static BSS_FORCEINLINE void Translation(T x, T y, T z, T(&out)[4][4]) { __Translation<false>(x, y, z, out); }
    static BSS_FORCEINLINE void Translation_T(T x, T y, T z, T(&out)[4][4]) { __Translation<true>(x, y, z, out); }
    template<bool Tp> static BSS_FORCEINLINE void __AffineRotationX(T r, T(&out)[4][4]) { T c = cos(r); T s = sin(r); SetSubMatrix<T, 4, 4, Tp>::M4x4(1, 0, 0, 0, 0, c, -s, 0, 0, s, c, 0, 0, 0, 0, 1); }
    static BSS_FORCEINLINE void AffineRotationX(T r, T(&out)[4][4]) { __AffineRotationX<false>(r, out); }
    static BSS_FORCEINLINE void AffineRotationX_T(T r, T(&out)[4][4]) { __AffineRotationX<true>(r, out); }
    template<bool Tp> static BSS_FORCEINLINE void __AffineRotationY(T r, T(&out)[4][4]) { T c = cos(r); T s = sin(r); SetSubMatrix<T, 4, 4, Tp>::M4x4(c, 0, s, 0, 0, 1, 0, 0, -s, 0, c, 0, 0, 0, 0, 1); }
    static BSS_FORCEINLINE void AffineRotationY(T r, T(&out)[4][4]) { __AffineRotationY<false>(r, out); }
    static BSS_FORCEINLINE void AffineRotationY_T(T r, T(&out)[4][4]) { __AffineRotationY<true>(r, out); }
    template<bool Tp> static BSS_FORCEINLINE void __AffineRotationZ(T r, T(&out)[4][4]) { T c = cos(r); T s = sin(r); SetSubMatrix<T, 4, 4, Tp>::M4x4(c, -s, 0, 0, s, c, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1); }
    static BSS_FORCEINLINE void AffineRotationZ(T r, T(&out)[4][4]) { __AffineRotationZ<false>(r, out); }
    static BSS_FORCEINLINE void AffineRotationZ_T(T r, T(&out)[4][4]) { __AffineRotationZ<true>(r, out); }
    static BSS_FORCEINLINE void Diagonal(T x, T y, T z, T w, T(&out)[4][4]) { SetSubMatrix<T, 4, 4>::M4x4(x, 0, 0, 0, 0, y, 0, 0, 0, 0, z, 0, 0, 0, 0, w); }
    static BSS_FORCEINLINE void Diagonal(const Vector<T, 4> v, T(&out)[4][4]) { return Diagonal(v.v[0], v.v[1], v.v[2], v.v[3]); }
    static BSS_FORCEINLINE void Diagonal(const T(&v)[4], T(&out)[4][4]) { return Diagonal(v[0], v[1], v[2], v[3]); }

    static BSS_FORCEINLINE void Identity(Matrix<T, 4, 4>& m) { Identity(m.v); }
    static BSS_FORCEINLINE void Identity(T(&m)[4][4]) { Diagonal(1, 1, 1, 1, m); }

    union {
      T v[4][4];
      struct { T a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p; };
    };
  };

  template<typename T, int N> inline static const Vector<T, N> BSS_FASTCALL operator +(const Vector<T, N>& l, const Vector<T, N>& r) { return Vector<T, N>(l)+=r; }
  template<typename T, int N> inline static const Vector<T, N> BSS_FASTCALL operator -(const Vector<T, N>& l, const Vector<T, N>& r) { return Vector<T, N>(l)-=r; }
  template<typename T, int N> inline static const Vector<T, N> BSS_FASTCALL operator *(const Vector<T, N>& l, const Vector<T, N>& r) { return Vector<T, N>(l)*=r; }
  template<typename T, int N> inline static const Vector<T, N> BSS_FASTCALL operator /(const Vector<T, N>& l, const Vector<T, N>& r) { return Vector<T, N>(l)/=r; }
  template<typename T, int N> inline static const Vector<T, N> BSS_FASTCALL operator +(const Vector<T, N>& l, const T scalar) { return Vector<T, N>(l)+=scalar; }
  template<typename T, int N> inline static const Vector<T, N> BSS_FASTCALL operator -(const Vector<T, N>& l, const T scalar) { return Vector<T, N>(l)-=scalar; }
  template<typename T, int N> inline static const Vector<T, N> BSS_FASTCALL operator *(const Vector<T, N>& l, const T scalar) { return Vector<T, N>(l)*=scalar; }
  template<typename T, int N> inline static const Vector<T, N> BSS_FASTCALL operator /(const Vector<T, N>& l, const T scalar) { return Vector<T, N>(l)/=scalar; }
  template<typename T, int N> inline static const Vector<T, N> BSS_FASTCALL operator +(const T scalar, const Vector<T, N>& r) { return Vector<T, N>(r)+=scalar; }
  template<typename T, int N> inline static const Vector<T, N> BSS_FASTCALL operator -(const T scalar, const Vector<T, N>& r) {
    Vector<T, N> ret(r); for(int i = 0; i < N; ++i) ret.v[i] = scalar - r.v[i]; return ret;
  }
  template<typename T, int N> inline static const Vector<T, N> BSS_FASTCALL operator *(const T scalar, const Vector<T, N>& r) { return Vector<T, N>(r)*=scalar; }
  template<typename T, int N> inline static const Vector<T, N> BSS_FASTCALL operator /(const T scalar, const Vector<T, N>& r) {
    Vector<T, N> ret(r); for(int i = 0; i < N; ++i) ret.v[i] = scalar / r.v[i]; return ret;
  }

  template<typename T, int N> inline static Vector<T, N>& BSS_FASTCALL operator +=(Vector<T, N>& l, const Vector<T, N>& r) { for(int i = 0; i < N; ++i) l.v[i] += r.v[i]; return l; }
  template<typename T, int N> inline static Vector<T, N>& BSS_FASTCALL operator -=(Vector<T, N>& l, const Vector<T, N>& r) { for(int i = 0; i < N; ++i) l.v[i] -= r.v[i]; return l; }
  template<typename T, int N> inline static Vector<T, N>& BSS_FASTCALL operator *=(Vector<T, N>& l, const Vector<T, N>& r) { for(int i = 0; i < N; ++i) l.v[i] *= r.v[i]; return l; }
  template<typename T, int N> inline static Vector<T, N>& BSS_FASTCALL operator /=(Vector<T, N>& l, const Vector<T, N>& r) { for(int i = 0; i < N; ++i) l.v[i] /= r.v[i]; return l; }
  template<typename T, int N> inline static Vector<T, N>& BSS_FASTCALL operator +=(Vector<T, N>& l, const T scalar) { for(int i = 0; i < N; ++i) l.v[i] += scalar; return l; }
  template<typename T, int N> inline static Vector<T, N>& BSS_FASTCALL operator -=(Vector<T, N>& l, const T scalar) { for(int i = 0; i < N; ++i) l.v[i] -= scalar; return l; }
  template<typename T, int N> inline static Vector<T, N>& BSS_FASTCALL operator *=(Vector<T, N>& l, const T scalar) { for(int i = 0; i < N; ++i) l.v[i] *= scalar; return l; }
  template<typename T, int N> inline static Vector<T, N>& BSS_FASTCALL operator /=(Vector<T, N>& l, const T scalar) { for(int i = 0; i < N; ++i) l.v[i] /= scalar; return l; }

  template<typename T, int N> inline static const Vector<T, N> operator -(const Vector<T, N>& l) { for(int i = 0; i < N; ++i) l.v[i] = -l.v[i]; return l; }

  template<typename T, int N> inline static bool BSS_FASTCALL operator !=(Vector<T, N>& l, const Vector<T, N>& r) { bool ret = true; for(int i = 0; i < N; ++i) ret = ret || (l.v[i] != r.v[i]); return ret; }
  template<typename T, int N> inline static bool BSS_FASTCALL operator ==(Vector<T, N>& l, const Vector<T, N>& r) { bool ret = true; for(int i = 0; i < N; ++i) ret = ret && (l.v[i] == r.v[i]); return ret; }
  template<typename T, int N> inline static bool BSS_FASTCALL operator !=(Vector<T, N>& l, const T scalar) { bool ret = true; for(int i = 0; i < N; ++i) ret = ret || (l.v[i] != scalar); return ret; }
  template<typename T, int N> inline static bool BSS_FASTCALL operator ==(Vector<T, N>& l, const T scalar) { bool ret = true; for(int i = 0; i < N; ++i) ret = ret && (l.v[i] == scalar); return ret; }
  template<typename T, int N> inline static bool BSS_FASTCALL operator >(Vector<T, N>& l, const Vector<T, N>& r) { char c = 0; for(int i = 0; i < N && !c; ++i) c = SGNCOMPARE(l.v[i], r.v[i]); return c>0; }
  template<typename T, int N> inline static bool BSS_FASTCALL operator <(Vector<T, N>& l, const Vector<T, N>& r) { char c = 0; for(int i = 0; i < N && !c; ++i) c = SGNCOMPARE(l.v[i], r.v[i]); return c<0; }
  template<typename T, int N> inline static bool BSS_FASTCALL operator >=(Vector<T, N>& l, const Vector<T, N>& r) { return !operator<(l, r); }
  template<typename T, int N> inline static bool BSS_FASTCALL operator <=(Vector<T, N>& l, const Vector<T, N>& r) { return !operator>(l, r); }
  template<typename T, int N> inline static bool BSS_FASTCALL operator >(Vector<T, N>& l, const T scalar) { bool ret = true; for(int i = 0; i < N; ++i) ret = ret && (l.v[i] > scalar); return ret; }
  template<typename T, int N> inline static bool BSS_FASTCALL operator <(Vector<T, N>& l, const T scalar) { bool ret = true; for(int i = 0; i < N; ++i) ret = ret && (l.v[i] < scalar); return ret; }
  template<typename T, int N> inline static bool BSS_FASTCALL operator >=(Vector<T, N>& l, const T scalar) { bool ret = true; for(int i = 0; i < N; ++i) ret = ret && (l.v[i] >= scalar); return ret; }
  template<typename T, int N> inline static bool BSS_FASTCALL operator <=(Vector<T, N>& l, const T scalar) { bool ret = true; for(int i = 0; i < N; ++i) ret = ret && (l.v[i] <= scalar); return ret; }

  // Component-wise matrix operations. Note that component-wise matrix multiplication is ^
  template<typename T, int M, int N> inline static const Matrix<T, M, N> BSS_FASTCALL operator +(const Matrix<T, M, N>& l, const Matrix<T, M, N>& r) { return Matrix<T, M, N>(l)+=r; }
  template<typename T, int M, int N> inline static const Matrix<T, M, N> BSS_FASTCALL operator -(const Matrix<T, M, N>& l, const Matrix<T, M, N>& r) { return Matrix<T, M, N>(l)-=r; }
  template<typename T, int M, int N> inline static const Matrix<T, M, N> BSS_FASTCALL operator ^(const Matrix<T, M, N>& l, const Matrix<T, M, N>& r) { return Matrix<T, M, N>(l)^=r; }
  template<typename T, int M, int N> inline static const Matrix<T, M, N> BSS_FASTCALL operator /(const Matrix<T, M, N>& l, const Matrix<T, M, N>& r) { return Matrix<T, M, N>(l)/=r; }
  template<typename T, int M, int N> inline static const Matrix<T, M, N> BSS_FASTCALL operator +(const Matrix<T, M, N>& l, const T scalar) { return Matrix<T, M, N>(l)+=scalar; }
  template<typename T, int M, int N> inline static const Matrix<T, M, N> BSS_FASTCALL operator -(const Matrix<T, M, N>& l, const T scalar) { return Matrix<T, M, N>(l)-=scalar; }
  template<typename T, int M, int N> inline static const Matrix<T, M, N> BSS_FASTCALL operator *(const Matrix<T, M, N>& l, const T scalar) { return Matrix<T, M, N>(l)*=scalar; }
  template<typename T, int M, int N> inline static const Matrix<T, M, N> BSS_FASTCALL operator /(const Matrix<T, M, N>& l, const T scalar) { return Matrix<T, M, N>(l)/=scalar; }
  template<typename T, int M, int N> inline static const Matrix<T, M, N> BSS_FASTCALL operator +(const T scalar, const Matrix<T, M, N>& r) { return Matrix<T, M, N>(r)+=scalar; }
  template<typename T, int M, int N> inline static const Matrix<T, M, N> BSS_FASTCALL operator -(const T scalar, const Matrix<T, M, N>& r) { return Matrix<T, M, N>(r)-=scalar; }
  template<typename T, int M, int N> inline static const Matrix<T, M, N> BSS_FASTCALL operator *(const T scalar, const Matrix<T, M, N>& r) { return Matrix<T, M, N>(r)*=scalar; }
  template<typename T, int M, int N> inline static const Matrix<T, M, N> BSS_FASTCALL operator /(const T scalar, const Matrix<T, M, N>& r) { return Matrix<T, M, N>(r)/=scalar; }

  template<typename T, int M, int N> inline static Matrix<T, M, N>& BSS_FASTCALL operator +=(Matrix<T, M, N>& l, const Matrix<T, M, N>& r) { for(int i = 0; i < N; ++i) for(int j = 0; j < N; ++j) l.v[i][j] += r.v[i][j]; return l; }
  template<typename T, int M, int N> inline static Matrix<T, M, N>& BSS_FASTCALL operator -=(Matrix<T, M, N>& l, const Matrix<T, M, N>& r) { for(int i = 0; i < N; ++i) for(int j = 0; j < N; ++j) l.v[i][j] -= r.v[i][j]; return l; }
  template<typename T, int M, int N> inline static Matrix<T, M, N>& BSS_FASTCALL operator ^=(Matrix<T, M, N>& l, const Matrix<T, M, N>& r) { for(int i = 0; i < N; ++i) for(int j = 0; j < N; ++j) l.v[i][j] *= r.v[i][j]; return l; }
  template<typename T, int M, int N> inline static Matrix<T, M, N>& BSS_FASTCALL operator /=(Matrix<T, M, N>& l, const Matrix<T, M, N>& r) { for(int i = 0; i < N; ++i) for(int j = 0; j < N; ++j) l.v[i][j] /= r.v[i][j]; return l; }
  template<typename T, int M, int N> inline static Matrix<T, M, N>& BSS_FASTCALL operator +=(Matrix<T, M, N>& l, const T scalar) { for(int i = 0; i < N; ++i) for(int j = 0; j < N; ++j) l.v[i][j] += scalar; return l; }
  template<typename T, int M, int N> inline static Matrix<T, M, N>& BSS_FASTCALL operator -=(Matrix<T, M, N>& l, const T scalar) { for(int i = 0; i < N; ++i) for(int j = 0; j < N; ++j) l.v[i][j] -= scalar; return l; }
  template<typename T, int M, int N> inline static Matrix<T, M, N>& BSS_FASTCALL operator *=(Matrix<T, M, N>& l, const T scalar) { for(int i = 0; i < N; ++i) for(int j = 0; j < N; ++j) l.v[i][j] *= scalar; return l; }
  template<typename T, int M, int N> inline static Matrix<T, M, N>& BSS_FASTCALL operator /=(Matrix<T, M, N>& l, const T scalar) { for(int i = 0; i < N; ++i) for(int j = 0; j < N; ++j) l.v[i][j] /= scalar; return l; }

  template<typename T, int M, int N> inline static const Matrix<T, M, N> operator -(const Matrix<T, M, N>& l) { return l*((T)-1); }
  template<typename T, int M, int N> inline static const Matrix<T, M, N> operator ~(const Matrix<T, M, N>& l) { return l.Transpose(); }

  template<typename T, int M, int N>
  inline static const Vector<T, N> BSS_FASTCALL operator *(const Vector<T, M>& v, const Matrix<T, M, N>& m) // Does v * M and assumes v is a row vector
  {
    Vector<T, N> out;
    MatrixMultiply<T, 1, M, N>(v.v_row, m.v, out.v_row);
    return out;
  }
  template<typename T, int M, int N>
  inline static const Vector<T, N> BSS_FASTCALL operator *(const Matrix<T, M, N>& m, const Vector<T, N>& v) // Does M * v and assumes v is a column vector
  {
    Vector<T, M> out;
    MatrixMultiply<T, M, N, 1>(m.v, v.v_column, out.v_column);
    return out;
  }
  template<typename T, int N>
  inline static Matrix<T, N, N>& BSS_FASTCALL operator *=(Vector<T, N>& l, const Matrix<T, N, N>& r) // Does v *= M and assumes v is a row vector
  {
    MatrixMultiply<T, 1, N, N>(l.v_row, r.v, l.v_row);
    return l;
  }
  template<typename T, int M, int N, int P>
  inline static const Matrix<T, M, P> BSS_FASTCALL operator *(const Matrix<T, M, N>& l, const Matrix<T, N, P>& r) // Does M * M
  {
    Matrix<T, M, P> out;
    MatrixMultiply<T, M, N, P>(l.v, r.v, out.v);
    return out;
  }
  template<typename T, int M, int N>
  inline static Matrix<T, M, N>& BSS_FASTCALL operator *=(Matrix<T, M, N>& l, const Matrix<T, N, N>& r) // Does M *= M
  {
    MatrixMultiply<T, M, N, N>(l.v, r.v, l.v);
    return l;
  }
}


#endif