// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __RATIONAL_H__BUN__ //These are used in case this header file is used by two different projects dependent on each other, resulting in duplicates which cannot be differentiated by #pragma once
#define __RATIONAL_H__BUN__

#include "defines.h"
#include <stdlib.h>
#include <type_traits>

namespace bun {
  // This class represents a rational number in the form of an integral fraction, and includes conversion routines and simplification.
  template<typename T = int> requires std::is_integral<T>::value
  class BUN_COMPILER_DLLEXPORT Rational 
  {
  public:
    inline Rational(T n = 0) : _n(n), _d(1) {}
    inline Rational(T n, T d) : _n(n), _d(d) { Simplify(); }
    //inline Rational(float f) : _n(n), _d(d) {}
    inline void SetFraction(T n, T d) { _n = n; _d = d; Simplify(); }
    BUN_FORCEINLINE T N() const { return _n; }
    BUN_FORCEINLINE T D() const { return _d; }
    inline void Simplify()
    {
      if(!_n) 
      { 
        _d = 1;
        return; 
      }

      T gcd = GCD<T>(_n, _d);
      _n /= gcd;
      _d /= gcd;

      if(_d < 0)
      {
        _d = -_d;
        _n = -_n;
      }
    }

    inline Rational operator+ (const Rational& r) const { return Rational(*this) += r; }
    inline Rational operator- (const Rational& r) const { return Rational(*this) -= r; }
    inline Rational operator* (const Rational& r) const { return Rational(*this) *= r; }
    inline Rational operator/ (const Rational& r) const { return Rational(*this) /= r; }
    inline Rational operator+ (T i) const { return Rational(*this) += i; }
    inline Rational operator- (T i) const { return Rational(*this) -= i; }
    inline Rational operator* (T i) const { return Rational(*this) *= i; }
    inline Rational operator/ (T i) const { return Rational(*this) /= i; }
    inline Rational& operator+= (const Rational& r) // Uses the operator += algorithm found here: http://www.boost.org/doc/libs/1_48_0/boost/rational.hpp
    {
      T r_n = r._n;
      T r_d = r._d;

      T g = GCD<T>(_d, r_d);
      _d /= g;
      _n = _n * (r_d / g) + r_n * _d;
      g = GCD<T>(_n, g);
      _n /= g;
      _d *= r_d / g;

      return *this;
    }
    inline Rational& operator-= (const Rational& r) // Same as above
    {
      T r_n = r._n;
      T r_d = r._d;

      T g = GCD<T>(_d, r_d);
      _d /= g;
      _n = _n * (r_d / g) - r_n * _d;
      g = GCD<T>(_n, g);
      _n /= g;
      _d *= r_d / g;

      return *this;
    }
    inline Rational& operator*= (const Rational& r) // Uses the operator *= algorithm found here: http://www.boost.org/doc/libs/1_48_0/boost/rational.hpp
    {
      T r_n = r._n;
      T r_d = r._d;

      T gcd1 = GCD<T>(_n, r_d);
      T gcd2 = GCD<T>(r_n, _d);
      _n = (_n / gcd1) * (r_n / gcd2);
      _d = (_d / gcd2) * (r_d / gcd1);
      return *this;
    }
    inline Rational& operator/= (const Rational& r) // Same as above
    {
      T r_n = r._n;
      T r_d = r._d;

      if(!_n)
        return *this;

      T gcd1 = GCD<T>(_n, r_n);
      T gcd2 = GCD<T>(r_d, _d);
      _n = (_n / gcd1) * (r_d / gcd2);
      _d = (_d / gcd2) * (r_n / gcd1);

      if(_d < 0)
      {
        _n = -_n;
        _d = -_d;
      }
      return *this;
    }
    inline Rational& operator+= (T i) { _n += i * _d; return *this; } // This is derived by going through the operator += formula above and simplifying for the case when r_d=1
    inline Rational& operator-= (T i) { _n -= i * _d; return *this; } // See above, for operator -=
    inline Rational& operator*= (T i) { T g = GCD<T>(i, _d); _n *= (i / g); _d /= g; return *this; } // See above, for operator *=
    inline Rational& operator/= (T i) { if(!_n) return *this; T g = GCD<T>(_n, i); _n /= g; _d *= (i / g); if(_d < 0) { _n = -_n; _d = -_d; } return *this; }  // See above, for operator /= (if you try to divide by zero, its going to crash with a divide by zero)
    inline Rational& operator++() { _n += _d; return *this; }
    inline Rational& operator--() { _n -= _d; return *this; }
    inline Rational& operator=(T n) { _n = n; _d = 1; return *this; }
    inline bool operator!() const { return !_n; }
    inline bool operator> (const Rational& r) const { return operator double() > ((double)r); } //This can potentially cause precision issues in pathalogical cases, but it should do the job most of the time and the alternative is not O(1) time.
    inline bool operator< (const Rational& r) const { return operator double() < ((double)r); }
    inline bool operator>= (const Rational& r) const { return !operator<(r); }
    inline bool operator<= (const Rational& r) const { return !operator>(r); }
    inline bool operator== (const Rational& r) const { return _n == r._n && _d == r._d; }
    inline bool operator!= (const Rational& r) const { return _n != r._n || _d != r._d; }
    inline bool operator< (T i) const { T q = _n / _d, r = _n % _d; while(r < 0) { r += _d; --q; } return q < i; } //While this isn't O(1) time, its fast enough to be used instead of conversion to doubles.
    inline bool operator> (T i) const { if(operator==(i)) return false; return !operator<(i); }
    inline bool operator<= (T i) const { if(operator==(i)) return true; return !operator>(i); }
    inline bool operator>= (T i) const { if(operator==(i)) return true; return !operator<(i); }
    inline bool operator== (T i) const { return _d == 1 && i == _n; }
    inline bool operator!= (T i) const { return _d != 1 || i != _n; }
    inline Rational Abs() const { return Rational(abs(_n), _d); }
    inline operator float() const { return operator double(); } //We force this to happen with double precision because its highly likely floating point is going to choke on the division.
    inline operator double() const { return ((double)_n) / ((double)_d); }
    inline T Truncate() const { return _n / _d; }

