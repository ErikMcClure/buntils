// Copyright ©2014 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BUN_LITERALS_H__
#define __BUN_LITERALS_H__

#include "buntils.h"
#include "Dual.h"
#include "Rational.h"

constexpr BUN_FORCEINLINE long double operator "" _deg(long double d) noexcept { return d*(bun::PI / 180.0); }
constexpr BUN_FORCEINLINE long double operator "" _deg(unsigned long long d) noexcept { return d*(bun::PI/180.0); }
constexpr BUN_FORCEINLINE long double operator "" _rad(long double d) noexcept { return d*(180.0 / bun::PI); }
constexpr BUN_FORCEINLINE long double operator "" _rad(unsigned long long d) noexcept { return d*(180.0/ bun::PI); }
BUN_FORCEINLINE bun::Rational<int64_t> operator "" _recip(unsigned long long d) noexcept { return bun::Rational<int64_t>(1, d); }
constexpr BUN_FORCEINLINE size_t operator "" _sz(unsigned long long d) noexcept { return (size_t)d; }

#endif
