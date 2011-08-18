// Copyright �2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_FIXED_PT_H__
#define __BSS_FIXED_PT_H__

#include "bss_util.h"

namespace bss_util {
  template<typename T, unsigned char D, bool SATURATE> struct __FIXED_PT_FUNC {};
  template<typename T, unsigned char D> struct __FIXED_PT_FUNC<T,D,false>
  {
    static inline T BSS_FASTCALL fixedpt_add(T x, T y) { return x+y; }
    static inline T BSS_FASTCALL fixedpt_mul(T x, T y) { 
      return (T)((((TSignPick<(sizeof(T)<<1)>::SIGNED)x)*((TSignPick<(sizeof(T)<<1)>::SIGNED)y))>>(D)); }
    static inline T BSS_FASTCALL fixedpt_div(T x, T y) { return (T)((((TSignPick<(sizeof(T)<<1)>::SIGNED)x)<<D)/y); }
  };
  template<typename T, unsigned char D> struct __FIXED_PT_FUNC<T,D,true>
  {
    static const T SMIN=((ABitLimit<((sizeof(T)<<3)-D)>::SIGNED_MIN)<<D);
    static const T SMAX=((ABitLimit<((sizeof(T)<<3)-D)>::SIGNED_MAX)<<D);

    static inline T BSS_FASTCALL fixedpt_add(T x, T y)
    { 
      T r=x+y;
      T u=(r>0)&(x<0)&(y<0);
      T v=(r<0)&(x>0)&(y>0);
      T w=(1&(~(u|v))); //w becomes 0 if either u or v evaluate to 1
      return (r*w)|(u*SMIN)|(v*SMAX);
    }
    static inline T BSS_FASTCALL fixedpt_mul(T x, T y)
    { 
      TSignPick<(sizeof(T)<<1)>::SIGNED r=((((TSignPick<(sizeof(T)<<1)>::SIGNED)x)*((TSignPick<(sizeof(T)<<1)>::SIGNED)y))>>(D));
      T u=(r<SMIN); //same technique for saturating with no branching
      T v=(r>SMAX);
      T w=(1&(~(u|v))); 
      return (((T)r)*w)|(u*SMIN)|(v*SMAX);
    }
    static inline T BSS_FASTCALL fixedpt_div(T x, T y)
    { 
      TSignPick<(sizeof(T)<<1)>::SIGNED r=((((TSignPick<(sizeof(T)<<1)>::SIGNED)x)<<D)/y);
      T u=(r<SMIN); //same technique for saturating with no branching
      T v=(r>SMAX);
      T w=(1&(~(u|v))); 
      return (((T)r)*w)|(u*SMIN)|(v*SMAX);
    }
  };
  
  /* Adaptive template based class for doing fixed-point math */
  template<unsigned char DBITS=12, typename T=__int32, bool SATURATE=true>
  class FixedPt
  {
  public:
    template<bool S>
    inline FixedPt(const FixedPt<DBITS,T,S>& copy) : _bits(copy._bits) {}
    template<unsigned char D, typename U, bool S>
    inline FixedPt(const FixedPt<D,U,S>& copy) : _bits((T)SAFESHIFT(copy._bits,DBITS-D)) {}
    inline FixedPt(const T v) : _bits(v<<DBITS) {}
		inline FixedPt(const float v) : _bits((T)(v*((float)(1<<DBITS)))) {}
		inline FixedPt(const double v) : _bits((T)(v*((double)(1<<DBITS)))) {}

    inline operator T() const { return _bits>>DBITS; }
    inline operator float() const { return ((float)_bits)/((float)(1<<DBITS)); }
    inline operator double() const { return ((double)_bits)/((double)(1<<DBITS)); }

    template<bool S>
    inline FixedPt& BSS_FASTCALL operator=(const FixedPt<DBITS,T,S>& right) { _bits=right._bits; }
    template<unsigned char D, typename U, bool S>
    inline FixedPt& BSS_FASTCALL operator=(const FixedPt<D,U,S>& right) { _bits=(T)SAFESHIFT(right._bits,DBITS-D); }

    inline FixedPt& operator -(void) const { FixedPt r(*this); r._bits=-r._bits; return r; }

