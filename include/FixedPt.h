// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_FIXED_H__
#define __BSS_FIXED_H__

#include "bss_util.h"

namespace bss_util {
  template<typename T, T S, typename F>
  inline T fixedpt_conv(F f) { return (T)(f * ((T)1 << S)); }
  template<typename T, T S>
  inline float convf_fixedpt(T i) { static const T SMIN = bssmax(0, S - 24); return ((float)(i >> S)) + (((i&(((T)1 << S) - 1)) >> SMIN) / (float)((T)1 << (S - SMIN))); }
  template<typename T, T S>
  inline double convd_fixedpt(T i) { static const T SMIN = bssmax(0, S - 53); return ((double)(i >> S)) + (((i&(((T)1 << S) - 1)) >> SMIN) / (double)((T)1 << (S - SMIN))); }
  
  template<typename T, T S_X, T S_Y>
  inline T fixedpt_add(T x, T y) { return x + SAFESHIFT(y, S_X - S_Y); }
  template<typename T>
  inline T fixedpt_mul(T x, T y, T S_Y) { return bssmultiplyextract<T>(x, y, S_Y); } // The result has a scaling factor of 2^S_X * 2^S_Y = 2^(S_X + S_Y). To convert this back into 2^S_X, we divide by 2^S_Y, which is equivilent to shifting right by S_Y
  //inline T fixedpt_mul(T x, T y) { typedef typename BitLimit<sizeof(T) << 4>::SIGNED U; return ((U)x * (U)y) >> S_Y; }
  template<typename T> // Resulting decimal bits after this when T is a 32-bit int is 30 - S
  inline T fixedpt_recip(T x) { return ((T)1 << ((sizeof(T) << 3) - 2)) / x; }
  template<typename T>
  inline T fixedpt_div(T x, T y, T S_Y) { // The result here has a scaling factor of 2^S_X / 2^S_Y = 2^(S_X - S_Y). By shifting left S_Y before the division, we end up with an S_X scaling factor.
    typedef typename BitLimit<sizeof(T) << 4>::SIGNED U;
    return (U(x) << S_Y) / (U)y;
  }
  template<> // For 64-bit, we can't divide a 128-bit number, so we use the reciprocal instead, which sacrifices just two bits of precision.
  inline int64_t fixedpt_div(int64_t x, int64_t y, int64_t S_Y) { return fixedpt_mul<int64_t>(x, fixedpt_recip<int64_t>(y), (sizeof(int64_t) << 3) - 2 - S_Y); }

  /*template<typename T>
  T fixedpt_add_saturate(T r, T x, T y)
  {
    T u = (r > 0)&(x < 0)&(y < 0);
    T v = (r < 0)&(x > 0)&(y > 0);
    T w = ((T)1 & (~(u | v)));
    return (r*w) | (u*std::numeric_limits<T>::min()) | (v*std::numeric_limits<T>::max());
  }
  template<typename T>
  T fixedpt_mul_saturate(T r, T x, T y)
  {
    T u = (r < SMIN); // TODO: This doesn't actually work
    T v = (r > SMAX);
    T w = ((T)1 & (~(u | v)));
    return (((T)r)*w) | (u*std::numeric_limits<T>::min()) | (v*std::numeric_limits<T>::max());
  }*/

  template<typename T>
  struct __FixedBits { T bits; };

  // Adaptive template based class for doing fixed-point math. Default number of decimal bits is 12 (corresponding to a scaling factor of 2^12)
  template<uint8_t D = 12, typename T = int32_t, bool SATURATE = false>
  class Fixed
  {
    static_assert(!SATURATE, "Saturation is not implemented.");
    static_assert(std::is_signed<T>::value, "T must be a signed integral type.");

  public:
    inline Fixed(const Fixed& copy) : _bits(copy._bits) {}
    template<uint8_t B, typename U, bool S>
    inline Fixed(const Fixed<B, U, S>& copy) : _bits((T)SAFESHIFT(copy.Bits(), B - D)) {}
    inline explicit Fixed(const T v) : _bits(v << D) {}
    inline Fixed(__FixedBits<T> bits) : _bits(bits.bits) {}
    inline explicit Fixed(const float f) : _bits(fixedpt_conv<T, D, float>(f)) {}
    inline explicit Fixed(const double f) : _bits(fixedpt_conv<T, D, double>(f)) {}
    inline T Bits() const { return _bits; }

    inline operator T() const { return _bits >> D; }
    inline operator float() const { return convf_fixedpt<T, D>(_bits); }
    inline operator double() const { return convd_fixedpt<T, D>(_bits); }

    inline Fixed& BSS_FASTCALL operator=(const Fixed& right) { _bits = right._bits; return *this; }
    template<uint8_t B, typename U, bool S>
    inline Fixed& BSS_FASTCALL operator=(const Fixed<B, U, S>& right) { _bits = (T)SAFESHIFT(right.Bits(), B - D); return *this; }

