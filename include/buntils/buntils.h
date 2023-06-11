/* Erik McClure Utility Library
   Copyright (c)2023 Erik McClure

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef __BUN_UTIL_H__
#define __BUN_UTIL_H__

#include "buntils_c.h"
#include "Str.h"
#include "defines.h"
#include <assert.h>
#include <cmath>
#include <memory>
#include <string.h> // for memcmp
#include <emmintrin.h> // for SSE intrinsics
#include <float.h>
#include <string>
#include <stdio.h>
#include <array>
#include <limits>
#include <ostream>
#include <utility>
#ifdef BUN_PLATFORM_WIN32
#include <intrin.h>
#endif
#ifdef BUN_PLATFORM_POSIX
#include <stdlib.h> // For abs(int) on POSIX systems
#include <fpu_control.h> // for FPU control on POSIX systems
#endif

namespace bun {
  //Useful numbers
  constexpr double PI = 3.141592653589793238462643383279;
  constexpr double PI_HALF = PI*0.5;
  constexpr double PI_DOUBLE = PI*2.0;
  constexpr double E_CONST = 2.718281828459045235360287471352;
  constexpr double SQRT_TWO = 1.414213562373095048801688724209;
  constexpr float PIf = 3.141592653589793238462643383279f; // Convenience definitions
  constexpr float PI_HALFf = PIf*0.5f;
  constexpr float PI_DOUBLEf = PIf*2.0f;
  constexpr float E_CONSTf = 2.718281828459045235360287471352f;
  constexpr float SQRT_TWOf = 1.414213562373095048801688724209f;
  constexpr float FLT_EPS = 1.192092896e-07F;
  constexpr double DBL_EPS = 2.2204460492503131e-016;

  template<typename, typename = void>
  constexpr bool is_type_complete_v = false;

  template<typename T>
  constexpr bool is_type_complete_v<T, std::void_t<decltype(sizeof(T))>> = true;

  template<typename, typename = void>
  constexpr bool is_copy_constructible_or_incomplete_v = true;

  template<typename T>
  constexpr bool is_copy_constructible_or_incomplete_v<T, std::void_t<decltype(sizeof(T))>> = std::is_copy_constructible_v<T>;

  namespace internal {
    template<class T, bool B>
    struct __make_integral { using type = T; };
    template<class T>
    struct __make_integral<T, true> { using type = typename std::underlying_type<T>::type; };

    template<class> struct is_pair_array : std::false_type {};
    template<class K, class V> struct is_pair_array<std::pair<K, V>> : std::true_type {};
    template<class K, class V> struct is_pair_array<std::tuple<K, V>> : std::true_type {};
  }
  template<class T>
  struct make_integral : internal::__make_integral<T, std::is_enum<T>::value> {};

  template<size_t N>
  struct StringLiteral {
    constexpr StringLiteral(const char(&str)[N]) {
      std::copy_n(str, N, value);
    }

    char value[N];
  };

  // Get max size of an arbitrary number of bits, either signed or unsigned (assuming one's or two's complement implementation)
  template<uint8_t BITS>
  struct BitLimit
  {
    static constexpr uint16_t BYTES = ((T_CHARGETMSB(BITS) >> 3) << (0 + ((BITS % 8) > 0))) + (BITS < 8);
    typedef typename std::conditional<sizeof(char) == BYTES, char,  //rounds the type up if necessary.
      typename std::conditional<sizeof(short) == BYTES, short,
      typename std::conditional<sizeof(int) == BYTES, int,
      typename std::conditional<sizeof(int64_t) == BYTES, int64_t,
#ifdef BUN_HASINT128
      typename std::conditional<sizeof(__int128) == BYTES, __int128, void>::type>::type>::type>::type>::type SIGNED;
#else
      void > ::type > ::type > ::type > ::type SIGNED;
#endif
    using UNSIGNED = typename std::make_unsigned<SIGNED>::type;

    static constexpr UNSIGNED UNSIGNED_MIN = 0;
    static constexpr UNSIGNED UNSIGNED_MAX = (((UNSIGNED)2) << (BITS - 1)) - ((UNSIGNED)1); //these are all done carefully to ensure no overflow is ever utilized unless appropriate and it respects an arbitrary bit limit. We use 2<<(BITS-1) here to avoid shifting more bits than there are bits in the type.
    static constexpr SIGNED SIGNED_MIN_RAW = (SIGNED)(((UNSIGNED)1) << (BITS - 1)); // When we have normal bit lengths (8,16, etc) this will correctly result in a negative value in two's complement.
    static constexpr UNSIGNED SIGNED_MIN_HELPER = (UNSIGNED)(((UNSIGNED)~0) << (BITS - 1)); // However if we have unusual bit lengths (3,19, etc) the raw bit representation will be technically correct in the context of that sized integer, but since we have to round to a real integer size to represent the number, the literal interpretation will be wrong. This yields the proper minimum value.
    static constexpr SIGNED SIGNED_MIN = (SIGNED)SIGNED_MIN_HELPER;
    static constexpr SIGNED SIGNED_MAX = ((~SIGNED_MIN_RAW)&UNSIGNED_MAX);
  };
  template<typename T>
  struct TBitLimit : public BitLimit<sizeof(T) << 3> {};

  // Typesafe malloc implementation, for when you don't want to use New because you don't want constructors to be called.
  template<typename T>
  BUN_FORCEINLINE T* bun_Malloc(size_t sz) { return reinterpret_cast<T*>(malloc(sz * sizeof(T))); }
  template<typename T>
  BUN_FORCEINLINE T* bun_CAlloc(size_t sz) { return reinterpret_cast<T*>(calloc(sz, sizeof(T))); }

  // template inferred version of T_GETBIT and T_GETBITRANGE
  template<class T>
  inline constexpr T GetBitMask(int bit) noexcept { return T_GETBIT(T, bit); }
  template<class T>
  inline constexpr T GetBitMask(int low, int high) noexcept { return T_GETBITRANGE(T, low, high); }
  template<class T>
  BUN_FORCEINLINE T bun_SetBit(T word, T bit, bool v) noexcept { return (((word) & (~(bit))) | ((T)(-(typename std::make_signed<T>::type)v) & (bit))); }

  template<class T>
  BUN_FORCEINLINE constexpr bool bun_GetBit(T* p, size_t index) { return (p[index / (sizeof(T) << 3)] & (1 << (index % (sizeof(T) << 3)))) != 0; }

  // Replaces one character with another in a string
  template<typename T>
  inline T* strreplace(T* string, const T find, const T replace) noexcept
  {
    static_assert(std::is_integral<T>::value, "T must be integral");
    if(!string) return 0;
    size_t curpos = (size_t)~0; //this will wrap around to 0 when we increment

    while(string[++curpos] != '\0') //replace until the null-terminator
      if(string[curpos] == find)
        string[curpos] = replace;

    return string;
  }

#ifndef BUN_COMPILER_MSC
  template<int SZ>
  BUN_FORCEINLINE char* strcpyx0(char(&dst)[SZ], const char* src) { return strncpy(dst, src, SZ - 1); }
#endif

  // Counts number of occurences of character c in string, up to the null terminator
  template<typename T>
  inline size_t strccount(const T* string, T c) noexcept
  {
    static_assert(std::is_integral<T>::value, "T must be integral");
    size_t ret = 0;
    while(*string) { if(*string == c) ++ret; ++string; }
    return ret;
  }

  // Counts number of occurences of character c in string, up to length characters
  template<typename T>
  inline size_t strccount(const T* string, T c, size_t length) noexcept
  {
    static_assert(std::is_integral<T>::value, "T must be integral");
    size_t ret = 0;
    for(size_t i = 0; (i < length) && ((*string) != 0); ++i)
      if(string[i] == c) ++ret;
    return ret;
  }
  
  // This yields mathematically correct integer division (i/div) towards negative infinity, but only if div is a positive integer. This
  // implementation obviously must have integral types for T and D, but can be explicitely specialized to handle vectors or other structs.
  template<typename T, typename D>
  inline T IntDiv(T i, D div) noexcept
  {
    static_assert(std::is_integral<T>::value, "T must be integral");
    static_assert(std::is_integral<D>::value, "D must be integral");
    assert(div > 0);
    return (i / div) - ((i < 0)&((i%div) != 0)); // If i is negative and has a nonzero remainder, subtract one to correct the truncation.
  }

  // Performs a compile-time safe shift (positive number shifts left, negative number shifts right), preventing undefined behavior.
  template<typename T, int S>
  BUN_FORCEINLINE T SafeShift(T v) noexcept
  {
    if constexpr(S > 0)
      return (v << S);
    else if constexpr(S < 0)
      return (v >> (-S));
    else
      return v;
  }

  // Performs a mathematically correct modulo, unlike the modulo operator, which doesn't actually perform modulo, it performs a remainder operation. THANKS GUYS!
  template<typename T> //T must be integral
  BUN_FORCEINLINE T bun_Mod(T x, T m) noexcept
  {
    static_assert(std::is_signed<T>::value && std::is_integral<T>::value, "T must be a signed integral type or this function is pointless");
    x %= m;
    return (x + ((-(T)(x < 0))&m));
    //return (x+((x<0)*m)); // This is a tad slower
  }

  // Performs a mathematically correct floating point modulo, unlike fmod, which performs a remainder operation, not a modulo operation.
  template<typename T> //T must be floating point
  BUN_FORCEINLINE T bun_FMod(T x, T m) noexcept
  {
    static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
    return x - floor(x / m)*m;
  }

  // Trims space from left end of string by returning a different pointer. It is possible to use const char or const wchar_t as a type
  // here because the string itself is not modified.
  template<typename T>
  inline T* strltrim(T* str) noexcept
  {
    static_assert(std::is_integral<T>::value, "T must be integral");
    for(; *str > 0 && *str < 33; ++str);
    return str;
  }

  // Trims space from right end of string by inserting a null terminator in the appropriate location
  template<typename T>
  inline T* strrtrim(T* str) noexcept
  {
    static_assert(std::is_integral<T>::value, "T must be integral");
    T* inter = str + strlen(str);

    for(; inter > str && *inter < 33; --inter);
    *(++inter) = '\0';
    return str;
  }

  // Trims space from left and right ends of a string
  template<typename T>
  BUN_FORCEINLINE T* strtrim(T* str) noexcept
  {
    return strrtrim(strltrim(str));
  }

  template<typename T>
  inline int ToArgV(T** argv, T* cmdline)
  {
    int count = 0;
    while(*cmdline)
    {
      if(!isspace(*cmdline))
      {
        if(*cmdline == '"')
        {
          if(argv != 0) argv[count++] = cmdline + 1;
          else ++count;
          while(*++cmdline != 0 && cmdline[1] != 0 && (*cmdline != '"' || !isspace(cmdline[1])));
        }
        else
        {
          if(argv != 0) argv[count++] = cmdline;
          else ++count;
          while(*++cmdline != 0 && !isspace(*cmdline));
        }
        if(!*cmdline) break;
        if(argv != 0) *cmdline = 0;
      }
      ++cmdline;
    }
    return count;
  }

  // Processes an argv command line using the given function and divider. Use ToArgV if you only have a string (for windows command lines)
  template<typename F> //std::function<void(const char* const*,int num)>
  inline void ProcessCmdArgs(int argc, const char* const* argv, F && fn, char divider = '-')
  {
    if(!argc || !argv) return;
    const char* const* cur = argv;
    int len = 1;
    for(int i = 1; i < argc; ++i)
    {
      if(argv[i][0] == divider)
      {
        fn(cur, len);
        cur = argv + i;
        len = 1;
      }
      else
        ++len;
    }
    fn(cur, len);
  }

  // Converts 32-bit unicode int to a series of utf8 encoded characters, appending them to the string
  inline void OutputUnicode(std::string& s, int c) noexcept
  {
    if(c < 0x0080) s += c;
    else if(c < 0x0800) { s += (0xC0 | c >> (6 * 1)); s += (0x80 | (c & 0x3F)); }
    else if(c < 0x10000) { s += (0xE0 | c >> (6 * 2)); s += (0x80 | (c & 0x0FC0) >> (6 * 1)); s += (0x80 | (c & 0x3F)); }
    else if(c < 0x10FFFF) { s += (0xF0 | c >> (6 * 3)); s += (0x80 | (c & 0x03F000) >> (6 * 2)); s += (0x80 | (c & 0x0FC0) >> (6 * 1)); s += (0x80 | (c & 0x3F)); }
    // Otherwise this is an illegal codepoint so just ignore it
  }

  // Flips the endianness of a memory location
  BUN_FORCEINLINE void FlipEndian(std::byte* target, uint8_t n) noexcept
  {
    uint8_t end = (n >> 1);
    --n;
    for(uint8_t i = 0; i < end; ++i)
      std::swap(target[n - i], target[i]);
  }

  template<int I>
  BUN_FORCEINLINE void FlipEndian(std::byte* target) noexcept
  { 
    if constexpr(I == 2)
      std::swap(target[0], target[1]);
    else if constexpr(I > 2)
      FlipEndian(target, I); 
  }

  template<typename T>
  BUN_FORCEINLINE void FlipEndian(T* target) noexcept { FlipEndian<sizeof(T)>(reinterpret_cast<std::byte*>(target)); }

    // This is a bit-shift method of calculating the next number in the fibonacci sequence by approximating the golden ratio with 0.6171875 (1/2 + 1/8 - 1/128)
  template<typename T>
  BUN_FORCEINLINE constexpr T fbnext(T x) noexcept
  {
    static_assert(std::is_integral<T>::value, "T must be integral");
    return T_FBNEXT(x);
    //return x + 1 + (x>>1) + (x>>3) - (x>>7) + (x>>10) - (x>>13) - (x>>17) - (x>>21) + (x>>24); // 0.61803394 (but kind of pointless)
  }

  // Gets the sign of any integer (0 is assumed to be positive)
  template<typename T>
  BUN_FORCEINLINE constexpr T tsign(T n) noexcept
  {
    static_assert(std::is_integral<T>::value, "T must be a signed integer.");
    static_assert(std::is_signed<T>::value, "T must be a signed integer.");
    return 1 | (n >> ((sizeof(T) << 3) - 1));
  }

  // Gets the sign of any number, where a value of 0 returns 0
  template<typename T>
  BUN_FORCEINLINE constexpr char tsignzero(T n) noexcept
  {
    return (n > 0) - (n < 0);
  }

  // Returns -1.0 if the sign bit is negative, or 1.0 if it is positive.
  BUN_FORCEINLINE float fsign(float f) noexcept
  {
    union { float f; uint32_t i; } u = { f };
    u.i = (u.i&(1u << ((sizeof(float) << 3) - 1))) | 0x3f800000;
    return u.f;
  }

  BUN_FORCEINLINE double fsign(double f) noexcept
  {
    union { double f; uint64_t i; } u = { f };
    u.i = (u.i&(1ULL << ((sizeof(double) << 3) - 1))) | 0x3FF0000000000000;
    return u.f;
  }

  // Gets the shortest distance between two angles in radians
  template<typename T>
  BUN_FORCEINLINE T AngleDist(T a, T b) noexcept
  {
    static_assert(std::is_floating_point<T>::value, "T must be float, double, or long double");
    return ((T)PI) - fabs(fmod(fabs(a - b), ((T)PI_DOUBLE)) - ((T)PI));
  }

  // Gets the SIGNED shortest distance between two angles starting with (a - b) in radians
  template<typename T>
  BUN_FORCEINLINE T AngleDistSigned(T a, T b) noexcept
  {
    static_assert(std::is_floating_point<T>::value, "T must be float, double, or long double");
    return fmod(bun_FMod(b - a, (T)PI_DOUBLE) + ((T)PI), (T)PI_DOUBLE) - ((T)PI);
  }

  // Smart compilers will use SSE2 instructions to eliminate the massive overhead of int -> float conversions. This uses SSE2 instructions
  // to round a float to an integer using whatever the rounding mode currently is (usually it will be round-to-nearest)
  BUN_FORCEINLINE int32_t fFastRound(float f) noexcept
  {
#ifdef BUN_SSE_ENABLED
    return _mm_cvt_ss2si(_mm_load_ss(&f));
#else
    return (int32_t)roundf(f);
#endif
  }

  BUN_FORCEINLINE int32_t fFastRound(double f) noexcept
  {
#ifdef BUN_SSE_ENABLED
    return _mm_cvtsd_si32(_mm_load_sd(&f));
#else
    return (int32_t)round(f);
#endif
  }

  BUN_FORCEINLINE int32_t fFastTruncate(float f) noexcept
  {
#ifdef BUN_SSE_ENABLED
    return _mm_cvtt_ss2si(_mm_load_ss(&f));
#else
    return (int32_t)f;
#endif
  }

  BUN_FORCEINLINE int32_t fFastTruncate(double f) noexcept
  {
#ifdef BUN_SSE_ENABLED
    return _mm_cvttsd_si32(_mm_load_sd(&f));
#else
    return (int32_t)f;
#endif
  }

#if defined(BUN_CPU_x86) || defined(BUN_CPU_x86_64) || defined(BUN_CPU_IA_64)
#ifndef MSC_MANAGED
  BUN_FORCEINLINE void fSetRounding(bool nearest) noexcept
  {
    uint32_t a;
#ifdef BUN_PLATFORM_WIN32
    _controlfp_s(&a, nearest ? _RC_NEAR : _RC_CHOP, _MCW_RC);
#else
    _FPU_GETCW(a);
    a = (a&(~(_FPU_RC_NEAREST | _FPU_RC_DOWN | _FPU_RC_UP | _FPU_RC_ZERO))) | (nearest ? _FPU_RC_NEAREST : _FPU_RC_ZERO);
    _FPU_SETCW(a);
#endif
  }
  BUN_FORCEINLINE void fSetDenormal(bool on) noexcept
  {
    uint32_t a;
#ifdef BUN_PLATFORM_WIN32
    _controlfp_s(&a, on ? _DN_SAVE : _DN_FLUSH, _MCW_DN);
#else
    _FPU_GETCW(a); // Linux doesn't know what the denormal flags are, so we just hardcode the values in from windows' flags.
    a = (a&(~0x03000000)) | (on ? 0 : 0x01000000);
    _FPU_SETCW(a);
#endif
  }
#endif

  // Returns true if FPU is in single precision mode and false otherwise (false for both double and extended precision)
  BUN_FORCEINLINE bool FPUsingle() noexcept
  {
    uint32_t i;
#ifdef BUN_PLATFORM_WIN32
    i = _mm_getcsr();
#else
    _FPU_GETCW(i);
#endif
    return ((i&(0x0300)) == 0); //0x0300 is the mask for the precision bits, 0 indicates single precision
  }
#endif

  // Extremely fast rounding function that truncates properly, but only works in double precision mode; see http://stereopsis.com/FPU.html
  /*BUN_FORCEINLINE int32_t fFastDoubleRound(double val)
  {
    const double _double2fixmagic = 4503599627370496.0*1.5; //2^52 for 52 bits of mantissa
    assert(!FPUsingle());
    val		= val + _double2fixmagic;
    return ((int32_t*)&val)[0];
  }
  BUN_FORCEINLINE int32_t fFastDoubleRound(float val) { return fFastDoubleRound((double)val); }

  // Single precision version of the above function. While precision problems are mostly masked in the above function by limiting it to
  // int32_t, in this function they are far more profound due to there only being 24 bits of mantissa to work with. Use with caution.
  BUN_FORCEINLINE int32_t fFastSingleRound(double val)
  {
    const double _single2fixmagic = 16777216.0*1.5; //2^24 for 24 bits of mantissa
    assert(FPUsingle());
    val		= val + _single2fixmagic;
    return (int32_t)(((((uint64_t*)&val)[0])&0xFFFFF0000000)>>28);
  }
  BUN_FORCEINLINE int32_t fFastSingleRound(float val) { return fFastSingleRound((double)val); } */

  // This is a super fast floating point comparison function with a significantly higher tolerance and no
  // regard towards the size of the floats.
  inline bool fWideCompare(float leftf, float rightf) noexcept
  {
    union { float f; int32_t i; } left = { leftf };
    union { float f; int32_t i; } right = { rightf };
    uint8_t dif = abs((0x7F800000 & left.i) - (0x7F800000 & right.i)) >> 23; // This grabs the 8 exponent bits and subtracts them.
    if(dif > 1) // An exponent difference of 2 or greater means the numbers are different.
      return false;
    return !dif ? ((0x007FFF80 & left.i) == (0x007FFF80 & right.i)) : !(abs((0x007FFF80 & left.i) - (0x007FFF80 & right.i)) - 0x007FFF80); //If there is no difference in exponent we tear off the last 7 bits and compare the value, otherwise we tear off the last 7 bits, subtract, and then subtract the highest possible significand to compensate for the extra exponent.
  }

  // Highly optimized traditional tolerance based approach to comparing floating point numbers, found here: http://www.randydillon.org/Papers/2007/everfast.htm
  inline bool fCompare(float af, float bf, int32_t maxDiff = 1) noexcept
  {
    union { float f; int32_t i; } a = { af };
    union { float f; int32_t i; } b = { bf };
    //assert(af!=0.0f && bf!=0.0f); // Use fSmall for this
    int32_t test = (-(int32_t)(((uint32_t)(a.i^b.i)) >> 31));
    assert((0 == test) || (0xFFFFFFFF == (uint32_t)test));
    int32_t diff = ((a.i + test) ^ (test & 0x7fffffff)) - b.i;
    int32_t v1 = maxDiff + diff;
    int32_t v2 = maxDiff - diff;
    return (v1 | v2) >= 0;
  }

  inline bool fCompare(double af, double bf, int64_t maxDiff = 1) noexcept
  {
    union { double f; int64_t i; } a = { af };
    union { double f; int64_t i; } b = { bf };
    //assert(af!=0.0 && bf!=0.0); // Use fSmall for this
    int64_t test = (-(int64_t)(((uint64_t)(a.i^b.i)) >> 63));
    assert((0 == test) || (0xFFFFFFFFFFFFFFFF == (uint64_t)test));
    int64_t diff = ((a.i + test) ^ (test & 0x7fffffffffffffff)) - b.i;
    int64_t v1 = maxDiff + diff;
    int64_t v2 = maxDiff - diff;
    return (v1 | v2) >= 0;
  }

  // This determines if a float is sufficiently close to 0
  BUN_FORCEINLINE bool fSmall(float f, float eps = FLT_EPS) noexcept
  {
    union { float f; uint32_t i; } u = { f };
    union { float f; uint32_t i; } e = { eps };
    u.i = (u.i & 0x7FFFFFFF); //0x7FFFFFFF strips off the sign bit (which is always the highest bit)
    return u.i <= e.i;
  }

  // This determines if a double is sufficiently close to 0
  BUN_FORCEINLINE bool fSmall(double f, double eps = DBL_EPS) noexcept
  {
    union { double f; uint64_t i; } u = { f };
    union { double f; uint64_t i; } e = { eps };
    u.i = (u.i & 0x7FFFFFFFFFFFFFFF); //0x7FFFFFFFFFFFFFFF strips off the sign bit (which is always the highest bit)
    return u.i <= e.i;
  }

  inline bool fCompareSmall(float af, float bf, int32_t maxDiff = 1, float eps = FLT_EPS) noexcept
  {
    if(af == 0.0) return fSmall(bf, eps);
    if(bf == 0.0) return fSmall(af, eps);
    return fCompare(af, bf, maxDiff);
  }

  inline bool fCompareSmall(double af, double bf, int64_t maxDiff = 1, double eps = DBL_EPS) noexcept
  {
    if(af == 0.0) return fSmall(bf, eps);
    if(bf == 0.0) return fSmall(af, eps);
    return fCompare(af, bf, maxDiff);
  }

  // This is a super fast length approximation for 2D coordinates; See http://www.azillionmonkeys.com/qed/sqroot.html for details (Algorithm by Paul Hsieh)
  inline float fLength(float x, float y) noexcept
  {
    x = fabs(x);
    y = fabs(y);
    //(1 + 1/(4-2*sqrt(2)))/2 = 0.92677669529663688
    float hold = 0.7071067811865475f*(x + y), mval = (x > y) ? x : y;
    return 0.92677669529663688f * ((hold > mval) ? hold : mval);
  }

  // The classic fast square root approximation, which is often mistakenly attributed to John Carmack. The algorithm is in fact over 15 years old and no one knows where it came from.
  inline float fFastSqrt(float number) noexcept
  {
    const float f = 1.5F;

    float x = number * 0.5F;
    union { float f; int32_t i; } y = { number };
    y.i = 0x5f3759df - (y.i >> 1);
    y.f = y.f * (f - (x * y.f * y.f));
    y.f = y.f * (f - (x * y.f * y.f)); //extra iteration for added accuracy
    return number * y.f;
  }

  // Adaptation of the class fast square root approximation for double precision, based on http://www.azillionmonkeys.com/qed/sqroot.html
  inline double dFastSqrt(double number) noexcept
  {
    const double f = 1.5;
    uint32_t* i;
    double x, y;

    x = number*0.5;
    y = number;
    i = reinterpret_cast<uint32_t*>(&y) + 1;
    *i = (0xbfcdd90a - *i) >> 1; // estimate of 1/sqrt(number)

    y = y * (f - (x * y * y));
    y = y * (f - (x * y * y));
    y = y * (f - (x * y * y)); //Newton raphson converges quadratically, so one additional iteration doubles the precision
    //y = y * ( f - ( x * y * y ) ); 
    return number * y;
  }

  // bit-twiddling based method of calculating an integral square root from Wilco Dijkstra - http://www.finesse.demon.co.uk/steven/sqrt.html
  template<typename T, size_t bits> // WARNING: bits should be HALF the actual number of bits in (T)!
  inline T IntFastSqrt(T n) noexcept
  {
    static_assert(std::is_integral<T>::value, "T must be integral");
    T root = 0, t;

    for(size_t i = bits; i > 0;)
    {
      --i;
      t = ((root + (((T)1) << (i))) << (i));
      if(n >= t)
      {
        n -= t;
        root |= 2 << (i);
      }
    }
    return root >> 1;
  }
  template<typename T>
  BUN_FORCEINLINE T IntFastSqrt(T n) noexcept
  {
    return IntFastSqrt<T, sizeof(T) << 2>(n); //done to ensure loop gets unwound (the bit conversion here is <<2 because the function wants HALF the bits in T, so <<3 >>1 -> <<2)
  }

  template<typename T> //assumes integer type if not one of the floating point types
  BUN_FORCEINLINE T FastSqrt(T n) noexcept { return IntFastSqrt(n); } //Picks correct method for calculating any square root quickly
  template<> BUN_FORCEINLINE float FastSqrt(float n) noexcept { return fFastSqrt(n); }
  template<> BUN_FORCEINLINE double FastSqrt(double n) noexcept { return dFastSqrt(n); }
  template<> BUN_FORCEINLINE long double FastSqrt(long double n) noexcept { return dFastSqrt((double)n); }

  // Distance calculation (squared)
  template<typename T>
  BUN_FORCEINLINE constexpr T DistSqr(T X, T Y, T x, T y) noexcept
  {
    T tx = X - x, ty = Y - y; return (tx*tx) + (ty*ty); //It doesn't matter if you use temporary values for floats, but it does if you use ints (for unknown reasons)
  }

  // Distance calculation
  template<typename T>
  BUN_FORCEINLINE T Dist(T X, T Y, T x, T y) noexcept
  {
    return FastSqrt<T>(DistSqr<T>(X, Y, x, y));
  }

  // Average aggregation without requiring a total variable that can overflow. Nextnum should be the current avg count incremented by 1.
  template<typename T, typename CT_> requires (std::is_integral<CT_>::value && std::is_floating_point<T>::value)
  BUN_FORCEINLINE constexpr T bun_Avg(T curavg, T nvalue, CT_ nextnum) noexcept
  { // USAGE: avg = bun_Avg<double, int>(avg, value, ++total);
    return curavg + ((nvalue - curavg) / (T)nextnum);
  }

  // Sum of squares of differences aggregation using an algorithm by Knuth. Nextnum should be the current avg count incremented by 1.
  template<typename T, typename CT_> requires (std::is_integral<CT_>::value&& std::is_floating_point<T>::value)
  BUN_FORCEINLINE constexpr void bun_Variance(T& curvariance, T& avg, T nvalue, CT_ nextnum) noexcept
  { // USAGE: bun_Variance<double, int>(variance, avg, value, ++total); Then use sqrt(variance/(n-1)) to get the actual standard deviation
    T delta = nvalue - avg;
    avg += delta / (T)nvalue;
    curvariance += delta*(nvalue - avg);
  }

  // Searches an arbitrary series of bytes for another arbitrary series of bytes
  inline const void* ByteSearch(const void* search, size_t length, const void* find, size_t fLength) noexcept
  {
    if(!search || !length || !find || !fLength || length < fLength) return 0;

    uint8_t* s = (uint8_t*)search;
    length -= fLength;
    for(size_t i = 0; i <= length; ++i) // i <= length works because of the above length-=fLength
    {
      search = s + i;
      if(!memcmp(search, find, fLength))
        return search;
    }
    return 0;
  }

  BUN_FORCEINLINE void* ByteSearch(void* search, size_t length, void* find, size_t fLength) noexcept
  {
    return const_cast<void*>(ByteSearch((const void*)search, length, (const void*)find, fLength));
  }

  // Counts the number of bits in v (up to 128-bit types) using the parallel method detailed here: http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
  template<typename Tx> requires std::is_integral<typename make_integral<Tx>::type>::value
  inline constexpr uint8_t BitCount(Tx v) noexcept
  {
    using T = typename make_integral<Tx>::type;
    v = v - ((v >> 1) & (T)~(T)0 / 3);                           // temp
    v = (v & (T)~(T)0 / 15 * 3) + ((v >> 2) & (T)~(T)0 / 15 * 3);      // temp
    v = (v + (v >> 4)) & (T)~(T)0 / 255 * 15;                      // temp
    return (uint8_t)((T)(v * ((T)~(T)0 / 255)) >> (sizeof(T) - 1) * (sizeof(char) << 3));
  }

  //Unlike FastSqrt, these are useless unless you are on a CPU without SSE instructions, or have a terrible std implementation.
  //// Fast sin function with 0.078% error when extra precision is left in. See http://www.devmaster.net/forums/showthread.php?t=5784
  //inline float FastSin(float x)
  //{
  //  x-= (int)(x*(1/(float)PI_DOUBLE))*(float)PI_DOUBLE;
  //  const float B = 4/(float)PI;
  //  const float C = -4/(float)PI_DOUBLE;

  //  float y = B * x + C * x * fabs(x);

  //  //const float Q = 0.775f;
  //  const float P = 0.225f;

  //  return (P * (y * fabs(y) - y) + y);   // Q * y + P * y * fabs(y) (extra precision)
  //}

  //inline float FastCos(float x)
  //{
  //  return FastSin(x+(float)PI_HALF);
  //}
  template<typename T, bool nullterminate = false>
  BUN_FORCEINLINE std::pair<std::unique_ptr<T[]>, size_t> LoadFile(const char* file)
  {
    FILE* f = 0;
    WFOPEN(f, file, "rb");
    if(!f) return std::pair<std::unique_ptr<T[]>, size_t>(nullptr, 0);
    fseek(f, 0, SEEK_END);
    size_t ln = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::unique_ptr<T[]> a(new T[ln + !!nullterminate]);
    fread(a.get(), sizeof(T), ln, f);
    fclose(f);
    if constexpr(nullterminate)
      a[ln] = 0;
    return std::pair<std::unique_ptr<T[]>, size_t>(std::move(a), ln + !!nullterminate);
  }
  // Round a number up to the next power of 2 (32 -> 32, 33 -> 64, etc.)
  inline constexpr uint64_t NextPow2(uint64_t v) noexcept
  {
    v -= 1;
    v |= (v >> 1);
    v |= (v >> 2);
    v |= (v >> 4);
    v |= (v >> 8);
    v |= (v >> 16);
    v |= (v >> 32);

    return v + 1;
  }
  inline constexpr uint32_t NextPow2(uint32_t v) noexcept
  {
    v -= 1;
    v |= (v >> 1);
    v |= (v >> 2);
    v |= (v >> 4);
    v |= (v >> 8);
    v |= (v >> 16);

    return v + 1;
  }
  inline constexpr uint16_t NextPow2(uint16_t v) noexcept
  {
    v -= 1;
    v |= (v >> 1);
    v |= (v >> 2);
    v |= (v >> 4);
    v |= (v >> 8);

    return v + 1;
  }
  inline constexpr uint8_t NextPow2(uint8_t v) noexcept
  {
    v -= 1;
    v |= (v >> 1);
    v |= (v >> 2);
    v |= (v >> 4);

    return v + 1;
  }