    inline FixedPt& BSS_FASTCALL operator+=(const FixedPt& right) { _bits=__FIXED_PT_FUNC<T,DBITS,SATURATE>::fixedpt_add(_bits,right._bits); return *this; }
    inline FixedPt& BSS_FASTCALL operator-=(const FixedPt& right) { _bits=__FIXED_PT_FUNC<T,DBITS,SATURATE>::fixedpt_add(_bits,-right._bits); return *this; }
    inline FixedPt& BSS_FASTCALL operator*=(const FixedPt& right) { _bits=__FIXED_PT_FUNC<T,DBITS,SATURATE>::fixedpt_mul(_bits,right._bits); return *this; }
    inline FixedPt& BSS_FASTCALL operator/=(const FixedPt& right) { _bits=__FIXED_PT_FUNC<T,DBITS,SATURATE>::fixedpt_div(_bits,right._bits); return *this; }
    
    inline FixedPt BSS_FASTCALL operator+(const FixedPt& right) const { return FixedPt(*this)+=right; }
    inline FixedPt BSS_FASTCALL operator-(const FixedPt& right) const { return FixedPt(*this)-=right; }
    inline FixedPt BSS_FASTCALL operator*(const FixedPt& right) const { return FixedPt(*this)*=right; }
    inline FixedPt BSS_FASTCALL operator/(const FixedPt& right) const { return FixedPt(*this)/=right; }
    
    inline bool BSS_FASTCALL operator !=(const FixedPt& right) const { return _bits!=right._bits; }
    inline bool BSS_FASTCALL operator ==(const FixedPt& right) const { return _bits==right._bits; }
    inline bool BSS_FASTCALL operator >=(const FixedPt& right) const { return _bits>=right._bits; }
    inline bool BSS_FASTCALL operator <=(const FixedPt& right) const { return _bits<=right._bits; }
    inline bool BSS_FASTCALL operator >(const FixedPt& right) const { return _bits>right._bits; }
    inline bool BSS_FASTCALL operator <(const FixedPt& right) const { return _bits<right._bits; }

  protected:
    T _bits;
  };
  
  template<unsigned char D, typename T, bool S>
  inline float operator+(const float l,const FixedPt<D,T,S>& r) { return l+(float)r; }
  template<unsigned char D, typename T, bool S>
  inline double operator+(const double l,const FixedPt<D,T,S>& r) { return l+(double)r; }
  template<unsigned char D, typename T, bool S>
  inline float operator-(const float l,const FixedPt<D,T,S>& r) { return l-(float)r; }
  template<unsigned char D, typename T, bool S>
  inline double operator-(const double l,const FixedPt<D,T,S>& r) { return l-(double)r; }
  template<unsigned char D, typename T, bool S>
  inline float operator*(const float l,const FixedPt<D,T,S>& r) { return l*(float)r; }
  template<unsigned char D, typename T, bool S>
  inline double operator*(const double l,const FixedPt<D,T,S>& r) { return l*(double)r; }
  template<unsigned char D, typename T, bool S>
  inline float operator/(const float l,const FixedPt<D,T,S>& r) { return l/(float)r; }
  template<unsigned char D, typename T, bool S>
  inline double operator/(const double l,const FixedPt<D,T,S>& r) { return l/(double)r; }
  
  template<unsigned char D, typename T, bool S>
  inline float& operator+=(float& l,const FixedPt<D,T,S>& r) { return l+=(float)r; }
  template<unsigned char D, typename T, bool S>
  inline double& operator+=(double& l,const FixedPt<D,T,S>& r) { return l+=(double)r; }
  template<unsigned char D, typename T, bool S>
  inline float& operator-=(float& l,const FixedPt<D,T,S>& r) { return l-=(float)r; }
  template<unsigned char D, typename T, bool S>
  inline double& operator-=(double& l,const FixedPt<D,T,S>& r) { return l-=(double)r; }
  template<unsigned char D, typename T, bool S>
  inline float& operator*=(float& l,const FixedPt<D,T,S>& r) { return l*=(float)r; }
  template<unsigned char D, typename T, bool S>
  inline double& operator*=(double& l,const FixedPt<D,T,S>& r) { return l*=(double)r; }
  template<unsigned char D, typename T, bool S>
  inline float& operator/=(float& l,const FixedPt<D,T,S>& r) { return l/=(float)r; }
  template<unsigned char D, typename T, bool S>
  inline double& operator/=(double& l,const FixedPt<D,T,S>& r) { return l/=(double)r; }
}

#endif