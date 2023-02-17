// Copyright ©2018 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BUN_SSE_H__
#define __BUN_SSE_H__

#include "defines.h"
#include <assert.h>
#include <emmintrin.h>
#include <stdint.h>

#ifndef BUN_SSE_ENABLED
#include "buntils.h"
#endif

namespace bun {
#ifndef BUN_SSE_ENABLED
#define BUN_SSE_M128 std::array<float, 4>
#define BUN_SSE_M128i8 std::array<int8_t, 16>
#define BUN_SSE_M128i16 std::array<int16_t, 8>
#define BUN_SSE_M128i std::array<int32_t, 4>
#define BUN_SSE_M128i64 std::array<int64_t, 2>
#define BUN_SSE_M128d std::array<double, 2>

struct bun_si128
{
  std::array<int64_t, 2> v;
  inline bun_si128(const std::array<int8_t, 16>& i) : v(*(std::array<int64_t, 2>*)&i) {}
  inline bun_si128(const std::array<int16_t, 8>& i) : v(*(std::array<int64_t, 2>*)&i) {}
  inline bun_si128(const std::array<int32_t, 4>& i) : v(*(std::array<int64_t, 2>*)&i) {}
  inline bun_si128(const std::array<int64_t, 2>& i) : v(i) {}
  inline operator std::array<int8_t, 16>() { return *(std::array<int8_t, 16>*)&v; }
  inline operator std::array<int16_t, 8>() { return *(std::array<int16_t, 8>*)&v; }
  inline operator std::array<int32_t, 4>() { return *(std::array<int32_t, 4>*)&v; }
  inline operator std::array<int64_t, 2>() { return v; }
};

template<class T, size_t I, T(*F)(T, T)>
BUN_FORCEINLINE std::array<T, I> _sse_array_single(const std::array<T, I>& a, const std::array<T, I>& b)
{
  std::array<T, I> r(a);
  r[0] = F(r[0], b[0]);
  return r;
}

template<class T, size_t TI, class D, size_t DI>
BUN_FORCEINLINE std::array<D, DI> _sse_array_cvt(const std::array<T, TI>& a)
{
  static_assert(DI >= TI, "DI can't be less than TI");
  std::array<D, DI> r;
  for(size_t i = 0; i < TI; ++i)
    r[i] = (D)a[i];
  return r;
}

template<class T, size_t I, T(*F)(T, int)>
BUN_FORCEINLINE std::array<T, I> _sse_array_shift(const std::array<T, I>& a, int s)
{
  std::array<T, I> r;
  for(size_t i = 0; i < I; ++i)
    r[i] = F(a[i], s);
  return r;
}

template<class T>
BUN_FORCEINLINE std::array<T, 2> _sse_array_shuffle2(const std::array<T, 2>& a, const std::array<T, 2>& b, int i)
{
  std::array<T, 2> r;
  r[0] = a[i != 0]; // According to the SSE docs, the instruction treats all nonzero numbers as one, even though the immediate integer has two bits reserved for each index.
  r[1] = b[(i >> 2) != 0];
  return r;
}

template<class T>
BUN_FORCEINLINE std::array<T, 4> _sse_array_shuffle4(const std::array<T, 4>& a, const std::array<T, 4>& b, int i)
{
  std::array<T, 4> r;
  r[0] = a[i & 3];
  r[1] = a[(i >> 2) & 3];
  r[2] = b[(i >> 4) & 3];
  r[3] = b[(i >> 6) & 3];
  return r;
}

template<class T, size_t O>
BUN_FORCEINLINE std::array<T, 8> _sse_array_shuffle8(const std::array<T, 8>& a, int i)
{
  std::array<T, 8> r(a);
  r[0 + O] = a[(i & 3) + O];
  r[1 + O] = a[((i >> 2) & 3) + O];
  r[2 + O] = a[((i >> 4) & 3) + O];
  r[3 + O] = a[((i >> 6) & 3) + O];
  return r;
}

template<class T, int I, T(*F)(T)>
BUN_FORCEINLINE std::array<T, I> _sse_array_unaryop(const std::array<T, I>& a)
{
  std::array<T, I> r;
  for(size_t i = 0; i < I; ++i)
    r[i] = F(a[i]);
  return r;
}

template<class T> BUN_FORCEINLINE T _sse_array_add(T l, T r) { return l + r; }
template<class T> BUN_FORCEINLINE T _sse_array_sub(T l, T r) { return l - r; }
template<class T> BUN_FORCEINLINE T _sse_array_mul(T l, T r) { return l * r; }
template<class T> BUN_FORCEINLINE T _sse_array_div(T l, T r) { return l / r; }
template<class T> BUN_FORCEINLINE T _sse_array_min(T l, T r) { return bun_min(l, r); }
template<class T> BUN_FORCEINLINE T _sse_array_max(T l, T r) { return bun_max(l, r); }
template<class T> BUN_FORCEINLINE T _sse_array_and(T l, T r) { return l & r; }
template<class T> BUN_FORCEINLINE T _sse_array_andnot(T l, T r) { return (~l) & r; }
template<class T> BUN_FORCEINLINE T _sse_array_or(T l, T r) { return l | r; }
template<class T> BUN_FORCEINLINE T _sse_array_xor(T l, T r) { return l ^ r; }
template<class T, bool(*F)(T, T)> BUN_FORCEINLINE T _sse_array_cmp(T l, T r) { typename bun::TBitLimit<T>::UNSIGNED t = (F(l, r) ? ((typename bun::TBitLimit<T>::UNSIGNED)~0) : 0); return *(T*)&t; }
template<class T> BUN_FORCEINLINE bool _sse_array_cmpeq(T l, T r) { return l == r; }
template<class T> BUN_FORCEINLINE bool _sse_array_cmpneq(T l, T r) { return l != r; }
template<class T> BUN_FORCEINLINE bool _sse_array_cmplt(T l, T r) { return l < r; }
template<class T> BUN_FORCEINLINE bool _sse_array_cmple(T l, T r) { return l <= r; }
template<class T> BUN_FORCEINLINE bool _sse_array_cmpgt(T l, T r) { return l > r; }
template<class T> BUN_FORCEINLINE bool _sse_array_cmpge(T l, T r) { return l >= r; }
template<class T> BUN_FORCEINLINE T _sse_array_sl(T l, int r) { return l << r; }
template<class T> BUN_FORCEINLINE T _sse_array_sr(T l, int r) { return T(typename std::make_unsigned<T>::type(l) >> r); }
template<class T> BUN_FORCEINLINE T _sse_array_sra(T l, int r) { return T(typename std::make_signed<T>::type(l) >> r); }

template<class T> BUN_FORCEINLINE bool _sse_array_shuffle(T l, T r) { return l >= r; }

// Define compiler-specific intrinsics for working with SSE.
#define BUN_SSE_LOAD_APS(x) BUN_SSE_M128{(x)[0], (x)[1], (x)[2], (x)[3]}
#define BUN_SSE_LOAD_UPS BUN_SSE_LOAD_APS
#define BUN_SSE_SET_PS(a, b, c, d) BUN_SSE_M128{d, c, b, a}
#define BUN_SSE_SET1_PS(a) BUN_SSE_M128{a, a, a, a}
#define BUN_SSE_STORE_APS(v, a) (v)[0] = (a)[0]; (v)[1] = (a)[1]; (v)[2] = (a)[2]; (v)[3] = (a)[3]
#define BUN_SSE_STORE_UPS BUN_SSE_STORE_APS
#define BUN_SSE_ADD_PS bun::ArrayMap<float, 4, _sse_array_add<float>>
#define BUN_SSE_SUB_PS bun::ArrayMap<float, 4, _sse_array_sub<float>>
#define BUN_SSE_MUL_PS bun::ArrayMap<float, 4, _sse_array_mul<float>>
#define BUN_SSE_DIV_PS bun::ArrayMap<float, 4, _sse_array_div<float>>
#define BUN_SSE_MIN_PS bun::ArrayMap<float, 4, _sse_array_min<float>>
#define BUN_SSE_MAX_PS bun::ArrayMap<float, 4, _sse_array_max<float>>
#define BUN_SSE_FLOOR_PS _sse_array_unaryop<float, 4, floorf>
#define BUN_SSE_CEIL_PS _sse_array_unaryop<float, 4, ceilf>
#define BUN_SSE_CMPEQ_PS bun::ArrayMap<float, 4, _sse_array_cmp<float, _sse_array_cmpeq<float>>>
#define BUN_SSE_CMPNEQ_PS bun::ArrayMap<float, 4, _sse_array_cmp<float, _sse_array_cmpneq<float>>>
#define BUN_SSE_CMPLT_PS bun::ArrayMap<float, 4, _sse_array_cmp<float, _sse_array_cmplt<float>>>
#define BUN_SSE_CMPLTE_PS bun::ArrayMap<float, 4, _sse_array_cmp<float, _sse_array_cmple<float>>>
#define BUN_SSE_CMPGT_PS bun::ArrayMap<float, 4, _sse_array_cmp<float, _sse_array_cmpgt<float>>>
#define BUN_SSE_CMPGTE_PS bun::ArrayMap<float, 4, _sse_array_cmp<float, _sse_array_cmpge<float>>>
#define BUN_SSE_SETZERO_PS() BUN_SSE_M128{0, 0, 0, 0}
BUN_FORCEINLINE BUN_SSE_M128 BUN_SSE_MOVEHL_PS(BUN_SSE_M128 a, BUN_SSE_M128 b) { return BUN_SSE_M128{ (b)[2],(b)[3],(a)[2],(a)[3] }; }
BUN_FORCEINLINE BUN_SSE_M128 BUN_SSE_MOVELH_PS(BUN_SSE_M128 a, BUN_SSE_M128 b) { return BUN_SSE_M128{ (a)[0],(a)[1],(b)[0],(b)[1] }; }
#define BUN_SSE_ADD_SS _sse_array_single<float, 4, _sse_array_add<float>>
#define BUN_SSE_SUB_SS _sse_array_single<float, 4, _sse_array_sub<float>>
#define BUN_SSE_MUL_SS _sse_array_single<float, 4, _sse_array_mul<float>>
#define BUN_SSE_DIV_SS _sse_array_single<float, 4, _sse_array_div<float>>

BUN_FORCEINLINE float BUN_SSE_SS_F32(BUN_SSE_M128 x) { return x[0]; }

#define BUN_SSE_LOAD_APD(x) BUN_SSE_M128d{(x)[0], (x)[1]}
#define BUN_SSE_LOAD_UPD BUN_SSE_LOAD_APD
#define BUN_SSE_SET_PD(x, y) BUN_SSE_M128d{y, x}
#define BUN_SSE_SET1_PD(x) BUN_SSE_M128d{x, x}
#define BUN_SSE_STORE_APD(v, a) (v)[0] = (a)[0]; (v)[1] = (a)[1];
#define BUN_SSE_STORE_UPD BUN_SSE_STORE_APD
#define BUN_SSE_ADD_PD bun::ArrayMap<double, 2, _sse_array_add<double>>
#define BUN_SSE_SUB_PD bun::ArrayMap<double, 2, _sse_array_sub<double>>
#define BUN_SSE_MUL_PD bun::ArrayMap<double, 2, _sse_array_mul<double>>
#define BUN_SSE_DIV_PD bun::ArrayMap<double, 2, _sse_array_div<double>>
#define BUN_SSE_MIN_PD bun::ArrayMap<double, 2, _sse_array_min<double>>
#define BUN_SSE_MAX_PD bun::ArrayMap<double, 2, _sse_array_max<double>>
#define BUN_SSE_FLOOR_PD _sse_array_unaryop<double, 2, floor>
#define BUN_SSE_CEIL_PD _sse_array_unaryop<double, 2, ceil>
#define BUN_SSE_CMPEQ_PD bun::ArrayMap<double, 2, _sse_array_cmp<double, _sse_array_cmpeq<double>>>
#define BUN_SSE_CMPNEQ_PD bun::ArrayMap<double, 2, _sse_array_cmp<double, _sse_array_cmpneq<double>>>
#define BUN_SSE_CMPLT_PD bun::ArrayMap<double, 2, _sse_array_cmp<double, _sse_array_cmplt<double>>>
#define BUN_SSE_CMPLTE_PD bun::ArrayMap<double, 2, _sse_array_cmp<double, _sse_array_cmple<double>>>
#define BUN_SSE_CMPGT_PD bun::ArrayMap<double, 2, _sse_array_cmp<double, _sse_array_cmpgt<double>>>
#define BUN_SSE_CMPGTE_PD bun::ArrayMap<double, 2, _sse_array_cmp<double, _sse_array_cmpge<double>>>
BUN_FORCEINLINE double BUN_SSE_SD_F64(BUN_SSE_M128d x) { return x[0]; }
#define BUN_SSE_SETZERO_PD() BUN_SSE_M128d{0, 0}

BUN_FORCEINLINE bun_si128 BUN_SSE_LOAD_ASI128(BUN_SSE_M128i* x) { return bun_si128(*x); }
#define BUN_SSE_LOAD_USI128 BUN_SSE_LOAD_ASI128 
#define BUN_SSE_SET_EPI32(a, b, c, d) BUN_SSE_M128i{d, c, b, a}
#define BUN_SSE_SET1_EPI32(a) BUN_SSE_M128i{a, a, a, a}
BUN_FORCEINLINE void BUN_SSE_STORE_ASI128(BUN_SSE_M128i* p, bun_si128 b) { *p = b; }
#define BUN_SSE_STORE_USI128 BUN_SSE_STORE_ASI128 
#define BUN_SSE_ADD_EPI32 bun::ArrayMap<int32_t, 4, _sse_array_add<int32_t>>
#define BUN_SSE_SUB_EPI32 bun::ArrayMap<int32_t, 4, _sse_array_sub<int32_t>>
#define BUN_SSE_MUL_EPI32 bun::ArrayMap<int32_t, 4, _sse_array_mul<int32_t>>
#define BUN_SSE_MIN_EPI32 bun::ArrayMap<int32_t, 4, _sse_array_min<int32_t>>
#define BUN_SSE_MAX_EPI32 bun::ArrayMap<int32_t, 4, _sse_array_max<int32_t>>
#define BUN_SSE_AND(a, b) bun_si128(bun::ArrayMap<int64_t, 2, _sse_array_and<int64_t>>(bun_si128(a), bun_si128(b)))
#define BUN_SSE_ANDNOT(a, b) bun_si128(bun::ArrayMap<int64_t, 2, _sse_array_andnot<int64_t>> (bun_si128(a), bun_si128(b)) )
#define BUN_SSE_OR(a, b) bun_si128(bun::ArrayMap<int64_t, 2, _sse_array_or<int64_t>>(bun_si128(a), bun_si128(b)))
#define BUN_SSE_XOR(a, b) bun_si128(bun::ArrayMap<int64_t, 2, _sse_array_xor<int64_t>>(bun_si128(a), bun_si128(b)))
#define BUN_SSE_SL_EPI32(a, i) _sse_array_shift<int32_t, 4, _sse_array_sl<int32_t>>(a, BUN_SSE_SI128_SI32(i))
#define BUN_SSE_SLI_EPI32 _sse_array_shift<int32_t, 4, _sse_array_sl<int32_t>>
#define BUN_SSE_SR_EPI32(a, i) _sse_array_shift<int32_t, 4, _sse_array_sr<int32_t>>(a, BUN_SSE_SI128_SI32(i))
#define BUN_SSE_SRI_EPI32 _sse_array_shift<int32_t, 4, _sse_array_sr<int32_t>>
#define BUN_SSE_SAR_EPI32(a, i) _sse_array_shift<int32_t, 4, _sse_array_sra<int32_t>>(a, BUN_SSE_SI128_SI32(i))
#define BUN_SSE_SARI_EPI32 _sse_array_shift<int32_t, 4, _sse_array_sra<int32_t>>
#define BUN_SSE_CMPEQ_EPI32 bun::ArrayMap<int32_t, 4, _sse_array_cmp<int32_t, _sse_array_cmpeq<int32_t>>>
#define BUN_SSE_CMPLT_EPI32 bun::ArrayMap<int32_t, 4, _sse_array_cmp<int32_t, _sse_array_cmplt<int32_t>>>
#define BUN_SSE_CMPGT_EPI32 bun::ArrayMap<int32_t, 4, _sse_array_cmp<int32_t, _sse_array_cmpgt<int32_t>>>
BUN_FORCEINLINE int32_t BUN_SSE_SI128_SI32(bun_si128 x) { return (int32_t)x.v[0]; }

#define BUN_SSE_SET_EPI8(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) BUN_SSE_M128i8{p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a}
#define BUN_SSE_SET1_EPI8(a) BUN_SSE_M128i8{a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a}
#define BUN_SSE_ADD_EPI8 bun::ArrayMap<int8_t, 16, _sse_array_add<int8_t>>
#define BUN_SSE_SUB_EPI8 bun::ArrayMap<int8_t, 16, _sse_array_sub<int8_t>>
#define BUN_SSE_CMPEQ_EPI8 bun::ArrayMap<int8_t, 16, _sse_array_cmp<int8_t, _sse_array_cmpeq<int8_t>>>
#define BUN_SSE_CMPLT_EPI8 bun::ArrayMap<int8_t, 16, _sse_array_cmp<int8_t, _sse_array_cmplt<int8_t>>>
#define BUN_SSE_CMPGT_EPI8 bun::ArrayMap<int8_t, 16, _sse_array_cmp<int8_t, _sse_array_cmpgt<int8_t>>>
#define BUN_SSE_SI128_SI8 (char)BUN_SSE_SI128_SI32

#define BUN_SSE_SET_EPI16(a, b, c, d, e, f, g, h) BUN_SSE_M128i16{h, g, f, e, d, c, b, a}
#define BUN_SSE_SET1_EPI16(a) BUN_SSE_M128i16{a, a, a, a, a, a, a, a}
#define BUN_SSE_ADD_EPI16 bun::ArrayMap<int16_t, 8, _sse_array_add<int16_t>>
#define BUN_SSE_SUB_EPI16 bun::ArrayMap<int16_t, 8, _sse_array_sub<int16_t>>
#define BUN_SSE_SL_EPI16(a, i) _sse_array_shift<int16_t, 8, _sse_array_sl<int16_t>>(a, BUN_SSE_SI128_SI32(i)) 
#define BUN_SSE_SLI_EPI16 _sse_array_shift<int16_t, 8, _sse_array_sl<int16_t>>
#define BUN_SSE_SR_EPI16(a, i) _sse_array_shift<int16_t, 8, _sse_array_sr<int16_t>>(a, BUN_SSE_SI128_SI32(i)) 
#define BUN_SSE_SRI_EPI16 _sse_array_shift<int16_t, 8, _sse_array_sr<int16_t>>
#define BUN_SSE_CMPEQ_EPI16 bun::ArrayMap<int16_t, 8, _sse_array_cmp<int16_t, _sse_array_cmpeq<int16_t>>>
#define BUN_SSE_CMPLT_EPI16 bun::ArrayMap<int16_t, 8, _sse_array_cmp<int16_t, _sse_array_cmplt<int16_t>>>
#define BUN_SSE_CMPGT_EPI16 bun::ArrayMap<int16_t, 8, _sse_array_cmp<int16_t, _sse_array_cmpgt<int16_t>>>
#define BUN_SSE_SI128_SI16 (short)BUN_SSE_SI128_SI32

#define BUN_SSE_SET_EPI64(a, b) BUN_SSE_M128i64{b, a}
#define BUN_SSE_SET1_EPI64(a) BUN_SSE_M128i64{a, a}
#define BUN_SSE_ADD_EPI64 bun::ArrayMap<int64_t, 2, _sse_array_add<int64_t>>
#define BUN_SSE_SUB_EPI64 bun::ArrayMap<int64_t, 2, _sse_array_sub<int64_t>>
#define BUN_SSE_SL_EPI64(a, i) _sse_array_shift<int64_t, 2, _sse_array_sl<int64_t>>(a, BUN_SSE_SI128_SI32(i)) 
#define BUN_SSE_SLI_EPI64 _sse_array_shift<int64_t, 2, _sse_array_sl<int64_t>>
#define BUN_SSE_SR_EPI64(a, i) _sse_array_shift<int64_t, 2, _sse_array_sr<int64_t>>(a, BUN_SSE_SI128_SI32(i)) 
#define BUN_SSE_SRI_EPI64 _sse_array_shift<int64_t, 2, _sse_array_sr<int64_t>>
BUN_FORCEINLINE int64_t BUN_SSE_SI128_SI64(bun_si128 x) { return x.v[0]; }

BUN_FORCEINLINE bun_si128 BUN_SSE_SHUFFLE_EPI32(bun_si128 x, int i) { return _sse_array_shuffle4<int32_t>(x, x, i); }
BUN_FORCEINLINE bun_si128 BUN_SSE_SHUFFLEHI_EPI16(bun_si128 x, int i) { return _sse_array_shuffle8<int16_t, 4>(x, i); }
BUN_FORCEINLINE bun_si128 BUN_SSE_SHUFFLELO_EPI16(bun_si128 x, int i) { return _sse_array_shuffle8<int16_t, 0>(x, i); }
#define BUN_SSE_SHUFFLE_PD _sse_array_shuffle2<double>
#define BUN_SSE_SHUFFLE_PS _sse_array_shuffle4<float>
#define BUN_SSE_SETZERO_SI128() bun_si128(std::array<int64_t, 2>{0,0})

#define BUN_SSE_TPS_EPI32 _sse_array_cvt<float, 4, int32_t, 4> //converts using truncation
#define BUN_SSE_TPD_EPI32 _sse_array_cvt<double, 2, int32_t, 4> //converts using truncation
#define BUN_SSE_PS_EPI32 _sse_array_cvt<float, 4, int32_t, 4> // This should convert using rounding but there is no easy way to do this if SSE instructions aren't available, so we just don't.
#define BUN_SSE_PD_EPI32 _sse_array_cvt<double, 2, int32_t, 4>
#define BUN_SSE_EPI32_PS _sse_array_cvt<int32_t, 4, float, 4>
#define BUN_SSE_PD_PS _sse_array_cvt<double, 2, float, 4> 

BUN_FORCEINLINE BUN_SSE_M128 BUN_SSE_CAST_PD_PS(BUN_SSE_M128d x) { return *reinterpret_cast<BUN_SSE_M128*>(&x); }
BUN_FORCEINLINE BUN_SSE_M128i BUN_SSE_CAST_PD_SI128(BUN_SSE_M128d x) { return *reinterpret_cast<BUN_SSE_M128i*>(&x); }
BUN_FORCEINLINE BUN_SSE_M128d BUN_SSE_CAST_PS_PD(BUN_SSE_M128 x) { return *reinterpret_cast<BUN_SSE_M128d*>(&x); }
BUN_FORCEINLINE BUN_SSE_M128i BUN_SSE_CAST_PS_SI128(BUN_SSE_M128 x) { return *reinterpret_cast<BUN_SSE_M128i*>(&x); }
BUN_FORCEINLINE BUN_SSE_M128d BUN_SSE_CAST_SI128_PD(BUN_SSE_M128i x) { return *reinterpret_cast<BUN_SSE_M128d*>(&x); }
BUN_FORCEINLINE BUN_SSE_M128 BUN_SSE_CAST_SI128_PS(BUN_SSE_M128i x) { return *reinterpret_cast<BUN_SSE_M128*>(&x); }

#else
#define BUN_SSE_M128 __m128
#define BUN_SSE_M128i8 __m128i
#define BUN_SSE_M128i16 __m128i
#define BUN_SSE_M128i __m128i
#define BUN_SSE_M128i64 __m128i
#define BUN_SSE_M128d __m128d

// Define compiler-specific intrinsics for working with SSE.
#define BUN_SSE_LOAD_APS _mm_load_ps
#define BUN_SSE_LOAD_UPS _mm_loadu_ps
#define BUN_SSE_SET_PS _mm_set_ps
#define BUN_SSE_SET1_PS _mm_set_ps1
#define BUN_SSE_STORE_APS _mm_store_ps
#define BUN_SSE_STORE_UPS _mm_storeu_ps
#define BUN_SSE_ADD_PS _mm_add_ps
#define BUN_SSE_SUB_PS _mm_sub_ps
#define BUN_SSE_MUL_PS _mm_mul_ps
#define BUN_SSE_DIV_PS _mm_div_ps
#define BUN_SSE_MIN_PS bun_mm_min_ps
#define BUN_SSE_MAX_PS bun_mm_max_ps
#define BUN_SSE_FLOOR_PS bun_mm_floor_ps
#define BUN_SSE_CEIL_PS bun_mm_ceil_ps
#define BUN_SSE_CMPEQ_PS _mm_cmpeq_ps
#define BUN_SSE_CMPNEQ_PS _mm_cmpneq_ps
#define BUN_SSE_CMPLT_PS _mm_cmplt_ps
#define BUN_SSE_CMPLTE_PS _mm_cmple_ps
#define BUN_SSE_CMPGT_PS _mm_cmpgt_ps
#define BUN_SSE_CMPGTE_PS _mm_cmpge_ps
#define BUN_SSE_SETZERO_PS _mm_setzero_ps
#define BUN_SSE_MOVEHL_PS _mm_movehl_ps
#define BUN_SSE_MOVELH_PS _mm_movelh_ps
#define BUN_SSE_ADD_SS _mm_add_ss
#define BUN_SSE_SUB_SS _mm_sub_ss
#define BUN_SSE_MUL_SS _mm_mul_ss
#define BUN_SSE_DIV_SS _mm_div_ss

#define BUN_SSE_SS_F32 _mm_cvtss_f32

#define BUN_SSE_LOAD_APD _mm_load_pd
#define BUN_SSE_LOAD_UPD _mm_loadu_pd
#define BUN_SSE_SET_PD _mm_set_pd
#define BUN_SSE_SET1_PD _mm_set1_pd
#define BUN_SSE_STORE_APD _mm_store_pd
#define BUN_SSE_STORE_UPD _mm_storeu_pd
#define BUN_SSE_ADD_PD _mm_add_pd
#define BUN_SSE_SUB_PD _mm_sub_pd
#define BUN_SSE_MUL_PD _mm_mul_pd
#define BUN_SSE_DIV_PD _mm_div_pd
//#define BUN_SSE_MIN_PD _mm_min_pd // These are apparently SSE4.1
//#define BUN_SSE_MAX_PD _mm_max_pd
#define BUN_SSE_MIN_PD bun_mm_min_pd
#define BUN_SSE_MAX_PD bun_mm_max_pd
#define BUN_SSE_CMPEQ_PD _mm_cmpeq_pd
#define BUN_SSE_CMPNEQ_PD _mm_cmpneq_pd
#define BUN_SSE_CMPLT_PD _mm_cmplt_pd
#define BUN_SSE_CMPLTE_PD _mm_cmple_pd
#define BUN_SSE_CMPGT_PD _mm_cmpgt_pd
#define BUN_SSE_CMPGTE_PD _mm_cmpge_pd
#define BUN_SSE_SD_F64 _mm_cvtsd_f64
#define BUN_SSE_SETZERO_PD _mm_setzero_pd
#define BUN_SSE_ADD_SD _mm_add_sd
#define BUN_SSE_SUB_SD _mm_sub_sd
#define BUN_SSE_MUL_SD _mm_mul_sd
#define BUN_SSE_DIV_SD _mm_div_sd

#define BUN_SSE_LOAD_ASI128 _mm_load_si128
#define BUN_SSE_LOAD_USI128 _mm_loadu_si128 
#define BUN_SSE_SET_EPI32 _mm_set_epi32
#define BUN_SSE_SET1_EPI32 _mm_set1_epi32
#define BUN_SSE_STORE_ASI128 _mm_store_si128 
#define BUN_SSE_STORE_USI128 _mm_storeu_si128 
#define BUN_SSE_ADD_EPI32 _mm_add_epi32
#define BUN_SSE_SUB_EPI32 _mm_sub_epi32
#define BUN_SSE_MUL_EPI32 bun_mm_mul_epi32
#define BUN_SSE_MIN_EPI32 bun_mm_min_epi32
#define BUN_SSE_MAX_EPI32 bun_mm_max_epi32
#define BUN_SSE_AND _mm_and_si128 
#define BUN_SSE_ANDNOT _mm_andnot_si128 
#define BUN_SSE_OR _mm_or_si128 
#define BUN_SSE_XOR _mm_xor_si128
#define BUN_SSE_SL_EPI32 _mm_sll_epi32
#define BUN_SSE_SLI_EPI32 _mm_slli_epi32
#define BUN_SSE_SR_EPI32 _mm_srl_epi32
#define BUN_SSE_SRI_EPI32 _mm_srli_epi32
#define BUN_SSE_SAR_EPI32 _mm_sra_epi32 //Arithmetic right shift
#define BUN_SSE_SARI_EPI32 _mm_srai_epi32
#define BUN_SSE_CMPEQ_EPI32 _mm_cmpeq_epi32
#define BUN_SSE_CMPLT_EPI32 _mm_cmplt_epi32
#define BUN_SSE_CMPGT_EPI32 _mm_cmpgt_epi32
#define BUN_SSE_SI128_SI32 _mm_cvtsi128_si32

#define BUN_SSE_SET_EPI8 _mm_set_epi8
#define BUN_SSE_SET1_EPI8 _mm_set1_epi8
#define BUN_SSE_ADD_EPI8 _mm_add_epi8
#define BUN_SSE_SUB_EPI8 _mm_sub_epi8
#define BUN_SSE_CMPEQ_EPI8 _mm_cmpeq_epi8
#define BUN_SSE_CMPLT_EPI8 _mm_cmplt_epi8
#define BUN_SSE_CMPGT_EPI8 _mm_cmpgt_epi8
#define BUN_SSE_SI128_SI8 (char)_mm_cvtsi128_si32

#define BUN_SSE_SET_EPI16 _mm_set_epi16
#define BUN_SSE_SET1_EPI16 _mm_set1_epi16
#define BUN_SSE_ADD_EPI16 _mm_add_epi16
#define BUN_SSE_SUB_EPI16 _mm_sub_epi16
#define BUN_SSE_SL_EPI16 _mm_sll_epi16
#define BUN_SSE_SLI_EPI16 _mm_slli_epi16
#define BUN_SSE_SR_EPI16 _mm_srl_epi16
#define BUN_SSE_SRI_EPI16 _mm_srli_epi16
#define BUN_SSE_CMPEQ_EPI16 _mm_cmpeq_epi16
#define BUN_SSE_CMPLT_EPI16 _mm_cmplt_epi16
#define BUN_SSE_CMPGT_EPI16 _mm_cmpgt_epi16
#define BUN_SSE_SI128_SI16 (short)_mm_cvtsi128_si32

BUN_FORCEINLINE __m128i BUN_SSE_SET_EPI64(int64_t y, int64_t x) { BUN_ALIGN(16) int64_t vv[2] = { x, y }; return BUN_SSE_LOAD_ASI128((BUN_SSE_M128i*)vv); }
#define BUN_SSE_SET1_EPI64(x) BUN_SSE_SET_EPI64(x, x)
#define BUN_SSE_ADD_EPI64 _mm_add_epi64
#define BUN_SSE_SUB_EPI64 _mm_sub_epi64
#define BUN_SSE_SL_EPI64 _mm_sll_epi64
#define BUN_SSE_SLI_EPI64 _mm_slli_epi64
#define BUN_SSE_SR_EPI64 _mm_srl_epi64
#define BUN_SSE_SRI_EPI64 _mm_srli_epi64
#define BUN_SSE_SI128_SI64 _mm_cvtsi128_si64 //64-bit architectures only

#define BUN_SSE_SHUFFLE_EPI32 _mm_shuffle_epi32
#define BUN_SSE_SHUFFLEHI_EPI16 _mm_shufflehi_epi16
#define BUN_SSE_SHUFFLELO_EPI16 _mm_shufflelo_epi16
#define BUN_SSE_SHUFFLE_PD _mm_shuffle_pd
#define BUN_SSE_SHUFFLE_PS _mm_shuffle_ps
#define BUN_SSE_SETZERO_SI128 _mm_setzero_si128

#define BUN_SSE_TPS_EPI32 _mm_cvttps_epi32 //converts using truncation
#define BUN_SSE_TPD_EPI32 _mm_cvttpd_epi32 //converts using truncation
#define BUN_SSE_PS_EPI32 _mm_cvtps_epi32
#define BUN_SSE_PD_EPI32 _mm_cvtpd_epi32
#define BUN_SSE_EPI32_PS _mm_cvtepi32_ps
#define BUN_SSE_PD_PS _mm_cvtpd_ps 

#define BUN_SSE_CAST_PD_PS _mm_castpd_ps
#define BUN_SSE_CAST_PD_SI128 _mm_castpd_si128
#define BUN_SSE_CAST_PS_PD _mm_castps_pd
#define BUN_SSE_CAST_PS_SI128 _mm_castps_si128
#define BUN_SSE_CAST_SI128_PD _mm_castsi128_pd
#define BUN_SSE_CAST_SI128_PS _mm_castsi128_ps

// SSE2 does not have min or max, so we use this manual implementation of the instruction
BUN_FORCEINLINE BUN_SSE_M128i bun_mm_min_epi32(BUN_SSE_M128i a, BUN_SSE_M128i b)
{
  BUN_SSE_M128i mask = BUN_SSE_CMPLT_EPI32(a, b);
  a = BUN_SSE_AND(a, mask);
  b = BUN_SSE_ANDNOT(mask, b);
  return BUN_SSE_OR(a, b);
}

BUN_FORCEINLINE BUN_SSE_M128i bun_mm_max_epi32(BUN_SSE_M128i a, BUN_SSE_M128i b)
{
  BUN_SSE_M128i mask = BUN_SSE_CMPGT_EPI32(a, b);
  a = BUN_SSE_AND(a, mask);
  b = BUN_SSE_ANDNOT(mask, b);
  return BUN_SSE_OR(a, b);
}

BUN_FORCEINLINE BUN_SSE_M128 bun_mm_min_ps(BUN_SSE_M128 a, BUN_SSE_M128 b)
{
  BUN_SSE_M128i mask = _mm_castps_si128(BUN_SSE_CMPLT_PS(a, b));
  BUN_SSE_M128i c = BUN_SSE_AND(_mm_castps_si128(a), mask);
  BUN_SSE_M128i d = BUN_SSE_ANDNOT(mask, _mm_castps_si128(b));
  return _mm_castsi128_ps(BUN_SSE_OR(c, d));
}

BUN_FORCEINLINE BUN_SSE_M128 bun_mm_max_ps(BUN_SSE_M128 a, BUN_SSE_M128 b)
{
  BUN_SSE_M128i mask = _mm_castps_si128(BUN_SSE_CMPGT_PS(a, b));
  BUN_SSE_M128i c = BUN_SSE_AND(_mm_castps_si128(a), mask);
  BUN_SSE_M128i d = BUN_SSE_ANDNOT(mask, _mm_castps_si128(b));
  return _mm_castsi128_ps(BUN_SSE_OR(c, d));
}

BUN_FORCEINLINE BUN_SSE_M128d bun_mm_min_pd(BUN_SSE_M128d a, BUN_SSE_M128d b)
{
  BUN_SSE_M128i mask = _mm_castpd_si128(BUN_SSE_CMPLT_PD(a, b));
  BUN_SSE_M128i c = BUN_SSE_AND(_mm_castpd_si128(a), mask);
  BUN_SSE_M128i d = BUN_SSE_ANDNOT(mask, _mm_castpd_si128(b));
  return _mm_castsi128_pd(BUN_SSE_OR(c, d));
}

BUN_FORCEINLINE BUN_SSE_M128d bun_mm_max_pd(BUN_SSE_M128d a, BUN_SSE_M128d b)
{
  BUN_SSE_M128i mask = _mm_castpd_si128(BUN_SSE_CMPGT_PD(a, b));
  BUN_SSE_M128i c = BUN_SSE_AND(_mm_castpd_si128(a), mask);
  BUN_SSE_M128i d = BUN_SSE_ANDNOT(mask, _mm_castpd_si128(b));
  return _mm_castsi128_pd(BUN_SSE_OR(c, d));
}

BUN_FORCEINLINE BUN_SSE_M128 bun_mm_floor_ps(BUN_SSE_M128 x)
{
  __m128i v0 = _mm_setzero_si128();
  __m128i v1 = _mm_cmpeq_epi32(v0, v0);
  __m128i ji = _mm_srli_epi32(v1, 25);
  __m128 j = _mm_castsi128_ps(_mm_slli_epi32(ji, 23)); //create vector 1.0f
  __m128i i = _mm_cvttps_epi32(x);
  __m128 fi = _mm_cvtepi32_ps(i);
  __m128 igx = _mm_cmpgt_ps(fi, x);
  j = _mm_and_ps(igx, j);
  return _mm_sub_ps(fi, j);
}

BUN_FORCEINLINE BUN_SSE_M128 bun_mm_ceil_ps(BUN_SSE_M128 x)
{
  __m128i v0 = _mm_setzero_si128();
  __m128i v1 = _mm_cmpeq_epi32(v0, v0);
  __m128i ji = _mm_srli_epi32(v1, 25);
  __m128 j = _mm_castsi128_ps(_mm_slli_epi32(ji, 23)); //create vector 1.0f
  __m128i i = _mm_cvttps_epi32(x);
  __m128 fi = _mm_cvtepi32_ps(i);
  __m128 igx = _mm_cmplt_ps(fi, x);
  j = _mm_and_ps(igx, j);
  return _mm_add_ps(fi, j);
}

// SSE2 does not have any way to multiply all four 32bit integer components
BUN_FORCEINLINE BUN_SSE_M128i bun_mm_mul_epi32(BUN_SSE_M128i a, BUN_SSE_M128i b)
{
  __m128i tmp1 = _mm_mul_epu32(a, b); /* mul 2,0*/
  __m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(a, 4), _mm_srli_si128(b, 4)); /* mul 3,1 */
  return _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 2, 0))); /* Shuffle results to [63..0] and pack */
}