#ifdef BUN_COMPILER_MSC
  inline constexpr uint32_t bun_Log2(uint32_t v) noexcept
  {
    if(!v) return 0;
    unsigned long r;
    if (std::is_constant_evaluated()) {
      const uint32_t b[] = { 0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000 };
      const uint32_t S[] = { 1, 2, 4, 8, 16 };

      if (v & b[4]) { v >>= S[4]; r |= S[4]; }
      if (v & b[3]) { v >>= S[3]; r |= S[3]; }
      if (v & b[2]) { v >>= S[2]; r |= S[2]; }
      if (v & b[1]) { v >>= S[1]; r |= S[1]; }
      if (v & b[0]) { v >>= S[0]; r |= S[0]; }
    }
    else {
      _BitScanReverse(&r, v);
    }
    return r;
  }
#elif defined(BUN_COMPILER_GCC)
  inline uint32_t constexpr bun_Log2(uint32_t v) noexcept { 
    if (std::is_constant_evaluated()) {
      const uint32_t b[] = { 0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000 };
      const uint32_t S[] = { 1, 2, 4, 8, 16 };

      uint32_t r = 0;
      if (v & b[4]) { v >>= S[4]; r |= S[4]; }
      if (v & b[3]) { v >>= S[3]; r |= S[3]; }
      if (v & b[2]) { v >>= S[2]; r |= S[2]; }
      if (v & b[1]) { v >>= S[1]; r |= S[1]; }
      if (v & b[0]) { v >>= S[0]; r |= S[0]; }
      return r;
    }
    else {
      return !v ? 0 : ((sizeof(uint32_t) << 3) - 1 - __builtin_clz(v));
    }
  }
