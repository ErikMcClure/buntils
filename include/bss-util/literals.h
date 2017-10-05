// Copyright ©2014 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_LITERALS_H__
#define __BSS_LITERALS_H__

#include "bss_util.h"
#include "Dual.h"
#include "Rational.h"

constexpr BSS_FORCEINLINE long double operator "" _deg(long double d) noexcept { return d*(bss::PI / 180.0); }
constexpr BSS_FORCEINLINE long double operator "" _deg(unsigned long long d) noexcept { return d*(bss::PI/180.0); }
constexpr BSS_FORCEINLINE long double operator "" _rad(long double d) noexcept { return d*(180.0 / bss::PI); }
constexpr BSS_FORCEINLINE long double operator "" _rad(unsigned long long d) noexcept { return d*(180.0/ bss::PI); }
BSS_FORCEINLINE bss::Rational<int64_t> operator "" _recip(unsigned long long d) noexcept { return bss::Rational<int64_t>(1, d); }
constexpr BSS_FORCEINLINE size_t operator "" _sz(unsigned long long d) noexcept { return (size_t)d; }

#endif