#endif

#define BUN_SSE_SHUFFLE(x,y,z,w) ((w<<6) | (z<<4) | (y<<2) | x)

// Struct that wraps around a pointer to signify that it is not aligned
template<typename T>
struct BUN_UNALIGNED {
  inline explicit BUN_UNALIGNED(T* p) : _p(p) {}
  T* _p;
};

// Struct that abstracts out a lot of common SSE operations
template<typename T> struct sseVecT {};

// 32-bit floating point operations
template<>
BUN_ALIGNED_STRUCT(16) sseVecT<float>
{
  BUN_FORCEINLINE sseVecT(BUN_SSE_M128 v) : xmm(v) {}
  BUN_FORCEINLINE explicit sseVecT(BUN_SSE_M128i v) : xmm(BUN_SSE_EPI32_PS(v)) {}
  BUN_FORCEINLINE explicit sseVecT(BUN_SSE_M128d v) : xmm(BUN_SSE_PD_PS(v)) {}
  BUN_FORCEINLINE sseVecT(float v) : xmm(BUN_SSE_SET1_PS(v)) {}
  //BUN_FORCEINLINE sseVecT<float>(const float*BUN_RESTRICT v) : xmm(BUN_SSE_LOAD_APS(v)) { assert(!(((size_t)v)%16)); }
  BUN_FORCEINLINE explicit sseVecT(const float(&v)[4]) : xmm(BUN_SSE_LOAD_APS(v)) { assert(!(((size_t)v) % 16)); }
  BUN_FORCEINLINE explicit sseVecT(BUN_UNALIGNED<const float> v) : xmm(BUN_SSE_LOAD_UPS(v._p)) {}
  BUN_FORCEINLINE sseVecT(float x, float y, float z, float w) : xmm(BUN_SSE_SET_PS(w, z, y, x)) {}
  BUN_FORCEINLINE sseVecT<float> operator+(const sseVecT<float>& r) const { return sseVecT<float>(BUN_SSE_ADD_PS(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<float> operator-(const sseVecT<float>& r) const { return sseVecT<float>(BUN_SSE_SUB_PS(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<float> operator*(const sseVecT<float>& r) const { return sseVecT<float>(BUN_SSE_MUL_PS(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<float> operator/(const sseVecT<float>& r) const { return sseVecT<float>(BUN_SSE_DIV_PS(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<float>& operator+=(const sseVecT<float>& r) { xmm = BUN_SSE_ADD_PS(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<float>& operator-=(const sseVecT<float>& r) { xmm = BUN_SSE_SUB_PS(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<float>& operator*=(const sseVecT<float>& r) { xmm = BUN_SSE_MUL_PS(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<float>& operator/=(const sseVecT<float>& r) { xmm = BUN_SSE_DIV_PS(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE operator BUN_SSE_M128() const { return xmm; }
  //BUN_FORCEINLINE void Set(float*BUN_RESTRICT v) { assert(!(((size_t)v)%16)); BUN_SSE_STORE_APS(v, xmm); }
  BUN_FORCEINLINE void Set(float(&v)[4]) const { assert(!(((size_t)v) % 16)); BUN_SSE_STORE_APS(v, xmm); }
  BUN_FORCEINLINE void Set(BUN_UNALIGNED<float> v) const { BUN_SSE_STORE_UPS(v._p, xmm); }
  BUN_FORCEINLINE void Set(float& v) const { v = BUN_SSE_SS_F32(xmm); }
  BUN_FORCEINLINE sseVecT<float>& operator=(BUN_SSE_M128 v) { xmm = v; return *this; }
  BUN_FORCEINLINE sseVecT<float>& operator=(float v) { xmm = BUN_SSE_SET1_PS(v); return *this; }
  BUN_FORCEINLINE sseVecT<float>& operator=(const float(&v)[4]) { xmm = BUN_SSE_LOAD_APS(v); assert(!(((size_t)v) % 16)); return *this; }
  BUN_FORCEINLINE sseVecT<float>& operator=(BUN_UNALIGNED<const float> v) { xmm = BUN_SSE_LOAD_UPS(v._p); return *this; }

  BUN_FORCEINLINE sseVecT<float> floor() const { return sseVecT<float>(BUN_SSE_FLOOR_PS(xmm)); }
  BUN_FORCEINLINE sseVecT<float> ceil() const { return sseVecT<float>(BUN_SSE_CEIL_PS(xmm)); }
  BUN_FORCEINLINE sseVecT<float> min(const sseVecT<float>& r) const { return sseVecT<float>(BUN_SSE_MIN_PS(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<float> max(const sseVecT<float>& r) const { return sseVecT<float>(BUN_SSE_MAX_PS(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<float> operator==(const sseVecT<float>& r) const { return sseVecT<float>(BUN_SSE_CMPEQ_PS(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<float> operator!=(const sseVecT<float>& r) const { return sseVecT<float>(BUN_SSE_CMPNEQ_PS(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<float> operator<(const sseVecT<float>& r) const { return sseVecT<float>(BUN_SSE_CMPLT_PS(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<float> operator<=(const sseVecT<float>& r) const { return sseVecT<float>(BUN_SSE_CMPLTE_PS(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<float> operator>(const sseVecT<float>& r) const { return sseVecT<float>(BUN_SSE_CMPGT_PS(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<float> operator>=(const sseVecT<float>& r) const { return sseVecT<float>(BUN_SSE_CMPGTE_PS(xmm, r.xmm)); }
  template<int x, int y, int z, int w>
  BUN_FORCEINLINE sseVecT<float> Shuffle() const { return Shuffle<BUN_SSE_SHUFFLE(x, y, z, w)>(*this); }
  template<int Positions>
  static sseVecT<float> Shuffle(const sseVecT<float>& x) { return BUN_SSE_CAST_SI128_PS(BUN_SSE_SHUFFLE_EPI32(BUN_SSE_CAST_PS_SI128(x), Positions)); }
  static sseVecT<float> ZeroVector() { return sseVecT<float>(BUN_SSE_SETZERO_PS()); }
  BUN_FORCEINLINE float Sum() const { sseVecT<float> x = BUN_SSE_ADD_PS(xmm, Shuffle<1, 0, 3, 2>()); return BUN_SSE_SS_F32(BUN_SSE_ADD_PS(x, x.Shuffle<2, 3, 0, 1>())); }

  BUN_SSE_M128 xmm;
};

// 64-bit double operations
template<>
BUN_ALIGNED_STRUCT(16) sseVecT<double>
{
  BUN_FORCEINLINE sseVecT(BUN_SSE_M128d v) : xmm(v) {}
  BUN_FORCEINLINE sseVecT(double v) : xmm(BUN_SSE_SET1_PD(v)) {}
  //BUN_FORCEINLINE sseVec(const double*BUN_RESTRICT v) : xmm(BUN_SSE_LOAD_APD(v)) { assert(!(((size_t)v)%16)); }
  BUN_FORCEINLINE explicit sseVecT(const double(&v)[2]) : xmm(BUN_SSE_LOAD_APD(v)) { assert(!(((size_t)v) % 16)); }
  BUN_FORCEINLINE explicit sseVecT(BUN_UNALIGNED<const double> v) : xmm(BUN_SSE_LOAD_UPD(v._p)) {}
  BUN_FORCEINLINE sseVecT(double x, double y) : xmm(BUN_SSE_SET_PD(y, x)) {}
  BUN_FORCEINLINE sseVecT<double> operator+(const sseVecT<double>& r) const { return sseVecT<double>(BUN_SSE_ADD_PD(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<double> operator-(const sseVecT<double>& r) const { return sseVecT<double>(BUN_SSE_SUB_PD(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<double> operator*(const sseVecT<double>& r) const { return sseVecT<double>(BUN_SSE_MUL_PD(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<double> operator/(const sseVecT<double>& r) const { return sseVecT<double>(BUN_SSE_DIV_PD(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<double>& operator+=(const sseVecT<double>& r) { xmm = BUN_SSE_ADD_PD(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<double>& operator-=(const sseVecT<double>& r) { xmm = BUN_SSE_SUB_PD(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<double>& operator*=(const sseVecT<double>& r) { xmm = BUN_SSE_MUL_PD(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<double>& operator/=(const sseVecT<double>& r) { xmm = BUN_SSE_DIV_PD(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE operator BUN_SSE_M128d() const { return xmm; }
  //BUN_FORCEINLINE void Set(double*BUN_RESTRICT v) { assert(!(((size_t)v)%16)); BUN_SSE_STORE_APD(v, xmm); }
  BUN_FORCEINLINE void Set(double(&v)[2]) const { assert(!(((size_t)v) % 16)); BUN_SSE_STORE_APD(v, xmm); }
  BUN_FORCEINLINE void Set(BUN_UNALIGNED<double> v) const { BUN_SSE_STORE_UPD(v._p, xmm); }
  BUN_FORCEINLINE void Set(double& v) const { v = BUN_SSE_SD_F64(xmm); }
  BUN_FORCEINLINE sseVecT<double>& operator=(BUN_SSE_M128d v) { xmm = v; return *this; }
  BUN_FORCEINLINE sseVecT<double>& operator=(double v) { xmm = BUN_SSE_SET1_PD(v); return *this; }
  BUN_FORCEINLINE sseVecT<double>& operator=(const double(&v)[2]) { xmm = BUN_SSE_LOAD_APD(v); assert(!(((size_t)v) % 16)); return *this; }
  BUN_FORCEINLINE sseVecT<double>& operator=(BUN_UNALIGNED<const double> v) { xmm = BUN_SSE_LOAD_UPD(v._p); return *this; }

  BUN_FORCEINLINE sseVecT<double> min(const sseVecT<double>& r) const { return sseVecT<double>(BUN_SSE_MIN_PD(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<double> max(const sseVecT<double>& r) const { return sseVecT<double>(BUN_SSE_MAX_PD(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<double> operator==(const sseVecT<double>& r) const { return sseVecT<double>(BUN_SSE_CMPEQ_PD(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<double> operator!=(const sseVecT<double>& r) const { return sseVecT<double>(BUN_SSE_CMPNEQ_PD(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<double> operator<(const sseVecT<double>& r) const { return sseVecT<double>(BUN_SSE_CMPLT_PD(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<double> operator<=(const sseVecT<double>& r) const { return sseVecT<double>(BUN_SSE_CMPLTE_PD(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<double> operator>(const sseVecT<double>& r) const { return sseVecT<double>(BUN_SSE_CMPGT_PD(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<double> operator>=(const sseVecT<double>& r) const { return sseVecT<double>(BUN_SSE_CMPGTE_PD(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<double> Swap() const { return BUN_SSE_SHUFFLE_PD(xmm, xmm, 1); }

  template<int Positions>
  static sseVecT<double> Shuffle(const sseVecT<double>& x) { return BUN_SSE_CAST_SI128_PD(BUN_SSE_SHUFFLE_EPI32(BUN_SSE_CAST_PD_SI128(x), Positions)); }
  static sseVecT<double> ZeroVector() { return sseVecT<double>(BUN_SSE_SETZERO_PD()); }
  BUN_FORCEINLINE double Sum() const { return BUN_SSE_SD_F64(BUN_SSE_ADD_PD(xmm, Swap())); }

  BUN_SSE_M128d xmm;
};

// 8-bit signed integer operations
template<>
BUN_ALIGNED_STRUCT(16) sseVecT<char>
{
  BUN_FORCEINLINE sseVecT(BUN_SSE_M128i8 v) : xmm(v) {} //__fastcall is obviously useless here since we're dealing with xmm registers
  BUN_FORCEINLINE sseVecT(char v) : xmm(BUN_SSE_SET1_EPI8(v)) {}
  BUN_FORCEINLINE explicit sseVecT(const char(&v)[16]) : xmm(BUN_SSE_LOAD_ASI128((BUN_SSE_M128i*)v)) { assert(!(((size_t)v) % 16)); }
  BUN_FORCEINLINE explicit sseVecT(BUN_UNALIGNED<const char> v) : xmm(BUN_SSE_LOAD_USI128((BUN_SSE_M128i*)v._p)) {}
  BUN_FORCEINLINE sseVecT(char b1, char b2, char b3, char b4, char b5, char b6, char b7, char b8, char b9, char b10,
    char b11, char b12, char b13, char b14, char b15, char b16) : // This is the craziest constructor definition ever
    xmm(BUN_SSE_SET_EPI8(b16, b15, b14, b13, b12, b11, b10, b9, b8, b7, b6, b5, b4, b3, b2, b1))
  {}
  BUN_FORCEINLINE sseVecT<char> operator+(const sseVecT<char>& r) const { return sseVecT<char>(BUN_SSE_ADD_EPI8(xmm, r.xmm)); } // These don't return const sseVecT<char> because it makes things messy.
  BUN_FORCEINLINE sseVecT<char> operator-(const sseVecT<char>& r) const { return sseVecT<char>(BUN_SSE_SUB_EPI8(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<char> operator&(const sseVecT<char>& r) const { return sseVecT<char>(BUN_SSE_AND(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<char> operator|(const sseVecT<char>& r) const { return sseVecT<char>(BUN_SSE_OR(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<char> operator^(const sseVecT<char>& r) const { return sseVecT<char>(BUN_SSE_XOR(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<char>& operator+=(const sseVecT<char>& r) { xmm = BUN_SSE_ADD_EPI8(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<char>& operator-=(const sseVecT<char>& r) { xmm = BUN_SSE_SUB_EPI8(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<char>& operator&=(const sseVecT<char>& r) { xmm = BUN_SSE_AND(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<char>& operator|=(const sseVecT<char>& r) { xmm = BUN_SSE_OR(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<char>& operator^=(const sseVecT<char>& r) { xmm = BUN_SSE_XOR(xmm, r.xmm); return *this; }

#ifndef BUN_SSE_ENABLED
  BUN_FORCEINLINE operator bun_si128() const { return xmm; }
#endif
  BUN_FORCEINLINE operator BUN_SSE_M128i8() const { return xmm; }
  BUN_FORCEINLINE sseVecT<char> operator==(const sseVecT<char>& r) const { return sseVecT<char>(BUN_SSE_CMPEQ_EPI8(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<char> operator!=(const sseVecT<char>& r) const { return !operator==(r); }
  BUN_FORCEINLINE sseVecT<char> operator<(const sseVecT<char>& r) const { return sseVecT<char>(BUN_SSE_CMPLT_EPI8(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<char> operator<=(const sseVecT<char>& r) const { return !operator>(r); }
  BUN_FORCEINLINE sseVecT<char> operator>(const sseVecT<char>& r) const { return sseVecT<char>(BUN_SSE_CMPGT_EPI8(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<char> operator>=(const sseVecT<char>& r) const { return !operator<(r); }
  BUN_FORCEINLINE sseVecT<char> operator~() const { return sseVecT<char>(BUN_SSE_XOR(xmm, BUN_SSE_SET1_EPI8(-1))); }
  BUN_FORCEINLINE sseVecT<char> operator!() const { return sseVecT<char>(BUN_SSE_CMPEQ_EPI8(xmm, ZeroVector())); }
  BUN_FORCEINLINE sseVecT<char>& operator=(BUN_SSE_M128i8 r) { xmm = r; return *this; }
  BUN_FORCEINLINE sseVecT<char>& operator=(char v) { xmm = BUN_SSE_SET1_EPI8(v); return *this; }
  BUN_FORCEINLINE sseVecT<char>& operator=(const char(&v)[16]) { xmm = BUN_SSE_LOAD_ASI128((BUN_SSE_M128i*)v); assert(!(((size_t)v) % 16)); return *this; }
  BUN_FORCEINLINE sseVecT<char>& operator=(BUN_UNALIGNED<const char> v) { xmm = BUN_SSE_LOAD_USI128((BUN_SSE_M128i*)v._p); return *this; }

  BUN_FORCEINLINE void Set(char(&v)[16]) const { assert(!(((size_t)v) % 16)); BUN_SSE_STORE_ASI128((BUN_SSE_M128i*)v, xmm); }
  BUN_FORCEINLINE void Set(BUN_UNALIGNED<char> v) const { BUN_SSE_STORE_USI128((BUN_SSE_M128i*)v._p, xmm); }
  BUN_FORCEINLINE void Set(char& v) const { v = BUN_SSE_SI128_SI8(xmm); }
  static sseVecT<char> ZeroVector() { return sseVecT<char>(BUN_SSE_SETZERO_SI128()); }
  template<int Positions>
  static sseVecT<char> Shuffle(const sseVecT<char>& x) { return BUN_SSE_SHUFFLE_EPI32(x, Positions); }
  template<int Positions>
  static sseVecT<char> ShuffleHi(const sseVecT<char>& x) { return BUN_SSE_SHUFFLEHI_EPI16(x, Positions); }
  template<int Positions>
  static sseVecT<char> ShuffleLo(const sseVecT<char>& x) { return BUN_SSE_SHUFFLELO_EPI16(x, Positions); }

  BUN_SSE_M128i8 xmm;
};

// 16-bit signed integer operations
template<>
BUN_ALIGNED_STRUCT(16) sseVecT<int16_t>
{
  BUN_FORCEINLINE sseVecT(BUN_SSE_M128i16 v) : xmm(v) {} //__fastcall is obviously useless here since we're dealing with xmm registers
  BUN_FORCEINLINE sseVecT(int16_t v) : xmm(BUN_SSE_SET1_EPI16(v)) {}
  //BUN_FORCEINLINE sseVecT<int16_t>(const int*BUN_RESTRICT v) : xmm(BUN_SSE_LOAD_ASI128(v)) { assert(!(((size_t)v)%16)); }
  BUN_FORCEINLINE explicit sseVecT(const int16_t(&v)[8]) : xmm(BUN_SSE_LOAD_ASI128((BUN_SSE_M128i*)v)) { assert(!(((size_t)v) % 16)); }
  BUN_FORCEINLINE explicit sseVecT(BUN_UNALIGNED<const int16_t> v) : xmm(BUN_SSE_LOAD_USI128((BUN_SSE_M128i*)v._p)) {}
  BUN_FORCEINLINE sseVecT(int16_t s, int16_t t, int16_t u, int16_t v, int16_t w, int16_t x, int16_t y, int16_t z) : xmm(BUN_SSE_SET_EPI16(z, y, x, w, v, u, t, s)) {}
  BUN_FORCEINLINE sseVecT<int16_t> operator+(const sseVecT<int16_t>& r) const { return sseVecT<int16_t>(BUN_SSE_ADD_EPI16(xmm, r.xmm)); } // These don't return const sseVecT<int16_t> because it makes things messy.
  BUN_FORCEINLINE sseVecT<int16_t> operator-(const sseVecT<int16_t>& r) const { return sseVecT<int16_t>(BUN_SSE_SUB_EPI16(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int16_t> operator&(const sseVecT<int16_t>& r) const { return sseVecT<int16_t>(BUN_SSE_AND(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int16_t> operator|(const sseVecT<int16_t>& r) const { return sseVecT<int16_t>(BUN_SSE_OR(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int16_t> operator^(const sseVecT<int16_t>& r) const { return sseVecT<int16_t>(BUN_SSE_XOR(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int16_t> operator>>(const sseVecT<int16_t>& r) const { return sseVecT<int16_t>(BUN_SSE_SR_EPI16(xmm, r.xmm)); } // Amazingly enough this does not conflict with the extraction operator
  BUN_FORCEINLINE sseVecT<int16_t> operator>>(int r) const { return sseVecT<int16_t>(BUN_SSE_SRI_EPI16(xmm, r)); }
  BUN_FORCEINLINE sseVecT<int16_t> operator<<(const sseVecT<int16_t>& r) const { return sseVecT<int16_t>(BUN_SSE_SL_EPI16(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int16_t> operator<<(int r) const { return sseVecT<int16_t>(BUN_SSE_SLI_EPI16(xmm, r)); }
  BUN_FORCEINLINE sseVecT<int16_t>& operator+=(const sseVecT<int16_t>& r) { xmm = BUN_SSE_ADD_EPI16(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<int16_t>& operator-=(const sseVecT<int16_t>& r) { xmm = BUN_SSE_SUB_EPI16(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<int16_t>& operator&=(const sseVecT<int16_t>& r) { xmm = BUN_SSE_AND(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<int16_t>& operator|=(const sseVecT<int16_t>& r) { xmm = BUN_SSE_OR(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<int16_t>& operator^=(const sseVecT<int16_t>& r) { xmm = BUN_SSE_XOR(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<int16_t>& operator>>=(const sseVecT<int16_t>& r) { xmm = BUN_SSE_SR_EPI16(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<int16_t>& operator>>=(int r) { xmm = BUN_SSE_SRI_EPI16(xmm, r); return *this; }
  BUN_FORCEINLINE sseVecT<int16_t>& operator<<=(const sseVecT<int16_t>& r) { xmm = BUN_SSE_SL_EPI16(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<int16_t>& operator<<=(int r) { xmm = BUN_SSE_SLI_EPI16(xmm, r); return *this; }

#ifndef BUN_SSE_ENABLED
  BUN_FORCEINLINE operator bun_si128() const { return xmm; }
#endif
  BUN_FORCEINLINE operator BUN_SSE_M128i16() const { return xmm; }
  BUN_FORCEINLINE sseVecT<int16_t> operator==(const sseVecT<int16_t>& r) const { return sseVecT<int16_t>(BUN_SSE_CMPEQ_EPI16(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int16_t> operator!=(const sseVecT<int16_t>& r) const { return !operator==(r); }
  BUN_FORCEINLINE sseVecT<int16_t> operator<(const sseVecT<int16_t>& r) const { return sseVecT<int16_t>(BUN_SSE_CMPLT_EPI16(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int16_t> operator<=(const sseVecT<int16_t>& r) const { return !operator>(r); }
  BUN_FORCEINLINE sseVecT<int16_t> operator>(const sseVecT<int16_t>& r) const { return sseVecT<int16_t>(BUN_SSE_CMPGT_EPI16(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int16_t> operator>=(const sseVecT<int16_t>& r) const { return !operator<(r); }
  BUN_FORCEINLINE sseVecT<int16_t> operator~() const { return sseVecT<int16_t>(BUN_SSE_XOR(xmm, BUN_SSE_SET1_EPI16(-1))); }
  BUN_FORCEINLINE sseVecT<int16_t> operator!() const { return sseVecT<int16_t>(BUN_SSE_CMPEQ_EPI16(xmm, ZeroVector())); }
  BUN_FORCEINLINE sseVecT<int16_t>& operator=(BUN_SSE_M128i16 r) { xmm = r; return *this; }
  BUN_FORCEINLINE sseVecT<int16_t>& operator=(int16_t v) { xmm = BUN_SSE_SET1_EPI16(v); return *this; }
  BUN_FORCEINLINE sseVecT<int16_t>& operator=(const int16_t(&v)[8]) { xmm = BUN_SSE_LOAD_ASI128((BUN_SSE_M128i*)v); assert(!(((size_t)v) % 16)); return *this; }
  BUN_FORCEINLINE sseVecT<int16_t>& operator=(BUN_UNALIGNED<const int16_t> v) { xmm = BUN_SSE_LOAD_USI128((BUN_SSE_M128i*)v._p); return *this; }

  BUN_FORCEINLINE void Set(int16_t(&v)[8]) const { assert(!(((size_t)v) % 16)); BUN_SSE_STORE_ASI128((BUN_SSE_M128i*)v, xmm); }
  BUN_FORCEINLINE void Set(BUN_UNALIGNED<int16_t> v) const { BUN_SSE_STORE_USI128((BUN_SSE_M128i*)v._p, xmm); }
  BUN_FORCEINLINE void Set(int16_t& v) const { v = BUN_SSE_SI128_SI16(xmm); }
  static sseVecT<int16_t> ZeroVector() { return sseVecT<int16_t>(BUN_SSE_SETZERO_SI128()); }
  template<int Positions>
  static sseVecT<int16_t> Shuffle(const sseVecT<int16_t>& x) { return BUN_SSE_SHUFFLE_EPI32(x, Positions); }
  template<int Positions>
  static sseVecT<int16_t> ShuffleHi(const sseVecT<int16_t>& x) { return BUN_SSE_SHUFFLEHI_EPI16(x, Positions); }
  template<int Positions>
  static sseVecT<int16_t> ShuffleLo(const sseVecT<int16_t>& x) { return BUN_SSE_SHUFFLELO_EPI16(x, Positions); }

  BUN_SSE_M128i16 xmm;
};

// 32-bit signed integer operations
template<>
BUN_ALIGNED_STRUCT(16) sseVecT<int32_t>
{
  BUN_FORCEINLINE sseVecT(BUN_SSE_M128i v) : xmm(v) {} //__fastcall is obviously useless here since we're dealing with xmm registers
  BUN_FORCEINLINE explicit sseVecT(BUN_SSE_M128 v) : xmm(BUN_SSE_TPS_EPI32(v)) {}
  BUN_FORCEINLINE explicit sseVecT(BUN_SSE_M128d v) : xmm(BUN_SSE_TPD_EPI32(v)) {}
  BUN_FORCEINLINE sseVecT(BUN_SSE_M128 v, char round) : xmm(BUN_SSE_PS_EPI32(v)) {}
  BUN_FORCEINLINE sseVecT(int v) : xmm(BUN_SSE_SET1_EPI32(v)) {}
  //BUN_FORCEINLINE sseVecT<int>(const int*BUN_RESTRICT v) : xmm(BUN_SSE_LOAD_ASI128(v)) { assert(!(((size_t)v)%16)); }
  BUN_FORCEINLINE explicit sseVecT(const int(&v)[4]) : xmm(BUN_SSE_LOAD_ASI128((BUN_SSE_M128i*)v)) { assert(!(((size_t)v) % 16)); }
  BUN_FORCEINLINE explicit sseVecT(BUN_UNALIGNED<const int> v) : xmm(BUN_SSE_LOAD_USI128((BUN_SSE_M128i*)v._p)) {}
  BUN_FORCEINLINE sseVecT(int x, int y, int z, int w) : xmm(BUN_SSE_SET_EPI32(w, z, y, x)) {}
  BUN_FORCEINLINE sseVecT<int> operator+(const sseVecT<int>& r) const { return sseVecT<int>(BUN_SSE_ADD_EPI32(xmm, r.xmm)); } // These don't return const sseVecT<int> because it makes things messy.
  BUN_FORCEINLINE sseVecT<int> operator-(const sseVecT<int>& r) const { return sseVecT<int>(BUN_SSE_SUB_EPI32(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int> operator*(const sseVecT<int>& r) const { return sseVecT<int>(BUN_SSE_MUL_EPI32(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int> operator&(const sseVecT<int>& r) const { return sseVecT<int>(BUN_SSE_AND(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int> operator|(const sseVecT<int>& r) const { return sseVecT<int>(BUN_SSE_OR(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int> operator^(const sseVecT<int>& r) const { return sseVecT<int>(BUN_SSE_XOR(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int> operator>>(const sseVecT<int>& r) const { return sseVecT<int>(BUN_SSE_SAR_EPI32(xmm, r.xmm)); } // Amazingly enough this does not conflict with the extraction operator
  BUN_FORCEINLINE sseVecT<int> operator>>(int r) const { return sseVecT<int>(BUN_SSE_SARI_EPI32(xmm, r)); }
  BUN_FORCEINLINE sseVecT<int> operator<<(const sseVecT<int>& r) const { return sseVecT<int>(BUN_SSE_SL_EPI32(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int> operator<<(int r) const { return sseVecT<int>(BUN_SSE_SLI_EPI32(xmm, r)); }
  BUN_FORCEINLINE sseVecT<int>& operator+=(const sseVecT<int>& r) { xmm = BUN_SSE_ADD_EPI32(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<int>& operator-=(const sseVecT<int>& r) { xmm = BUN_SSE_SUB_EPI32(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<int>& operator*=(const sseVecT<int>& r) { xmm = BUN_SSE_MUL_EPI32(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<int>& operator&=(const sseVecT<int>& r) { xmm = BUN_SSE_AND(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<int>& operator|=(const sseVecT<int>& r) { xmm = BUN_SSE_OR(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<int>& operator^=(const sseVecT<int>& r) { xmm = BUN_SSE_XOR(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<int>& operator>>=(const sseVecT<int>& r) { xmm = BUN_SSE_SAR_EPI32(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<int>& operator>>=(int r) { xmm = BUN_SSE_SARI_EPI32(xmm, r); return *this; }
  BUN_FORCEINLINE sseVecT<int>& operator<<=(const sseVecT<int>& r) { xmm = BUN_SSE_SL_EPI32(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<int>& operator<<=(int r) { xmm = BUN_SSE_SLI_EPI32(xmm, r); return *this; }

  BUN_FORCEINLINE sseVecT<int> min(const sseVecT<int>& r) const { return sseVecT<int>(BUN_SSE_MIN_EPI32(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int> max(const sseVecT<int>& r) const { return sseVecT<int>(BUN_SSE_MAX_EPI32(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int> operator==(const sseVecT<int>& r) const { return sseVecT<int>(BUN_SSE_CMPEQ_EPI32(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int> operator!=(const sseVecT<int>& r) const { return !operator==(r); }
  BUN_FORCEINLINE sseVecT<int> operator<(const sseVecT<int>& r) const { return sseVecT<int>(BUN_SSE_CMPLT_EPI32(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int> operator<=(const sseVecT<int>& r) const { return !operator>(r); }
  BUN_FORCEINLINE sseVecT<int> operator>(const sseVecT<int>& r) const { return sseVecT<int>(BUN_SSE_CMPGT_EPI32(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int> operator>=(const sseVecT<int>& r) const { return !operator<(r); }

#ifndef BUN_SSE_ENABLED
  BUN_FORCEINLINE operator bun_si128() const { return xmm; }
#endif
  BUN_FORCEINLINE operator BUN_SSE_M128i() const { return xmm; }
  BUN_FORCEINLINE sseVecT<int> operator!() const { return sseVecT<int>(BUN_SSE_CMPEQ_EPI32(xmm, ZeroVector())); }
  BUN_FORCEINLINE sseVecT<int> operator~() const { return sseVecT<int>(BUN_SSE_XOR(xmm, BUN_SSE_SET1_EPI32(-1))); }
  BUN_FORCEINLINE sseVecT<int>& operator=(BUN_SSE_M128i r) { xmm = r; return *this; }
  BUN_FORCEINLINE sseVecT<int>& operator=(BUN_SSE_M128 r) { xmm = BUN_SSE_TPS_EPI32(r); return *this; }
  BUN_FORCEINLINE sseVecT<int>& operator=(int v) { xmm = BUN_SSE_SET1_EPI32(v); return *this; }
  BUN_FORCEINLINE sseVecT<int>& operator=(const int(&v)[4]) { xmm = BUN_SSE_LOAD_ASI128((BUN_SSE_M128i*)v); assert(!(((size_t)v) % 16)); return *this; }
  BUN_FORCEINLINE sseVecT<int>& operator=(BUN_UNALIGNED<const int> v) { xmm = BUN_SSE_LOAD_USI128((BUN_SSE_M128i*)v._p); return *this; }

  //BUN_FORCEINLINE void Set(int*BUN_RESTRICT v) { assert(!(((size_t)v)%16)); BUN_SSE_STORE_APS(v, xmm); }
  BUN_FORCEINLINE void Set(int(&v)[4]) const { assert(!(((size_t)v) % 16)); BUN_SSE_STORE_ASI128((BUN_SSE_M128i*)v, xmm); }
  BUN_FORCEINLINE void Set(BUN_UNALIGNED<int> v) const { BUN_SSE_STORE_USI128((BUN_SSE_M128i*)v._p, xmm); }
  BUN_FORCEINLINE void Set(int& v) const { v = BUN_SSE_SI128_SI32(xmm); }
  static sseVecT<int> ZeroVector() { return sseVecT<int>(BUN_SSE_SETZERO_SI128()); }
  template<int x, int y, int z, int w>
  BUN_FORCEINLINE sseVecT<int> Shuffle() const { return Shuffle<BUN_SSE_SHUFFLE(x, y, z, w)>(*this); }
  BUN_FORCEINLINE int Sum() const { sseVecT<int> x = BUN_SSE_ADD_EPI32(xmm, Shuffle<1, 0, 3, 2>()); return BUN_SSE_SI128_SI32(BUN_SSE_ADD_EPI32(x, x.Shuffle<2, 3, 0, 1>())); }

  template<int Positions>
  static sseVecT<int> Shuffle(const sseVecT<int>& x) { return sseVecT<int>(BUN_SSE_SHUFFLE_EPI32(x.xmm, Positions)); }
  template<int Positions>
  static sseVecT<int> ShuffleHi(const sseVecT<int>& x) { return sseVecT<int>(BUN_SSE_SHUFFLEHI_EPI16(x.xmm, Positions)); }
  template<int Positions>
  static sseVecT<int> ShuffleLo(const sseVecT<int>& x) { return sseVecT<int>(BUN_SSE_SHUFFLELO_EPI16(x.xmm, Positions)); }

  BUN_SSE_M128i xmm;
};

// 64-bit signed integer operations
template<>
BUN_ALIGNED_STRUCT(16) sseVecT<int64_t>
{
  BUN_FORCEINLINE sseVecT(BUN_SSE_M128i64 v) : xmm(v) {} //__fastcall is obviously useless here since we're dealing with xmm registers
  BUN_FORCEINLINE sseVecT(int64_t v) : xmm(BUN_SSE_SET1_EPI64(v)) {}
  BUN_FORCEINLINE explicit sseVecT(const int64_t(&v)[2]) : xmm(BUN_SSE_LOAD_ASI128((BUN_SSE_M128i*)v)) { assert(!(((size_t)v) % 16)); }
  BUN_FORCEINLINE explicit sseVecT(BUN_UNALIGNED<const int64_t> v) : xmm(BUN_SSE_LOAD_USI128((BUN_SSE_M128i*)v._p)) {}
  BUN_FORCEINLINE sseVecT(int64_t x, int64_t y) : xmm(BUN_SSE_SET_EPI64(y, x)) {}
  BUN_FORCEINLINE sseVecT<int64_t> operator+(const sseVecT<int64_t>& r) const { return sseVecT<int64_t>(BUN_SSE_ADD_EPI64(xmm, r.xmm)); } // These don't return const sseVecT<int64_t> because it makes things messy.
  BUN_FORCEINLINE sseVecT<int64_t> operator-(const sseVecT<int64_t>& r) const { return sseVecT<int64_t>(BUN_SSE_SUB_EPI64(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int64_t> operator&(const sseVecT<int64_t>& r) const { return sseVecT<int64_t>(BUN_SSE_AND(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int64_t> operator|(const sseVecT<int64_t>& r) const { return sseVecT<int64_t>(BUN_SSE_OR(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int64_t> operator^(const sseVecT<int64_t>& r) const { return sseVecT<int64_t>(BUN_SSE_XOR(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int64_t> operator>>(const sseVecT<int64_t>& r) const { return sseVecT<int64_t>(BUN_SSE_SR_EPI64(xmm, r.xmm)); } // Amazingly enough this does not conflict with the extraction operator
  BUN_FORCEINLINE sseVecT<int64_t> operator>>(int r) const { return sseVecT<int64_t>(BUN_SSE_SRI_EPI64(xmm, r)); }
  BUN_FORCEINLINE sseVecT<int64_t> operator<<(const sseVecT<int64_t>& r) const { return sseVecT<int64_t>(BUN_SSE_SL_EPI64(xmm, r.xmm)); }
  BUN_FORCEINLINE sseVecT<int64_t> operator<<(int r) const { return sseVecT<int64_t>(BUN_SSE_SLI_EPI64(xmm, r)); }
  BUN_FORCEINLINE sseVecT<int64_t>& operator+=(const sseVecT<int64_t>& r) { xmm = BUN_SSE_ADD_EPI64(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<int64_t>& operator-=(const sseVecT<int64_t>& r) { xmm = BUN_SSE_SUB_EPI64(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<int64_t>& operator&=(const sseVecT<int64_t>& r) { xmm = BUN_SSE_AND(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<int64_t>& operator|=(const sseVecT<int64_t>& r) { xmm = BUN_SSE_OR(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<int64_t>& operator^=(const sseVecT<int64_t>& r) { xmm = BUN_SSE_XOR(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<int64_t>& operator>>=(const sseVecT<int64_t>& r) { xmm = BUN_SSE_SR_EPI64(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<int64_t>& operator>>=(int r) { xmm = BUN_SSE_SRI_EPI64(xmm, r); return *this; }
  BUN_FORCEINLINE sseVecT<int64_t>& operator<<=(const sseVecT<int64_t>& r) { xmm = BUN_SSE_SL_EPI64(xmm, r.xmm); return *this; }
  BUN_FORCEINLINE sseVecT<int64_t>& operator<<=(int r) { xmm = BUN_SSE_SLI_EPI64(xmm, r); return *this; }

#ifndef BUN_SSE_ENABLED
  BUN_FORCEINLINE operator bun_si128() const { return xmm; }
#endif
  BUN_FORCEINLINE operator BUN_SSE_M128i64() const { return xmm; }
  BUN_FORCEINLINE sseVecT<int64_t> operator~() const { return sseVecT<int64_t>(BUN_SSE_XOR(xmm, BUN_SSE_SET1_EPI64(-1))); }
  BUN_FORCEINLINE sseVecT<int64_t>& operator=(BUN_SSE_M128i64 r) { xmm = r; return *this; }
  BUN_FORCEINLINE sseVecT<int64_t>& operator=(int64_t v) { xmm = BUN_SSE_SET1_EPI64(v); return *this; }
  BUN_FORCEINLINE sseVecT<int64_t>& operator=(const int64_t(&v)[2]) { xmm = BUN_SSE_LOAD_ASI128((BUN_SSE_M128i*)v); assert(!(((size_t)v) % 16)); return *this; }
  BUN_FORCEINLINE sseVecT<int64_t>& operator=(BUN_UNALIGNED<const int64_t> v) { xmm = BUN_SSE_LOAD_USI128((BUN_SSE_M128i*)v._p); return *this; }
  BUN_FORCEINLINE void Set(int64_t(&v)[2]) const { assert(!(((size_t)v) % 16)); BUN_SSE_STORE_ASI128((BUN_SSE_M128i*)v, xmm); }
  BUN_FORCEINLINE void Set(BUN_UNALIGNED<int64_t> v) const { BUN_SSE_STORE_USI128((BUN_SSE_M128i*)v._p, xmm); }
#ifdef BUN_64BIT
  BUN_FORCEINLINE void Set(int64_t& v) const { v = BUN_SSE_SI128_SI64(xmm); }
#else
  BUN_FORCEINLINE void Set(int64_t& v) const { BUN_ALIGN(16) int64_t store[2]; BUN_SSE_STORE_ASI128((BUN_SSE_M128i*)store, xmm); v = store[0]; }
#endif
  BUN_FORCEINLINE int64_t Sum() const { BUN_SSE_M128i64 x = BUN_SSE_ADD_EPI64(xmm, Swap()); BUN_ALIGN(16) int64_t store[2]; BUN_SSE_STORE_ASI128((BUN_SSE_M128i*)store, x); return store[0]; }
  BUN_FORCEINLINE sseVecT<int64_t> Swap() const { return (BUN_SSE_M128i64)BUN_SSE_SHUFFLE_EPI32(xmm, BUN_SSE_SHUFFLE(2, 3, 0, 1)); }

  static sseVecT<int64_t> ZeroVector() { return sseVecT<int64_t>(BUN_SSE_SETZERO_SI128()); }
  template<int Positions>
  static sseVecT<int64_t> Shuffle(const sseVecT<int64_t>& x) { return BUN_SSE_SHUFFLE_EPI32(x, Positions); }
  template<int Positions>
  static sseVecT<int64_t> ShuffleHi(const sseVecT<int64_t>& x) { return BUN_SSE_SHUFFLEHI_EPI16(x, Positions); }
  template<int Positions>
  static sseVecT<int64_t> ShuffleLo(const sseVecT<int64_t>& x) { return BUN_SSE_SHUFFLELO_EPI16(x, Positions); }

  BUN_SSE_M128i64 xmm;
};

using sseVec = sseVecT<float>;
using sseVecd = sseVecT<double>;
typedef sseVecT<char> sseVeci8;
typedef sseVecT<int16_t> sseVeci16;
using sseVeci = sseVecT<int32_t>;
typedef sseVecT<int64_t> sseVeci64;
}

#endif