#else
  // Bit-twiddling hack for base 2 log by Sean Eron Anderson
  inline uint32_t constexpr bun_Log2(uint8_t v) noexcept
  {
    const uint32_t b[] = { 0x2, 0xC, 0xF0 };
    const uint32_t S[] = { 1, 2, 4 };

    uint32_t r = 0; // result of bun_Log2(v) will go here
    if(v & b[2]) { v >>= S[2]; r |= S[2]; }
    if(v & b[1]) { v >>= S[1]; r |= S[1]; }
    if(v & b[0]) { v >>= S[0]; r |= S[0]; }

    return r;
  }
  inline uint32_t constexpr bun_Log2(uint16_t v) noexcept
  {
    const uint32_t b[] = { 0x2, 0xC, 0xF0, 0xFF00 };
    const uint32_t S[] = { 1, 2, 4, 8 };

    uint32_t r = 0; // result of bun_Log2(v) will go here
    if(v & b[3]) { v >>= S[3]; r |= S[3]; }
    if(v & b[2]) { v >>= S[2]; r |= S[2]; }
    if(v & b[1]) { v >>= S[1]; r |= S[1]; }
    if(v & b[0]) { v >>= S[0]; r |= S[0]; }

    return r;
  }

  inline uint32_t constexpr bun_Log2(uint32_t v) noexcept
  {
    const uint32_t b[] = { 0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000 };
    const uint32_t S[] = { 1, 2, 4, 8, 16 };

    uint32_t r = 0; // result of bun_Log2(v) will go here
    if(v & b[4]) { v >>= S[4]; r |= S[4]; }
    if(v & b[3]) { v >>= S[3]; r |= S[3]; }
    if(v & b[2]) { v >>= S[2]; r |= S[2]; }
    if(v & b[1]) { v >>= S[1]; r |= S[1]; }
    if(v & b[0]) { v >>= S[0]; r |= S[0]; }

    return r;
  }
