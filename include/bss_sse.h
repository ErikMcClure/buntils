// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_SSE_H__
#define __BSS_SSE_H__

#include "bss_compiler.h"
#include "bss_deprecated.h"
#include <emmintrin.h>

// Define compiler-specific intrinsics for working with SSE.
#ifdef BSS_COMPILER_INTEL

#elif defined BSS_COMPILER_GCC // GCC

#elif defined BSS_COMPILER_MSC // VC++
#define BSS_SSE_LOAD_APS _mm_load_ps
#define BSS_SSE_LOAD_UPS _mm_loadu_ps
#define BSS_SSE_SET_PS _mm_set_ps
#define BSS_SSE_SET1_PS _mm_set_ps1
#define BSS_SSE_STORE_APS _mm_store_ps
#define BSS_SSE_STORE_UPS _mm_storeu_ps
#define BSS_SSE_ADD_PS _mm_add_ps
#define BSS_SSE_SUB_PS _mm_sub_ps
#define BSS_SSE_MUL_PS _mm_mul_ps
#define BSS_SSE_DIV_PS _mm_div_ps

#define BSS_SSE_LOAD_ASI128 _mm_load_si128
#define BSS_SSE_LOAD_USI128 _mm_loadu_si128 
#define BSS_SSE_SET_EPI32 _mm_set_epi32
#define BSS_SSE_SET1_EPI32 _mm_set1_epi32
#define BSS_SSE_STORE_ASI128 _mm_store_si128 
#define BSS_SSE_STORE_USI128 _mm_storeu_si128 
#define BSS_SSE_ADD_EPI32 _mm_add_epi32
#define BSS_SSE_SUB_EPI32 _mm_sub_epi32
#define BSS_SSE_AND _mm_and_si128 
#define BSS_SSE_OR _mm_or_si128 
#define BSS_SSE_XOR _mm_xor_si128
#define BSS_SSE_SL_EPI32 _mm_sll_epi32
#define BSS_SSE_SLI_EPI32 _mm_slli_epi32
#define BSS_SSE_SR_EPI32 _mm_srl_epi32
#define BSS_SSE_SRI_EPI32 _mm_srli_epi32
#define BSS_SSE_SAR_EPI32 _mm_sra_epi32 //Arithmetic right shift
#define BSS_SSE_SARI_EPI32 _mm_srai_epi32
#define BSS_SSE_CMPEQ_EPI32 _mm_cmpeq_epi32
#define BSS_SSE_CMPLT_EPI32 _mm_cmplt_epi32
#define BSS_SSE_CMPGT_EPI32 _mm_cmpgt_epi32
#define BSS_SSE_SHUFFLE_EPI32 _mm_shuffle_epi32

#define BSS_SSE_SET_EPI64 _mm_set_epi64
#define BSS_SSE_SET1_EPI64 _mm_set1_epi64
#define BSS_SSE_ADD_EPI64 _mm_add_epi64
#define BSS_SSE_SUB_EPI64 _mm_sub_epi64
#define BSS_SSE_SL_EPI64 _mm_sll_epi64
#define BSS_SSE_SLI_EPI64 _mm_slli_epi64
#define BSS_SSE_SR_EPI64 _mm_srl_epi64
#define BSS_SSE_SRI_EPI64 _mm_srli_epi64

#define BSS_SSE_SHUFFLEHI_EPI16 _mm_shufflehi_epi16
#define BSS_SSE_SHUFFLELO_EPI16 _mm_shufflelo_epi16

