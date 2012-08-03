// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_SSE_H__
#define __BSS_SSE_H__

#include "bss_compiler.h"
#include "bss_deprecated.h"
#include <xmmintrin.h>

// Define compiler-specific intrinsics for working with SSE.
#ifdef BSS_COMPILER_INTEL

#elif defined BSS_COMPILER_GCC // GCC

#elif defined BSS_COMPILER_MSC // VC++
#define BSS_SSE_LOAD_APS _mm_load_ps
#define BSS_SSE_LOAD_UPS _mm_loadu_ps
#define BSS_SSE_SET_PS _mm_set_ps
#define BSS_SSE_STORE_APS _mm_store_ps
#define BSS_SSE_STORE_UPS _mm_storeu_ps
#define BSS_SSE_ADD_PS _mm_add_ps
#define BSS_SSE_SUB_PS _mm_sub_ps
#define BSS_SSE_MUL_PS _mm_mul_ps
#define BSS_SSE_DIV_PS _mm_div_ps
#define BSS_SSE_ADD_EPI32 _mm_add_epi32
#define BSS_SSE_SUB_PI32 _mm_sub_pi32
#define BSS_SSE_M128 __m128
#define BSS_SSE_M128i __m128i
#endif

// Struct that wraps around a pointer to signify that it is not aligned
template<typename T>
struct BSS_UNALIGNED {
  inline explicit BSS_UNALIGNED(T* p) : _p(p) { } 
  T* _p;
};

// Struct that abstracts out a lot of common SSE operations
BSS_ALIGNED_STRUCT(16) sseVec
{
  BSS_FORCEINLINE sseVec(BSS_SSE_M128 v) : xmm(v) {}
  BSS_FORCEINLINE sseVec(float v) : xmm(_mm_set_ps1(v)) {}
  //BSS_FORCEINLINE sseVec(const float*BSS_RESTRICT v) : xmm(BSS_SSE_LOAD_APS(v)) {}
  BSS_FORCEINLINE explicit sseVec(const float (&v)[4]) : xmm(BSS_SSE_LOAD_APS(v)) { assert(!(((size_t)v)%16)); }
  BSS_FORCEINLINE explicit sseVec(BSS_UNALIGNED<float> v) : xmm(BSS_SSE_LOAD_UPS(v._p)) {}
  BSS_FORCEINLINE sseVec(float x,float y,float z,float w) : xmm(BSS_SSE_SET_PS(w,z,y,x)) {}
  BSS_FORCEINLINE sseVec operator+(const sseVec& r) const { return sseVec(BSS_SSE_ADD_PS(xmm, r.xmm)); } // These don't return const sseVec because it makes things messy.
  BSS_FORCEINLINE sseVec operator-(const sseVec& r) const { return sseVec(BSS_SSE_SUB_PS(xmm, r.xmm)); }
  BSS_FORCEINLINE sseVec operator*(const sseVec& r) const { return sseVec(BSS_SSE_MUL_PS(xmm, r.xmm)); }
  BSS_FORCEINLINE sseVec operator/(const sseVec& r) const { return sseVec(BSS_SSE_DIV_PS(xmm, r.xmm)); }
  BSS_FORCEINLINE sseVec& operator+=(const sseVec& r) { xmm=BSS_SSE_ADD_PS(xmm, r.xmm); return *this; }
  BSS_FORCEINLINE sseVec& operator-=(const sseVec& r) { xmm=BSS_SSE_SUB_PS(xmm, r.xmm); return *this; }
  BSS_FORCEINLINE sseVec& operator*=(const sseVec& r) { xmm=BSS_SSE_MUL_PS(xmm, r.xmm); return *this; }
  BSS_FORCEINLINE sseVec& operator/=(const sseVec& r) { xmm=BSS_SSE_DIV_PS(xmm, r.xmm); return *this; }
  BSS_FORCEINLINE operator BSS_SSE_M128() { return xmm; }
  //BSS_FORCEINLINE void operator>>(float*BSS_RESTRICT v) { BSS_SSE_STORE_APS(v, xmm); }
  BSS_FORCEINLINE void operator>>(float (&v)[4]) { assert(!(((size_t)v)%16)); BSS_SSE_STORE_APS(v, xmm); }
  BSS_FORCEINLINE void operator>>(BSS_UNALIGNED<float> v) { BSS_SSE_STORE_UPS(v._p, xmm); }
  
  BSS_SSE_M128 xmm;
};

#endif