#endif
  inline uint32_t constexpr bun_Log2(uint64_t v) noexcept
  {
#if (defined(BUN_COMPILER_MSC) || defined(BUN_COMPILER_GCC)) && defined(BUN_64BIT)
    if (std::is_constant_evaluated()) 
#endif
    {
      const uint64_t b[] = { 0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000, 0xFFFFFFFF00000000 };
      const uint32_t S[] = { 1, 2, 4, 8, 16, 32 };

      uint32_t r = 0; // result of bun_Log2(v) will go here
      if (v & b[5]) { v >>= S[5]; r |= S[5]; }
      if (v & b[4]) { v >>= S[4]; r |= S[4]; }
      if (v & b[3]) { v >>= S[3]; r |= S[3]; }
      if (v & b[2]) { v >>= S[2]; r |= S[2]; }
      if (v & b[1]) { v >>= S[1]; r |= S[1]; }
      if (v & b[0]) { v >>= S[0]; r |= S[0]; }
      return r;
    }
#if defined(BUN_COMPILER_MSC) && defined(BUN_64BIT)
    else
    {
      if (!v) return 0;
      unsigned long r;
      _BitScanReverse64(&r, v);
      return r;
    }
#elif defined(BUN_COMPILER_GCC) && defined(BUN_64BIT)
    else
    {
      uint32_t r = !v ? 0 : ((sizeof(uint64_t) << 3) - 1 - __builtin_clz(v));
      return r;
    }
#endif
  }
  inline uint32_t bun_Log2_p2(uint32_t v) noexcept //Works only if v is a power of 2
  {
    assert(v && !(v & (v - 1))); //debug version checks to ensure its a power of two
#ifdef BUN_COMPILER_MSC
    unsigned long r;
    _BitScanReverse(&r, v);
#elif defined(BUN_COMPILER_GCC)
    uint32_t r = (sizeof(uint32_t) << 3) - 1 - __builtin_clz(v);
#else
    const uint32_t b[] = { 0xAAAAAAAA, 0xCCCCCCCC, 0xF0F0F0F0, 0xFF00FF00, 0xFFFF0000 };
    uint32_t r = (v & b[0]) != 0;
    r |= ((v & b[4]) != 0) << 4;
    r |= ((v & b[3]) != 0) << 3;
    r |= ((v & b[2]) != 0) << 2;
    r |= ((v & b[1]) != 0) << 1;
#endif
    return r;
  }

  template<class T, bool U = std::is_signed<T>::value>
  BUN_FORCEINLINE typename std::enable_if<U, typename std::make_unsigned<T>::type>::type bun_Abs(T x) noexcept
  {
    static_assert(std::is_signed<T>::value, "T must be signed for this to work properly.");
    T const mask = x >> ((sizeof(T) << 3) - 1); // Uses a bit twiddling hack to take absolute value without branching: https://graphics.stanford.edu/~seander/bithacks.html#IntegerAbs
    return (x + mask) ^ mask;
  }

  template<class T, bool U = std::is_signed<T>::value>
  BUN_FORCEINLINE typename std::enable_if<!U, T>::type bun_Abs(T x) noexcept { return x; }

  template<class T>
  inline typename std::make_signed<T>::type bun_Negate(T x, char negate) noexcept
  {
    static_assert(std::is_unsigned<T>::value, "T must be unsigned for this to work properly.");
    return (x ^ -negate) + negate;
  }

  namespace internal {
    // Double width multiplication followed by a right shift and truncation.
    template<class T>
    inline T _bunmultiplyextract(T xs, T ys, T shift) noexcept
    {
      using U = typename std::make_unsigned<T>::type;
      U x = bun_Abs<T>(xs);
      U y = bun_Abs<T>(ys);
      static const U halfbits = (sizeof(U) << 2);
      static const U halfmask = ((U)~0) >> halfbits;
      U a = x >> halfbits, b = x & halfmask;
      U c = y >> halfbits, d = y & halfmask;

      U ac = a * c;
      U bc = b * c;
      U ad = a * d;
      U bd = b * d;

      U mid34 = (bd >> halfbits) + (bc & halfmask) + (ad & halfmask);

      U high = ac + (bc >> halfbits) + (ad >> halfbits) + (mid34 >> halfbits); // high
      U low = (mid34 << halfbits) | (bd & halfmask); // low
      if constexpr(std::is_signed<T>::value)
      {
        if((xs < 0) ^ (ys < 0))
        {
          high = ~high + !low; // only add one if the x addition would overflow, which can only happen if x is the maximum value
          low = ~low + 1;
        }
      }

      if(shift >= (T)(sizeof(U) << 3))
        return ((T)high) >> (shift - (sizeof(U) << 3));
      low = (low >> shift);
      high = (high << ((sizeof(T) << 3) - shift)) & (-(shift > 0)); // shifting left by 64 bits is undefined, so we use a bit trick to set high to zero if shift is 0 without branching.
      return (T)(low | high);
    }
  }
  template<class T>
  BUN_FORCEINLINE T bun_MultiplyExtract(T x, T y, T shift) noexcept
  {
    using U = typename std::conditional<std::is_signed<T>::value, typename BitLimit<sizeof(T) << 4>::SIGNED, typename BitLimit<sizeof(T) << 4>::UNSIGNED>::type;
    return (T)(((U)x * (U)y) >> shift);
  }
