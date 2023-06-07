// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BUN_DUAL_H__
#define __BUN_DUAL_H__

#include "compare.h"
#include <cmath>
#include <assert.h>

namespace bun {
  // Represents a dual number that can be used for (very inefficient) automatic differentiation.
  template<typename T, int O = 1>
  struct Dual
  {
    template<typename U>
    inline constexpr Dual(const Dual<U, O>& copy) : _x((T)copy._x), _dx(copy._dx) {}
    inline constexpr Dual(T x = 0, const Dual<T, O - 1>& dx = 0) : _x(x), _dx(dx) {}

    inline Dual operator+(T r) const { return operator+(Dual(r)); }
    inline Dual operator-(T r) const { return operator-(Dual(r)); }
    inline Dual operator*(T r) const { return operator*(Dual(r)); }
    inline Dual operator/(T r) const { return operator/(Dual(r)); }
    inline Dual operator+(const Dual& r) const { return Dual(_x + r._x, _dx + r._dx); }
    inline Dual operator-(const Dual& r) const { return Dual(_x - r._x, _dx - r._dx); }
    inline Dual operator*(const Dual& r) const { return Dual(_x*r._x, _dx*r._low() + r._dx*_low()); }
    inline Dual operator/(const Dual& r) const { return Dual(_x / r._x, (_dx*r._low() - r._dx*_low()) / (r._low()*r._low())); }
    inline Dual operator^(double k) const { return Dual(std::pow(_x, (T)k), _dx*Dual<T, O - 1>(k)*(_low() ^ ((T)(k - 1)))); }
    inline Dual operator^(const Dual& r) const { return Dual(std::pow(_x, r._x), (_low() ^ (r._low() - Dual<T, O - 1>(1)))*((r._low()*_dx) + (_low()*r._dx*Dual<T, O - 1>::log(_low())))); }
    inline Dual operator -(void) const { return Dual(-_x, -_dx); }
    inline Dual Abs() const { return Dual(std::abs(_x), _dx*SGNCOMPARE(_x, 0)); }
    inline Dual<T, O - 1> _low() const { return Dual<T, O - 1>(_x, _dx._low()); }

    inline T operator()(int derivative) const { if(!derivative) return _x; return _dx(derivative - 1); }
    //inline operator T() const { return _x; }
    template<typename U>
    inline Dual& operator=(const Dual<U, O>& r) { _x = (T)r._x; _dx = r._dx; return *this; }

    inline static Dual sin(const Dual& d) { return Dual(std::sin(d._x), d._dx*Dual<T, O - 1>::cos(d._low())); }
    inline static Dual cos(const Dual& d) { return Dual(std::cos(d._x), d._dx*(-Dual<T, O - 1>::sin(d._low()))); }
    inline static Dual exp(const Dual& d) { return Dual(std::exp(d._x), d._dx*(Dual<T, O - 1>::exp(d._low()))); }
    inline static Dual log(const Dual& d) { return Dual(std::log(d._x), d._dx / d._low()); }
    inline static Dual log10(const Dual& d) { return Dual(std::log10(d._x), d._dx / (Dual<T, O - 1>(std::log((T)10))*d._low())); }
    inline Dual sin() const { return sin(*this); }
    inline Dual cos() const { return cos(*this); }
    inline Dual exp() const { return exp(*this); }
    inline Dual log() const { return log(*this); }
    inline Dual log10() const { return log10(*this); }

    T _x;
    Dual<T, O - 1> _dx;
  };

  // Explicit specialization that terminates recursive order evaluation
  template<typename T>
  struct Dual<T, 0>
  {
    inline constexpr Dual(T x, T dx) : _x(x) {}
    inline constexpr Dual(T x = 0) : _x(x) {}
    inline Dual operator+(const Dual& r) const { return Dual(_x + r._x); }
    inline Dual operator-(const Dual& r) const { return Dual(_x - r._x); }
    inline Dual operator*(const Dual& r) const { return Dual(_x*r._x); }
    inline Dual operator/(const Dual& r) const { return Dual(_x / r._x); }
    inline Dual operator^(double k) const { return Dual(std::pow(_x, (T)k)); }
    inline Dual operator^(const Dual& r) const { return Dual(std::pow(_x, r._x)); }
    inline Dual operator -(void) const { return Dual(-_x); }

    inline T operator()(int derivative) const { assert(!derivative); return _x; }
    inline T _low() const { return 0; }

    inline static Dual sin(const Dual& d) { return Dual(std::sin(d._x)); }
    inline static Dual cos(const Dual& d) { return Dual(std::cos(d._x)); }
    inline static Dual exp(const Dual& d) { return Dual(std::exp(d._x)); }
    inline static Dual log(const Dual& d) { return Dual(std::log(d._x)); }
    inline static Dual log10(const Dual& d) { return Dual(std::log10(d._x)); }

    T _x;
  };

  // overloads for common math operators translated into static template functions (disabled because this blows stuff up)
  //template<typename T, int O>
  //const Dual<T,O> sin(const Dual<T,O>& d) { return Dual<T,O>::sin(d); }
  //template<typename T, int O>
  //const Dual<T,O> cos(const Dual<T,O>& d) { return Dual<T,O>::cos(d); }
  //template<typename T, int O>
  //const Dual<T,O> exp(const Dual<T,O>& d) { return Dual<T,O>::exp(d); }
  //template<typename T, int O>
  //const Dual<T,O> log(const Dual<T,O>& d) { return Dual<T,O>::log(d); }
  //template<typename T, int O>
  //const Dual<T,O> log10(const Dual<T,O>& d) { return Dual<T,O>::log10(d); }
}

// Global operator overloads
template<typename T, int O>
inline const bun::Dual<T, O> operator +(T l, const bun::Dual<T, O>& r) { return bun::Dual<T, O>(l) + r; }
template<typename T, int O>
inline const bun::Dual<T, O> operator -(T l, const bun::Dual<T, O>& r) { return bun::Dual<T, O>(l) - r; }
template<typename T, int O>
inline const bun::Dual<T, O> operator *(T l, const bun::Dual<T, O>& r) { return bun::Dual<T, O>(l)*r; }
template<typename T, int O>
inline const bun::Dual<T, O> operator /(T l, const bun::Dual<T, O>& r) { return bun::Dual<T, O>(l) / r; }

#endif