    template<typename T2>
    static T2 GCD(T2 a, T2 b)
    {
      a = abs(a);
      b = abs(b);
      T2 tmp;

      if(b > a) { tmp = a; a = b; b = tmp; }
      while(b > 1) { tmp = b; b = a % b; a = tmp; }
      if(!b) return a;
      return b;
    }

    T _n; //numerator
    T _d; //denominator
  };

  template<typename T> inline Rational<T> operator+(T l, const Rational<T>& r) { Rational<T> n(r); n += l; return n; } // this is reversed using associativity for efficiency.
  template<typename T> inline Rational<T> operator-(T l, const Rational<T>& r) { Rational<T> n(l); n -= r; return n; }
  template<typename T> inline Rational<T> operator*(T l, const Rational<T>& r) { Rational<T> n(r); n *= l; return n; } // this is reversed using associativity for efficiency.
  template<typename T> inline Rational<T> operator/(T l, const Rational<T>& r) { Rational<T> n(l); n /= r; return n; }
  template<typename T> inline bool operator<(T l, const Rational<T>& r) { return r > l; }
  template<typename T> inline bool operator>(T l, const Rational<T>& r) { return r < l; }
  template<typename T> inline bool operator<=(T l, const Rational<T>& r) { return r >= l; }
  template<typename T> inline bool operator>=(T l, const Rational<T>& r) { return r <= l; }
  template<typename T> inline bool operator==(T l, const Rational<T>& r) { return r == l; }
  template<typename T> inline bool operator!=(T l, const Rational<T>& r) { return r != l; }
}

#endif