#ifndef BUN_HASINT128
  template<>
  BUN_FORCEINLINE int64_t bun_MultiplyExtract<int64_t>(int64_t x, int64_t y, int64_t shift) noexcept { return internal::_bunmultiplyextract<int64_t>(x, y, shift); }
  template<>
  BUN_FORCEINLINE uint64_t bun_MultiplyExtract<uint64_t>(uint64_t x, uint64_t y, uint64_t shift) noexcept { return internal::_bunmultiplyextract<uint64_t>(x, y, shift); }
#endif

  template<class I>
  inline I DaysFromCivil(I y, unsigned m, unsigned d) noexcept
  {
    static_assert(std::numeric_limits<unsigned>::digits >= 18, "This algorithm has not been ported to a 16 bit unsigned integer");
    static_assert(std::numeric_limits<I>::digits >= 20, "This algorithm has not been ported to a 16 bit signed integer");
    y -= m <= 2;
    const I era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);      // [0, 399]
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;  // [0, 365]
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;         // [0, 146096]
    return era * 146097 + static_cast<I>(doe) - 719468;
  }

  // Generic function application to an array
  template<class T, size_t I, T(*F)(T, T)>
  BUN_FORCEINLINE std::array<T, I> ArrayMap(const std::array<T, I>& l, const std::array<T, I>& r) noexcept
  {
    std::array<T, I> x;
    for(size_t i = 0; i < I; ++i)
      x[i] = F(l[i], r[i]);
    return x;
  }

  // Basic lerp function with no bounds checking
  template<class T, class D = double>
  BUN_FORCEINLINE constexpr T lerp(T a, T b, D t) noexcept
  {
    return T((D(1.0) - t)*a) + T(t*b);
    //return a+((T)((b-a)*t)); // This is susceptible to floating point errors when t = 1
  }

  // post: https://notes.underscorediscovery.com/constexpr-fnv1a/
  inline constexpr uint32_t hash_fnv1a(const char* const str, const uint32_t value = 0x811c9dc5) noexcept {
    return (str[0] == '\0') ? value : hash_fnv1a(&str[1], uint32_t(uint64_t(value ^ uint32_t(str[0])) * 0x1000193ULL));
  }

  inline constexpr uint64_t hash64_fnv1a(const char* const str, const uint64_t value = 0xcbf29ce484222325) noexcept {
    return (str[0] == '\0') ? value : hash64_fnv1a(&str[1], uint32_t(uint64_t(value ^ uint64_t(str[0])) * 0x100000001b3ULL));
  }

  // template helper is_specialization_of, which can check if T is a specialization of any type that takes a variadic number of arguments (variant, tuple, etc.)
  template <typename T, template <typename...> class Template>
  struct is_specialization_of : std::false_type {};
  template <template <typename...> class Template, typename... Args>
  struct is_specialization_of<Template<Args...>, Template> : std::true_type {};
  
  template <typename T>
  struct is_specialization_of_array : std::false_type {};
  template <typename T, int N>
  struct is_specialization_of_array<std::array<T, N>> : std::true_type {};

  // Implements remove_cvref from the C++20 standard
  template<class T> struct remove_cvref { using type = std::remove_cv_t<std::remove_reference_t<T>>; };
  template<class T> using remove_cvref_t = typename remove_cvref<T>::type;

  // Type safe memset functions
  template<typename T>
  BUN_FORCEINLINE void bun_Fill(T& p, unsigned char val = 0)
  {
    memset(&p, val, sizeof(T));
  }
  template<typename T, int I>
  BUN_FORCEINLINE void bun_Fill(T (&p)[I], int val = 0)
  {
    memset(p, val, sizeof(T)*I);
  }
  template<typename T, int I>
  BUN_FORCEINLINE void bun_Fill(std::array<T, I>& p, int val = 0)
  {
    memset(p.data(), val, sizeof(T)*I);
  }
  template<typename T, int I, int J>
  BUN_FORCEINLINE void bun_Fill(T(&p)[I][J], int val = 0)
  {
    memset(p, val, sizeof(T)*I*J);
  }
  template<typename T>
  BUN_FORCEINLINE void bun_FillN(T* p, size_t n, unsigned char val = 0)
  {
    memset(p, val, sizeof(T)*n);
  }

  template<class T, class SUBCLASS>
  void memsubset(T* p, int v) { memset(reinterpret_cast<char*>(p) + sizeof(SUBCLASS), v, sizeof(T) - sizeof(SUBCLASS)); }
  template<class T, class SUBCLASS>
  void memsubcpy(T* dest, const T* src)
  {
    MEMCPY(((char*)dest) + sizeof(SUBCLASS), sizeof(T) - sizeof(SUBCLASS), ((char*)src) + sizeof(SUBCLASS), sizeof(T) - sizeof(SUBCLASS));
  }

  BUN_COMPILER_DLLEXPORT extern void bun_DLLDeleteFunc(void* p);

  //unique_ptr deleter class that forces the deletion to occur in this DLL
  template<class _Ty>
  struct bun_DLLDelete
  {	// default deleter for unique_ptr
    using _Myt = bun_DLLDelete<_Ty>;

    void operator()(_Ty *_Ptr) const
    {
      if(0 < sizeof(_Ty))
      { // won't compile for incomplete type
        _Ptr->~_Ty(); // call destructor because delete won't
        bun_DLLDeleteFunc(_Ptr);
      }
    }
  };

  template<class _Ty>
  struct bun_DLLDelete<_Ty[]>
  {	// default deleter for unique_ptr to array of unknown size
    using _Myt = bun_DLLDelete<_Ty>;

    void operator()(_Ty *_Ptr) const
    {
      if(0 < sizeof(_Ty))
      { // won't compile for incomplete type
        _Ptr->~_Ty(); // call destructor because delete won't
        bun_DLLDeleteFunc(_Ptr);
      }
    }
  };
}