#define BSS_SSE_TPS_EPI32 _mm_cvttps_epi32 //converts using truncation
#define BSS_SSE_PS_EPI32 _mm_cvtps_epi32
#define BSS_SSE_EPI32_PS _mm_cvtepi32_ps
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
  BSS_FORCEINLINE explicit sseVec(BSS_SSE_M128i v) : xmm(BSS_SSE_EPI32_PS(v)) {}
  BSS_FORCEINLINE sseVec(float v) : xmm(BSS_SSE_SET1_PS(v)) {}
  //BSS_FORCEINLINE sseVec(const float*BSS_RESTRICT v) : xmm(BSS_SSE_LOAD_APS(v)) { assert(!(((size_t)v)%16)); }
  BSS_FORCEINLINE explicit sseVec(const float (&v)[4]) : xmm(BSS_SSE_LOAD_APS(v)) { assert(!(((size_t)v)%16)); }
  BSS_FORCEINLINE explicit sseVec(BSS_UNALIGNED<const float> v) : xmm(BSS_SSE_LOAD_UPS(v._p)) {}
  BSS_FORCEINLINE sseVec(float x,float y,float z,float w) : xmm(BSS_SSE_SET_PS(w,z,y,x)) {}
  BSS_FORCEINLINE const sseVec operator+(const sseVec& r) const { return sseVec(BSS_SSE_ADD_PS(xmm, r.xmm)); }
  BSS_FORCEINLINE const sseVec operator-(const sseVec& r) const { return sseVec(BSS_SSE_SUB_PS(xmm, r.xmm)); }
  BSS_FORCEINLINE const sseVec operator*(const sseVec& r) const { return sseVec(BSS_SSE_MUL_PS(xmm, r.xmm)); }
  BSS_FORCEINLINE const sseVec operator/(const sseVec& r) const { return sseVec(BSS_SSE_DIV_PS(xmm, r.xmm)); }
  BSS_FORCEINLINE sseVec& operator+=(const sseVec& r) { xmm=BSS_SSE_ADD_PS(xmm, r.xmm); return *this; }
  BSS_FORCEINLINE sseVec& operator-=(const sseVec& r) { xmm=BSS_SSE_SUB_PS(xmm, r.xmm); return *this; }
  BSS_FORCEINLINE sseVec& operator*=(const sseVec& r) { xmm=BSS_SSE_MUL_PS(xmm, r.xmm); return *this; }
  BSS_FORCEINLINE sseVec& operator/=(const sseVec& r) { xmm=BSS_SSE_DIV_PS(xmm, r.xmm); return *this; }
  BSS_FORCEINLINE operator BSS_SSE_M128() const { return xmm; }
  //BSS_FORCEINLINE void operator>>(float*BSS_RESTRICT v) { assert(!(((size_t)v)%16)); BSS_SSE_STORE_APS(v, xmm); }
  BSS_FORCEINLINE void operator>>(float (&v)[4]) const { assert(!(((size_t)v)%16)); BSS_SSE_STORE_APS(v, xmm); }
  BSS_FORCEINLINE void operator>>(BSS_UNALIGNED<float> v) const { BSS_SSE_STORE_UPS(v._p, xmm); }
  
  BSS_SSE_M128 xmm;
};