    inline Fixed<(sizeof(T) << 3) - D, T, SATURATE> Reciprocal() const { return Fixed<(sizeof(T) << 3) - D, T, SATURATE>(__FixedBits<T> {fixedpt_recip(_bits) << 2 }); } // Shift to the right because the actual resulting decimal bits is (sizeof(T) << 3) - 2 - B
    inline Fixed operator -(void) const { return __FixedBits<T> { -_bits }; }

    inline Fixed& BSS_FASTCALL operator%=(const Fixed& r) { _bits %= r._bits; return *this; }
    inline Fixed& BSS_FASTCALL operator>>=(T r) { _bits >>= r; return *this; }
    inline Fixed& BSS_FASTCALL operator<<=(T r) { _bits <<= r; return *this; }
    inline Fixed& BSS_FASTCALL operator+=(const Fixed& r) { _bits += r._bits; return *this; }
    inline Fixed& BSS_FASTCALL operator-=(const Fixed& r) { _bits -= r._bits; return *this; }
    template<uint8_t B, bool S>
    inline Fixed& BSS_FASTCALL operator+=(const Fixed<B, T, S>& r) { _bits = fixedpt_add<T, D, B>(_bits, r.Bits()); return *this; }
    template<uint8_t B, bool S>
    inline Fixed& BSS_FASTCALL operator-=(const Fixed<B, T, S>& r) { _bits = fixedpt_add<T, D, B>(_bits, -r.Bits()); return *this; }
    template<uint8_t B, bool S>
    inline Fixed& BSS_FASTCALL operator*=(const Fixed<B, T, S>& r) { _bits = fixedpt_mul<T>(_bits, r.Bits(), B); return *this; }
    template<uint8_t B, bool S>
    inline Fixed& BSS_FASTCALL operator/=(const Fixed<B, T, S>& r) { _bits = fixedpt_div<T>(_bits, r.Bits(), B); return *this; }
    inline Fixed& BSS_FASTCALL operator+=(float r) { _bits += fixedpt_conv<T, D, float>(r); return *this; }
    inline Fixed& BSS_FASTCALL operator-=(float r) { _bits -= fixedpt_conv<T, D, float>(r); return *this; }
    inline Fixed& BSS_FASTCALL operator*=(float r) { _bits = fixedpt_mul<T>(_bits, fixedpt_conv<T, D, float>(r), D); return *this; }
    inline Fixed& BSS_FASTCALL operator/=(float r) { _bits = fixedpt_div<T>(_bits, fixedpt_conv<T, D, float>(r), D); return *this; }
    inline Fixed& BSS_FASTCALL operator+=(double r) { _bits += fixedpt_conv<T, D, double>(r); return *this; }
    inline Fixed& BSS_FASTCALL operator-=(double r) { _bits -= fixedpt_conv<T, D, double>(r); return *this; }
    inline Fixed& BSS_FASTCALL operator*=(double r) { _bits = fixedpt_mul<T>(_bits, fixedpt_conv<T, D, double>(r), D); return *this; }
    inline Fixed& BSS_FASTCALL operator/=(double r) { _bits = fixedpt_div<T>(_bits, fixedpt_conv<T, D, double>(r), D); return *this; }

    inline Fixed BSS_FASTCALL operator %(const Fixed& r) const { return __FixedBits<T> { _bits % r._bits }; }
    inline Fixed BSS_FASTCALL operator <<(T r) const { return __FixedBits<T> { _bits << r }; }
    inline Fixed BSS_FASTCALL operator >> (T r) const { return __FixedBits<T> { _bits >> r }; }
    inline Fixed BSS_FASTCALL operator+(const Fixed& r) const { return __FixedBits<T> { _bits + r._bits }; }
    inline Fixed BSS_FASTCALL operator-(const Fixed& r) const { return __FixedBits<T> { _bits - r._bits }; }
    template<uint8_t B, bool S>
    inline Fixed BSS_FASTCALL operator+(const Fixed<B, T, S>& r) const { return __FixedBits<T> { fixedpt_add<T, D, B>(_bits, r.Bits()) }; }
    template<uint8_t B, bool S>
    inline Fixed BSS_FASTCALL operator-(const Fixed<B, T, S>& r) const { return __FixedBits<T> { fixedpt_add<T, D, B>(_bits, -r.Bits()) }; }
    template<uint8_t B, bool S>
    inline Fixed BSS_FASTCALL operator*(const Fixed<B, T, S>& r) const { return __FixedBits<T> { fixedpt_mul<T>(_bits, r.Bits(), B) }; }
    template<uint8_t B, bool S>
    inline Fixed BSS_FASTCALL operator/(const Fixed<B, T, S>& r) const { return __FixedBits<T> { fixedpt_div<T>(_bits, r.Bits(), B) }; }
    inline Fixed BSS_FASTCALL operator+(float r) const { return __FixedBits<T> { _bits + fixedpt_conv<T, D, float>(r) }; }
    inline Fixed BSS_FASTCALL operator-(float r) const { return __FixedBits<T> { _bits - fixedpt_conv<T, D, float>(r) }; }
    inline Fixed BSS_FASTCALL operator*(float r) const { return __FixedBits<T> { fixedpt_mul<T>(_bits, fixedpt_conv<T, D, float>(r), D) }; }
    inline Fixed BSS_FASTCALL operator/(float r) const { return __FixedBits<T> { fixedpt_div<T>(_bits, fixedpt_conv<T, D, float>(r), D) }; }
    inline Fixed BSS_FASTCALL operator+(double r) const { return __FixedBits<T> { _bits + fixedpt_conv<T, D, double>(r) }; }
    inline Fixed BSS_FASTCALL operator-(double r) const { return __FixedBits<T> { _bits - fixedpt_conv<T, D, double>(r) }; }
    inline Fixed BSS_FASTCALL operator*(double r) const { return __FixedBits<T> { fixedpt_mul<T>(_bits, fixedpt_conv<T, D, double>(r), D) }; }
    inline Fixed BSS_FASTCALL operator/(double r) const { return __FixedBits<T> { fixedpt_div<T>(_bits, fixedpt_conv<T, D, double>(r), D) }; }