#endif

  // Simple 128bit integer for x86 instruction set. Replaced with __int128 if possible
//#ifdef BUN32BIT
//  struct int128
//  {
//    inline static int128& operator+=(const int128& right) { _add(right.ints[0],right.ints[1],right.ints[2],right.ints[3]); return *this; }
//
//    int32_t ints[4];
//
//  private:
//    inline static void _add(int32_t a,int32_t b,int32_t c,int32_t d) //Done because I suck with assembly :D
//    {
//      __asm {
//#ifdef BUN_NO_FASTCALL //if we are using fastcall we don't need these instructions
//        mov ECX, a
//        mov EDX, b
//#endif
//        mov EAX, c
//        mov EBX, d
//        add ints[0], ECX
//        adc ints[1], EDX
//        adc ints[2], EAX
//        adc ints[3], EBX
//      }
//    }
//  };
//  struct uint128
//  {
//    inline static uint128& operator+=(const uint128& right) { _add(right.ints[0],right.ints[1],right.ints[2],right.ints[3]); return *this; }
//
//    uint32_t ints[4];
//
//  private:
//    inline static void _add(uint32_t a,uint32_t b,uint32_t c,uint32_t d) //Done because I suck with assembly :D
//    {
//      __asm {
//#ifdef BUN_NO_FASTCALL //if we are using fastcall we don't need these instructions
//        mov ECX, a
//        mov EDX, b
//#endif
//        mov EAX, c
//        mov EBX, d
//        add ints[0], ECX
//        adc ints[1], EDX
//        adc ints[2], EAX
//        adc ints[3], EBX
//      }
//    }
//  };
//#else
//  typedef __int128 int128;
//  typedef unsigned __int128 uint128;
//#endif