BSS_ALIGNED_STRUCT(16) sseVeci
{
  BSS_FORCEINLINE sseVeci(BSS_SSE_M128i v) : xmm(v) {} //__fastcall is obviously useless here since we're dealing with xmm registers
  BSS_FORCEINLINE explicit sseVeci(BSS_SSE_M128 v) : xmm(BSS_SSE_TPS_EPI32(v)) {}
  BSS_FORCEINLINE sseVeci(BSS_SSE_M128 v, char round) : xmm(BSS_SSE_PS_EPI32(v)) {}
  BSS_FORCEINLINE sseVeci(int v) : xmm(BSS_SSE_SET1_EPI32(v)) {}
  //BSS_FORCEINLINE sseVeci(const int*BSS_RESTRICT v) : xmm(BSS_SSE_LOAD_ASI128(v)) { assert(!(((size_t)v)%16)); }
  BSS_FORCEINLINE explicit sseVeci(const int (&v)[4]) : xmm(BSS_SSE_LOAD_ASI128((BSS_SSE_M128i*)v)) { assert(!(((size_t)v)%16)); }
  BSS_FORCEINLINE explicit sseVeci(BSS_UNALIGNED<const int> v) : xmm(BSS_SSE_LOAD_USI128((BSS_SSE_M128i*)v._p)) {}
  BSS_FORCEINLINE sseVeci(int x,int y,int z,int w) : xmm(BSS_SSE_SET_EPI32(w,z,y,x)) {}
  BSS_FORCEINLINE const sseVeci operator+(const sseVeci& r) const { return sseVeci(BSS_SSE_ADD_EPI32(xmm, r.xmm)); } // These don't return const sseVeci because it makes things messy.
  BSS_FORCEINLINE const sseVeci operator-(const sseVeci& r) const { return sseVeci(BSS_SSE_SUB_EPI32(xmm, r.xmm)); }
  BSS_FORCEINLINE const sseVeci operator&(const sseVeci& r) const { return sseVeci(BSS_SSE_AND(xmm, r.xmm)); }
  BSS_FORCEINLINE const sseVeci operator|(const sseVeci& r) const { return sseVeci(BSS_SSE_OR(xmm, r.xmm)); }
  BSS_FORCEINLINE const sseVeci operator^(const sseVeci& r) const { return sseVeci(BSS_SSE_XOR(xmm, r.xmm)); }
  BSS_FORCEINLINE const sseVeci operator>>(const sseVeci& r) const { return sseVeci(BSS_SSE_SAR_EPI32(xmm, r.xmm)); } // Amazingly enough this does not conflict with the extraction operator
  BSS_FORCEINLINE const sseVeci operator>>(int r) const { return sseVeci(BSS_SSE_SARI_EPI32(xmm, r)); }
  BSS_FORCEINLINE const sseVeci operator<<(const sseVeci& r) const { return sseVeci(BSS_SSE_SL_EPI32(xmm, r.xmm)); }
  BSS_FORCEINLINE const sseVeci operator<<(int r) const { return sseVeci(BSS_SSE_SLI_EPI32(xmm, r)); }
  BSS_FORCEINLINE sseVeci& operator+=(const sseVeci& r) { xmm=BSS_SSE_ADD_EPI32(xmm, r.xmm); return *this; }
  BSS_FORCEINLINE sseVeci& operator-=(const sseVeci& r) { xmm=BSS_SSE_SUB_EPI32(xmm, r.xmm); return *this; }
  BSS_FORCEINLINE sseVeci& operator&=(const sseVeci& r) { xmm=BSS_SSE_AND(xmm, r.xmm); return *this; }
  BSS_FORCEINLINE sseVeci& operator|=(const sseVeci& r) { xmm=BSS_SSE_OR(xmm, r.xmm); return *this; }
  BSS_FORCEINLINE sseVeci& operator^=(const sseVeci& r) { xmm=BSS_SSE_XOR(xmm, r.xmm); return *this; }
  BSS_FORCEINLINE sseVeci& operator>>=(const sseVeci& r) { xmm=BSS_SSE_SAR_EPI32(xmm, r.xmm); return *this; }
  BSS_FORCEINLINE sseVeci& operator>>=(int r) { xmm=BSS_SSE_SARI_EPI32(xmm, r); return *this; }
  BSS_FORCEINLINE sseVeci& operator<<=(const sseVeci& r) { xmm=BSS_SSE_SL_EPI32(xmm, r.xmm); return *this; }
  BSS_FORCEINLINE sseVeci& operator<<=(int r) { xmm=BSS_SSE_SLI_EPI32(xmm, r); return *this; }

  BSS_FORCEINLINE const sseVeci operator==(const sseVeci& r) const { return sseVeci(BSS_SSE_CMPEQ_EPI32(xmm,r.xmm)); }
  BSS_FORCEINLINE const sseVeci operator!=(const sseVeci& r) const { return !operator==(r); }
  BSS_FORCEINLINE const sseVeci operator<(const sseVeci& r) const { return sseVeci(BSS_SSE_CMPLT_EPI32(xmm,r.xmm)); }
  BSS_FORCEINLINE const sseVeci operator<=(const sseVeci& r) const { return !operator>(r); }
  BSS_FORCEINLINE const sseVeci operator>(const sseVeci& r) const { return sseVeci(BSS_SSE_CMPGT_EPI32(xmm,r.xmm)); }
  BSS_FORCEINLINE const sseVeci operator>=(const sseVeci& r) const { return !operator<(r); }
  BSS_FORCEINLINE operator BSS_SSE_M128i() const { return xmm; }
  BSS_FORCEINLINE const sseVeci operator!() const { return sseVeci(BSS_SSE_CMPEQ_EPI32(xmm,ZeroVector())); }
  BSS_FORCEINLINE const sseVeci operator~() const { return sseVeci(BSS_SSE_XOR(xmm, sseVeci(-1))); }
  BSS_FORCEINLINE sseVeci& operator=(BSS_SSE_M128i r) { xmm=r; return *this; }
  BSS_FORCEINLINE sseVeci& operator=(BSS_SSE_M128 r) { xmm=BSS_SSE_TPS_EPI32(r); return *this; }
  //BSS_FORCEINLINE void operator>>(int*BSS_RESTRICT v) { assert(!(((size_t)v)%16)); BSS_SSE_STORE_APS(v, xmm); }
  BSS_FORCEINLINE void operator>>(int (&v)[4]) const { assert(!(((size_t)v)%16)); BSS_SSE_STORE_ASI128((BSS_SSE_M128i*)v, xmm); }
  BSS_FORCEINLINE void operator>>(BSS_UNALIGNED<int> v) const { BSS_SSE_STORE_USI128((BSS_SSE_M128i*)v._p, xmm); }
  //BSS_FORCEINLINE void operator>>(int& v) { v=_mm_cvtsi128_si32(xmm); }
  static sseVeci ZeroVector() { return sseVeci(_mm_setzero_si128()); }

  BSS_SSE_M128i xmm;
};