    BSS_FORCEINLINE bool BSS_FASTCALL operator !=(const Fixed& r) const { return _bits != r._bits; }
    BSS_FORCEINLINE bool BSS_FASTCALL operator ==(const Fixed& r) const { return _bits == r._bits; }
    BSS_FORCEINLINE bool BSS_FASTCALL operator >=(const Fixed& r) const { return _bits >= r._bits; }
    BSS_FORCEINLINE bool BSS_FASTCALL operator <=(const Fixed& r) const { return _bits <= r._bits; }
    BSS_FORCEINLINE bool BSS_FASTCALL operator >(const Fixed& r) const { return _bits > r._bits; }
    BSS_FORCEINLINE bool BSS_FASTCALL operator <(const Fixed& r) const { return _bits < r._bits; }

  protected:
    T _bits;
  };

  template<uint8_t D, typename T, bool S>
  inline Fixed<D, T, S> operator+(float l, const Fixed<D, T, S>& r) { return Fixed<D, T, S>(l) + r; }
  template<uint8_t D, typename T, bool S>
  inline Fixed<D, T, S> operator+(double l, const Fixed<D, T, S>& r) { return Fixed<D, T, S>(l) + r; }
  template<uint8_t D, typename T, bool S>
  inline Fixed<D, T, S> operator-(float l, const Fixed<D, T, S>& r) { return Fixed<D, T, S>(l) - r; }
  template<uint8_t D, typename T, bool S>
  inline Fixed<D, T, S> operator-(double l, const Fixed<D, T, S>& r) { return Fixed<D, T, S>(l) - r; }
  template<uint8_t D, typename T, bool S>
  inline Fixed<D, T, S> operator*(float l, const Fixed<D, T, S>& r) { return Fixed<D, T, S>(l) * r; }
  template<uint8_t D, typename T, bool S>
  inline Fixed<D, T, S> operator*(double l, const Fixed<D, T, S>& r) { return Fixed<D, T, S>(l) * r; }
  template<uint8_t D, typename T, bool S>
  inline Fixed<D, T, S> operator/(float l, const Fixed<D, T, S>& r) { return Fixed<D, T, S>(l) / r; }
  template<uint8_t D, typename T, bool S>
  inline Fixed<D, T, S> operator/(double l, const Fixed<D, T, S>& r) { return Fixed<D, T, S>(l) / r; }

  template<uint8_t D, typename T, bool S>
  inline float& operator+=(float& l, const Fixed<D, T, S>& r) { return l += (float)r; }
  template<uint8_t D, typename T, bool S>
  inline double& operator+=(double& l, const Fixed<D, T, S>& r) { return l += (double)r; }
  template<uint8_t D, typename T, bool S>
  inline float& operator-=(float& l, const Fixed<D, T, S>& r) { return l -= (float)r; }
  template<uint8_t D, typename T, bool S>
  inline double& operator-=(double& l, const Fixed<D, T, S>& r) { return l -= (double)r; }
  template<uint8_t D, typename T, bool S>
  inline float& operator*=(float& l, const Fixed<D, T, S>& r) { return l *= (float)r; }
  template<uint8_t D, typename T, bool S>
  inline double& operator*=(double& l, const Fixed<D, T, S>& r) { return l *= (double)r; }
  template<uint8_t D, typename T, bool S>
  inline float& operator/=(float& l, const Fixed<D, T, S>& r) { return l /= (float)r; }
  template<uint8_t D, typename T, bool S>
  inline double& operator/=(double& l, const Fixed<D, T, S>& r) { return l /= (double)r; }
}

#endif