BSS_ALIGNED_STRUCT(16) sseVeci64
{
  BSS_FORCEINLINE sseVeci64(BSS_SSE_M128i v) : xmm(v) {} //__fastcall is obviously useless here since we're dealing with xmm registers
  BSS_FORCEINLINE sseVeci64(__int64 v) { BSS_ALIGN(16) __int64 vv[2]={v,v}; xmm=BSS_SSE_LOAD_ASI128((BSS_SSE_M128i*)vv); }
  //BSS_FORCEINLINE sseVeci64(const int*BSS_RESTRICT v) : xmm(BSS_SSE_LOAD_ASI128(v)) { assert(!(((size_t)v)%16)); }
  BSS_FORCEINLINE explicit sseVeci64(const __int64 (&v)[2]) : xmm(BSS_SSE_LOAD_ASI128((BSS_SSE_M128i*)v)) { assert(!(((size_t)v)%16)); }
  BSS_FORCEINLINE explicit sseVeci64(BSS_UNALIGNED<const __int64> v) : xmm(BSS_SSE_LOAD_USI128((BSS_SSE_M128i*)v._p)) {}
  BSS_FORCEINLINE sseVeci64(__int64 x,__int64 y) { BSS_ALIGN(16) __int64 xy[2]={x,y}; xmm=BSS_SSE_LOAD_ASI128((BSS_SSE_M128i*)xy); }
  BSS_FORCEINLINE const sseVeci64 operator+(const sseVeci64& r) const { return sseVeci64(BSS_SSE_ADD_EPI64(xmm, r.xmm)); } // These don't return const sseVeci64 because it makes things messy.
  BSS_FORCEINLINE const sseVeci64 operator-(const sseVeci64& r) const { return sseVeci64(BSS_SSE_SUB_EPI64(xmm, r.xmm)); }
  BSS_FORCEINLINE const sseVeci64 operator&(const sseVeci64& r) const { return sseVeci64(BSS_SSE_AND(xmm, r.xmm)); }
  BSS_FORCEINLINE const sseVeci64 operator|(const sseVeci64& r) const { return sseVeci64(BSS_SSE_OR(xmm, r.xmm)); }
  BSS_FORCEINLINE const sseVeci64 operator^(const sseVeci64& r) const { return sseVeci64(BSS_SSE_XOR(xmm, r.xmm)); }
  BSS_FORCEINLINE const sseVeci64 operator>>(const sseVeci64& r) const { return sseVeci64(BSS_SSE_SR_EPI64(xmm, r.xmm)); } // Amazingly enough this does not conflict with the extraction operator
  BSS_FORCEINLINE const sseVeci64 operator>>(int r) const { return sseVeci64(BSS_SSE_SRI_EPI64(xmm, r)); }
  BSS_FORCEINLINE const sseVeci64 operator<<(const sseVeci64& r) const { return sseVeci64(BSS_SSE_SL_EPI64(xmm, r.xmm)); }
  BSS_FORCEINLINE const sseVeci64 operator<<(int r) const { return sseVeci64(BSS_SSE_SLI_EPI64(xmm, r)); }
  BSS_FORCEINLINE sseVeci64& operator+=(const sseVeci64& r) { xmm=BSS_SSE_ADD_EPI64(xmm, r.xmm); return *this; }
  BSS_FORCEINLINE sseVeci64& operator-=(const sseVeci64& r) { xmm=BSS_SSE_SUB_EPI64(xmm, r.xmm); return *this; }
  BSS_FORCEINLINE sseVeci64& operator&=(const sseVeci64& r) { xmm=BSS_SSE_AND(xmm, r.xmm); return *this; }
  BSS_FORCEINLINE sseVeci64& operator|=(const sseVeci64& r) { xmm=BSS_SSE_OR(xmm, r.xmm); return *this; }
  BSS_FORCEINLINE sseVeci64& operator^=(const sseVeci64& r) { xmm=BSS_SSE_XOR(xmm, r.xmm); return *this; }
  BSS_FORCEINLINE sseVeci64& operator>>=(const sseVeci64& r) { xmm=BSS_SSE_SR_EPI64(xmm, r.xmm); return *this; }
  BSS_FORCEINLINE sseVeci64& operator>>=(int r) { xmm=BSS_SSE_SRI_EPI64(xmm, r); return *this; }
  BSS_FORCEINLINE sseVeci64& operator<<=(const sseVeci64& r) { xmm=BSS_SSE_SL_EPI64(xmm, r.xmm); return *this; }
  BSS_FORCEINLINE sseVeci64& operator<<=(int r) { xmm=BSS_SSE_SLI_EPI64(xmm, r); return *this; }

  BSS_FORCEINLINE operator BSS_SSE_M128i() const { return xmm; }
  BSS_FORCEINLINE const sseVeci64 operator~() const { return sseVeci64(BSS_SSE_XOR(xmm, sseVeci64(-1))); }
  BSS_FORCEINLINE sseVeci64& operator=(BSS_SSE_M128i r) { xmm=r; return *this; }
  BSS_FORCEINLINE void operator>>(__int64 (&v)[2]) const { assert(!(((size_t)v)%16)); BSS_SSE_STORE_ASI128((BSS_SSE_M128i*)v, xmm); }
  BSS_FORCEINLINE void operator>>(BSS_UNALIGNED<__int64> v) const { BSS_SSE_STORE_USI128((BSS_SSE_M128i*)v._p, xmm); }
  static sseVeci64 ZeroVector() { return sseVeci64(_mm_setzero_si128()); }

  BSS_SSE_M128i xmm;
};

#endif