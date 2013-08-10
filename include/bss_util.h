/* Black Sphere Studios Utility Library
   Copyright ©2013 Black Sphere Studios

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

#ifndef __BSS_UTIL_H__
#define __BSS_UTIL_H__

#include "bss_util_c.h"
#include <assert.h>
#include <math.h>
#include <memory>
//#include <type_traits>
//#include <utility>
//#include <cstring> // for memcmp
#ifdef BSS_COMPILER_GCC
#include <stdlib.h> // For abs(int) on GCC
#endif
#ifdef BSS_MSC_NOASM
#include <intrin.h>
#endif

namespace bss_util { 
  static const VersionType BSSUTIL_VERSION = { BSS_VERSION_MAJOR,BSS_VERSION_MINOR,BSS_VERSION_REVISION };
  
  BSS_COMPILER_DLLEXPORT extern void BSS_FASTCALL SetWorkDirToCur(); //Sets the working directory to the actual goddamn location of the EXE instead of the freaking start menu, or possibly the desktop. The possibilities are endless! Fuck you, windows.
  BSS_COMPILER_DLLEXPORT extern void BSS_FASTCALL ForceWin64Crash(); // I can't believe this function exists (forces 64-bit windows to not silently ignore fatal errors)
  BSS_COMPILER_DLLEXPORT extern unsigned long long BSS_FASTCALL bssFileSize(const char* path);
  BSS_COMPILER_DLLEXPORT extern unsigned long long BSS_FASTCALL bssFileSize(const wchar_t* path);
  BSS_COMPILER_DLLEXPORT extern long BSS_FASTCALL GetTimeZoneMinutes(); //Returns the current time zone difference from UTC in minutes

  //Useful numbers
  const double PI = 3.141592653589793238462643383279;
  const double PI_HALF = PI*0.5;
  const double PI_DOUBLE = PI*2.0;  
  const double E_CONST = 2.718281828459045235360287471352;
  const double SQRT_TWO = 1.414213562373095048801688724209;
  const float PIf = 3.141592653589793238462643383279f; // Convenience definitions
  const float PI_HALFf = PIf*0.5f;
  const float PI_DOUBLEf = PIf*2.0f;  
  const float E_CONSTf = 2.718281828459045235360287471352f;
  const float SQRT_TWOf = 1.414213562373095048801688724209f;
  const float FLT_EPS = 1.192092896e-07F;
  const double DBL_EPS = 2.2204460492503131e-016;
  const __int32 FLT_INTEPS = *(__int32*)(&FLT_EPS);
  const __int64 DBL_INTEPS = *(__int64*)(&DBL_EPS);

  // Given the size of a type, lets you return the signed or unsigned integral equivelent
  template<int bytes> struct TSignPick {};
  template<> struct TSignPick<1> { typedef char SIGNED; typedef unsigned char UNSIGNED; };
  template<> struct TSignPick<2> { typedef __int16 SIGNED; typedef unsigned __int16 UNSIGNED; };
  template<> struct TSignPick<4> { typedef __int32 SIGNED; typedef unsigned __int32 UNSIGNED; };
  template<> struct TSignPick<8> { typedef __int64 SIGNED; typedef unsigned __int64 UNSIGNED; };
#ifdef BSS_COMPILER_GCC
  template<> struct TSignPick<16> { typedef __int128 SIGNED; typedef unsigned __int128 UNSIGNED; };
#endif 

  // Get max size of an arbitrary number of bits, either signed or unsigned (assuming one's or two's complement implementation)
  template<unsigned char BITS>
  struct ABitLimit
  {
    typedef typename TSignPick<((T_CHARGETMSB(BITS)>>3) << (0+((BITS%8)>0))) + (BITS<8)>::SIGNED SIGNED; //rounds the type up if necessary.
    typedef typename TSignPick<((T_CHARGETMSB(BITS)>>3) << (0+((BITS%8)>0))) + (BITS<8)>::UNSIGNED UNSIGNED;

    static const UNSIGNED UNSIGNED_MIN=0;
    static const UNSIGNED UNSIGNED_MAX=(((UNSIGNED)2)<<(BITS-1))-((UNSIGNED)1); //these are all done carefully to ensure no overflow is ever utilized unless appropriate and it respects an arbitrary bit limit. We use 2<<(BITS-1) here to avoid shifting more bits than there are bits in the type.
    static const SIGNED SIGNED_MIN_RAW=(((SIGNED)1)<<(BITS-1)); // When we have normal bit lengths (8,16, etc) this will correctly result in a negative value in two's complement.
#ifdef BSS_COMPILER_GCC
    static const SIGNED SIGNED_MIN=-((__int128)SIGNED_MIN_RAW); // GCC is a pendantic fuckwad that treats signed overflow as undefined even if I want it to overflow, so we work around it.
#else
    static const SIGNED SIGNED_MIN=(-SIGNED_MIN_RAW); // However if we have unusual bit lengths (3,19, etc) the raw bit representation will be technically correct in the context of that sized integer, but since we have to round to a real integer size to represent the number, the literal interpretation will be wrong. This yields the proper minimum value.
#endif
    static const SIGNED SIGNED_MAX=((~SIGNED_MIN_RAW)&UNSIGNED_MAX);
  };
  template<typename T>
  struct TBitLimit : public ABitLimit<sizeof(T)<<3> {};

  // template inferred version of T_GETBIT and T_GETBITRANGE
  template<class T>
  inline T BSS_FASTCALL GetBitMask(int bit) { return T_GETBIT(T,bit); }
  template<class T>
  inline T BSS_FASTCALL GetBitMask(int low, int high) { return T_GETBITRANGE(T,low,high); }

  // Replaces one character with another in a string
  template<typename T>
  inline T* BSS_FASTCALL strreplace(T* string, const T find, const T replace)
	{
    static_assert(std::is_integral<T>::value,"T must be integral");
		if(!string) return 0;
		unsigned int curpos = (unsigned int)-1; //this will wrap around to 0 when we increment

		while(string[++curpos] != '\0') //replace until the null-terminator
			if(string[curpos] == find)
				string[curpos] = replace;

		return string;
	}

#ifndef BSS_COMPILER_MSC
  template<int SZ>
  BSS_FORCEINLINE char* BSS_FASTCALL strcpyx0(char (&dst)[SZ], const char* src) { return strncpy(dst,src,SZ-1); }
#endif

  // Counts number of occurences of character c in string, up to the null terminator
  template<typename T>
  inline unsigned int BSS_FASTCALL strccount(const T* string, T c)
  {
    static_assert(std::is_integral<T>::value,"T must be integral");
    unsigned int ret=0;
    while(*string) { if(*string==c) ++ret; ++string; }
    return ret;
  }
  
  // Counts number of occurences of character c in string, up to length characters
  template<typename T>
  inline unsigned int BSS_FASTCALL strccount(const T* string, T c, unsigned int length)
  {
    static_assert(std::is_integral<T>::value,"T must be integral");
    unsigned int ret=0;
    for(unsigned int i = 0; (i < length) && ((*string)!=0); ++i)
      if(string[i]==c) ++ret;
    return ret;
  }

  // template swap function, h should be optimized out by the compiler
  template<typename T>
  BSS_FORCEINLINE void BSS_FASTCALL rswap(T& p, T& q)
  {
    T h(std::move(p));
    p=std::move(q);
    q=std::move(h);
  }
  
  // This yields mathematically correct integer division (i/div) towards negative infinity, but only if div is a positive integer. This
  // implementation obviously must have integral types for T and D, but can be explicitely specialized to handle vectors or other structs.
  template<typename T, typename D>
  inline T BSS_FASTCALL intdiv(T i, D div) 
  { 
    static_assert(std::is_integral<T>::value,"T must be integral");
    static_assert(std::is_integral<D>::value,"D must be integral");
    assert(div>0); 
    return (i/div) - ((i<0)&((i%div)!=0)); // If i is negative and has a nonzero remainder, subtract one to correct the truncation.
  }

  // Uses rswap to reverse the order of an array
  template<typename T>
  inline void BSS_FASTCALL bssreverse(T* src, unsigned int length)
  {
    assert(length>0);
    for(unsigned int i = 0,j=(--length); i < j; j=length-(++i)) // Equivelent to: for(unsigned int i = 0,j=length-1; i < j; j=length-1-(++i))
      rswap(src[i],src[j]);
  }
  template<typename T, unsigned int size>
  BSS_FORCEINLINE void BSS_FASTCALL bssreverse(T (&p)[size]) { bssreverse(p,size); }

  /* Trims space from left end of string by returning a different pointer. It is possible to use const char or const wchar_t as a type
     here because the string itself is not modified. */
  template<typename T>
  inline T* BSS_FASTCALL strltrim(T* str)
  {
    static_assert(std::is_integral<T>::value,"T must be integral");
    for(;*str>0 && *str<33;++str);
    return str;
  }
  
  // Trims space from right end of string by inserting a null terminator in the appropriate location
  template<typename T>
  inline T* BSS_FASTCALL strrtrim(T* str)
  {
    static_assert(std::is_integral<T>::value,"T must be integral");
    T* inter=str+strlen(str);

    for(;inter>str && *inter<33;--inter);
    *(++inter)='\0';
    return str;
  }

  // Performs a mathematically correct modulo, unlike the modulo operator, which doesn't actually perform modulo, it performs a remainder operation. THANKS GUYS!
  template<typename T> //T must be integral
  BSS_FORCEINLINE T BSS_FASTCALL bssmod(T x, T m)
  {
		static_assert(std::is_signed<T>::value && std::is_integral<T>::value,"T must be a signed integral type or this function is pointless");
    x%=m;
    return (x+((x<0)*m));
  }

  // Trims space from left and right ends of a string
  template<typename T>
  BSS_FORCEINLINE T* BSS_FASTCALL strtrim(T* str)
  {
    return strrtrim(strltrim(str));
  }

  // This is a bit-shift method of calculating the next number in the fibonacci sequence by approximating the golden ratio with 0.6171875 (1/2 + 1/8 - 1/128)
  template<typename T>
  BSS_FORCEINLINE T BSS_FASTCALL fbnext(T in)
  {
    static_assert(std::is_integral<T>::value,"T must be integral");
    return in + 1 + (in>>1) + (in>>3) - (in>>7);
  }

  // Gets the sign of any number (0 is assumed to be positive)
  template<typename T>
  BSS_FORCEINLINE char BSS_FASTCALL tsign(T n)
  {
    return (n >= 0) - (n < 0);
  }

  // Gets the sign of any number, where a value of 0 returns 0
  template<typename T>
  BSS_FORCEINLINE char BSS_FASTCALL tsignzero(T n)
  {
    return (n > 0) - (n < 0);
  }
  
  // Gets the shortest distance between two angles in radians
  template<typename T>
  BSS_FORCEINLINE T BSS_FASTCALL angledist(T a, T b)
  {
    static_assert(std::is_floating_point<T>::value,"T must be float, double, or long double");
    return ((T)PI) - fabs(fmod(fabs(a - b), ((T)PI_DOUBLE)) - ((T)PI));
  }
  
  // Gets the SIGNED shortest distance between two angles starting with (a - b) in radians
  template<typename T>
  BSS_FORCEINLINE T BSS_FASTCALL angledistsgn(T a, T b)
  {
    static_assert(std::is_floating_point<T>::value,"T must be float, double, or long double");
    return fmod(a - b - ((T)PI), (T)PI_DOUBLE) + ((T)PI);
  }

  /* Directly calls the fistp function for float to int rounding. Because the FPU tends to be a rounding mode that doesn't clamp to 0, this
     will usually round the float to the nearest integer instead of simply chopping off the decimal. This usually takes 6 cycles, compared
     to the 80 or so cycles caused by a normal float to int cast, and works in any precision. */
  template<typename T> //T must be either float or double or you need to stop programming when you're high
  BSS_FORCEINLINE __int32 fFastRound(T f)
  {
    static_assert(std::is_floating_point<T>::value,"T must be float, double, or long double");
#if defined(BSS_CPU_x86) || defined(BSS_CPU_x86_64) || defined(BSS_CPU_IA_64)
#ifdef BSS_COMPILER_GCC
	  __int32 retval;
    __asm__("fld %1\n\t"
            "fistp %0\n\t"
            : "=m" (retval) //outputs
            : "f" (f)); //inputs
	  return retval;
#elif defined(BSS_COMPILER_MSC)
#ifdef BSS_CPU_x86
	  __int32 retval;
	  __asm fld f
	  __asm fistp retval
	  return retval;
#else // defined(BSS_CPU_x86_64) || defined(BSS_CPU_IA_64)
    return _mm_cvt_ss2si(_mm_load_ss(&f)); // HACK: Non-optimal workaround for MSC idiocy
#endif
#else
    return (__int32)f;
#endif
#else
    return (__int32)f;
#endif
  }

  // Returns true if FPU is in single precision mode and false otherwise (false for both double and extended precision)
  BSS_FORCEINLINE bool FPUsingle()
  { 
#if defined(BSS_CPU_x86) || defined(BSS_CPU_x86_64) || defined(BSS_CPU_IA_64)
    unsigned int i;
#ifdef BSS_COMPILER_GCC
    __asm__ __volatile__ ("fnstcw %0" : "=m" (i));
#elif defined(BSS_COMPILER_MSC)
#ifdef BSS_CPU_x86
    __asm fnstcw i;
#else // defined(BSS_CPU_x86_64) || defined(BSS_CPU_IA_64)
    i=_mm_getcsr();
#endif
#endif
    return ((i&(0x0300))==0); //0x0300 is the mask for the precision bits, 0 indicates single precision
#else
    return false;
#endif
  }

  // Extremely fast rounding function that again will usually round to the nearest integer, but only works in double precision mode
  BSS_FORCEINLINE __int32 fFastDoubleRound(double val)
  {
    const double _double2fixmagic = 4503599627370496.0*1.5; //2^52 for 52 bits of mantissa
    assert(!FPUsingle());
	  val		= val + _double2fixmagic;
	  return ((__int32*)&val)[0]; 
  }
  BSS_FORCEINLINE __int32 fFastDoubleRound(float val) { return fFastDoubleRound((double)val); }

  /* Single precision version of the above function. While precision problems are mostly masked in the above function by limiting it to
    __int32, in this function they are far more profound due to there only being 24 bits of mantissa to work with. Use with caution. */
  BSS_FORCEINLINE __int32 fFastSingleRound(double val)
  {
    const double _single2fixmagic = 16777216.0*1.5; //2^24 for 24 bits of mantissa
    assert(FPUsingle());
	  val		= val + _single2fixmagic;
	  return (__int32)(((((unsigned __int64*)&val)[0])&0xFFFFF0000000)>>28);
  }
  BSS_FORCEINLINE __int32 fFastSingleRound(float val) { return fFastSingleRound((double)val); }

	/* This is a super fast floating point comparison function with a significantly higher tolerance and no
		 regard towards the size of the floats. */
	inline bool BSS_FASTCALL fwidecompare(float fleft, float fright)
	{
		__int32 left = *(__int32*)(&fleft); //This maps our float to an int so we can do bitshifting operations on it
		__int32 right = *(__int32*)(&fright); //see above
		unsigned char dif = abs((0x7F800000&left)-(0x7F800000&right))>>23; // This grabs the 8 exponent bits and subtracts them.
		if(dif>1) // An exponent difference of 2 or greater means the numbers are different.
			return false;
		return !dif?((0x007FFF80&left)==(0x007FFF80&right)):!(abs((0x007FFF80&left)-(0x007FFF80&right))-0x007FFF80); //If there is no difference in exponent we tear off the last 7 bits and compare the value, otherwise we tear off the last 7 bits, subtract, and then subtract the highest possible significand to compensate for the extra exponent.
	}

  // Highly optimized traditional tolerance based approach to comparing floating point numbers, found here: http://www.randydillon.org/Papers/2007/everfast.htm
  inline bool BSS_FASTCALL fcompare(float af, float bf, __int32 maxDiff=1)
  { 
    assert(af!=0.0f && bf!=0.0f); // Use fsmall for this
    __int32 ai = *reinterpret_cast<__int32*>(&af);
    __int32 bi = *reinterpret_cast<__int32*>(&bf);
    __int32 test = (-(__int32)(((unsigned __int32)(ai^bi))>>31));
    assert((0 == test) || (0xFFFFFFFF == test));
    __int32 diff = ((ai + test) ^ (test & 0x7fffffff)) - bi;
    __int32 v1 = maxDiff + diff;
    __int32 v2 = maxDiff - diff;
    return (v1|v2) >= 0;
  }
  inline bool BSS_FASTCALL fcompare(double af, double bf, __int64 maxDiff=1)
  { 
    assert(af!=0.0 && bf!=0.0); // Use fsmall for this
    __int64 ai = *reinterpret_cast<__int64*>(&af);
    __int64 bi = *reinterpret_cast<__int64*>(&bf);
    __int64 test = (-(__int64)(((unsigned __int64)(ai^bi))>>63));
    assert((0 == test) || (0xFFFFFFFFFFFFFFFF == test));
    __int64 diff = ((ai + test) ^ (test & 0x7fffffffffffffff)) - bi;
    __int64 v1 = maxDiff + diff;
    __int64 v2 = maxDiff - diff;
    return (v1|v2) >= 0;
  }

  // This determines if a float is sufficiently close to 0
  BSS_FORCEINLINE bool BSS_FASTCALL fsmall(float f, float eps=FLT_EPS)
  {
    unsigned __int32 i=((*((unsigned __int32*)&f))&0x7FFFFFFF); //0x7FFFFFFF strips off the sign bit (which is always the highest bit)
    unsigned __int32 e=((*((unsigned __int32*)&eps)));
    return i<=e; 
  }

  // This determines if a double is sufficiently close to 0
  BSS_FORCEINLINE bool BSS_FASTCALL fsmall(double f, double eps=DBL_EPS)
  {
    unsigned __int64 i=((*((unsigned __int64*)&f))&0x7FFFFFFFFFFFFFFF); //0x7FFFFFFFFFFFFFFF strips off the sign bit (which is always the highest bit)
    unsigned __int64 e=((*((unsigned __int64*)&eps)));
    return i<=e; 
  }

  // This is a super fast length approximation for 2D coordinates; See http://www.azillionmonkeys.com/qed/sqroot.html for details (Algorithm by Paul Hsieh)
  inline float BSS_FASTCALL flength(float x, float y)
  {
    x = fabs(x);
    y = fabs(y);
    //(1 + 1/(4-2*sqrt(2)))/2 = 0.92677669529663688
    float hold=0.7071067811865475f*(x+y), mval=(x > y)?x:y;
    return 0.92677669529663688f * ((hold > mval)?hold:mval);
  }
  
  // The classic fast square root approximation, which is often mistakenly attributed to John Carmack. The algorithm is in fact over 15 years old and no one knows where it came from.
  static float BSS_FASTCALL fFastSqrt(float number)
  {
    const float f = 1.5F;
    __int32 i;
    float x, y;

    x = number * 0.5F;
    y  = number;
    i  = * ( __int32 * ) &y;
    i  = 0x5f3759df - ( i >> 1 );
    y  = * ( float * ) &i;
    y  = y * ( f - ( x * y * y ) );
    y  = y * ( f - ( x * y * y ) ); //extra iteration for added accuracy
    return number * y;
  }

  // Adaptation of the class fast square root approximation for double precision, based on http://www.azillionmonkeys.com/qed/sqroot.html
  static double BSS_FASTCALL dFastSqrt(double number)
  {
    const double f = 1.5;
    unsigned __int32* i;
    double x, y;

    x = number*0.5;
	  y = number;
    i = ((unsigned __int32 *)&y) + 1;
	  *i = (0xbfcdd90a - *i)>>1; // estimate of 1/sqrt(number)

    y = y * ( f - ( x * y * y ) );
    y = y * ( f - ( x * y * y ) );
    y = y * ( f - ( x * y * y ) ); //Newton raphson converges quadratically, so one additional iteration doubles the precision
    //y = y * ( f - ( x * y * y ) ); 
    return number * y;
  }

  // bit-twiddling based method of calculating an integral square root from Wilco Dijkstra - http://www.finesse.demon.co.uk/steven/sqrt.html
  template<typename T, unsigned int bits> // WARNING: bits should be HALF the actual number of bits in (T)!
  static T BSS_FASTCALL IntFastSqrt(T n)
  {
    static_assert(std::is_integral<T>::value,"T must be integral");
    T root = 0, t;

    for(unsigned int i = bits; i>0;)
    {
      --i;
      t = ((root + (((T)1) << (i))) << (i)); 
      if (n >= t)   
      {   n -= t;   
          root |= 2 << (i); 
      }
    }
    return root >> 1;
  }
  template<typename T>
  BSS_FORCEINLINE T BSS_FASTCALL IntFastSqrt(T n)
  {
    return IntFastSqrt<T,sizeof(T)<<2>(n); //done to ensure loop gets unwound (the bit conversion here is <<2 because the function wants HALF the bits in T, so <<3 >>1 -> <<2)
  }
  
  template<typename T> //assumes integer type if not one of the floating point types
  BSS_FORCEINLINE T BSS_FASTCALL FastSqrt(T n) { return IntFastSqrt(n); } //Picks correct method for calculating any square root quickly
  template<> BSS_FORCEINLINE float BSS_FASTCALL FastSqrt(float n) { return fFastSqrt(n); }
  template<> BSS_FORCEINLINE double BSS_FASTCALL FastSqrt(double n) { return dFastSqrt(n); }
  template<> BSS_FORCEINLINE long double BSS_FASTCALL FastSqrt(long double n) { return dFastSqrt((double)n); }

  // Distance calculation (squared)
  template<typename T>
  BSS_FORCEINLINE T BSS_FASTCALL distsqr(T X, T Y, T x, T y)
  {
    T tx=X-x,ty=Y-y; return (tx*tx)+(ty*ty); //It doesn't matter if you use temporary values for floats, but it does if you use ints (for unknown reasons)
  }

  // Distance calculation
  template<typename T>
  BSS_FORCEINLINE T BSS_FASTCALL dist(T X, T Y, T x, T y)
  {
    return FastSqrt<T>(distsqr<T>(X,Y,x,y));
  }

  // Average aggregation without requiring a total variable that can overflow. Nextnum should be the current avg count incremented by 1.
  template<typename T, typename ST_> // T must be float or double, ST_ must be integral
  BSS_FORCEINLINE T BSS_FASTCALL bssavg(T curavg, T nvalue, ST_ nextnum)
  {
    static_assert(std::is_integral<ST_>::value,"ST_ must be integral");
    static_assert(std::is_floating_point<T>::value,"T must be float, double, or long double");
    return (curavg*(((T)(nextnum-1))/((T)nextnum))) + nvalue/((T)nextnum);
  }

  // Searches an arbitrary series of bytes for another arbitrary series of bytes
  static const void* bytesearch(const void* search, size_t length, const void* find, size_t flength)
  {
    if(!search || !length || !find || !flength || length < flength) return 0;

    unsigned char* s=(unsigned char*)search;
    length-=flength;
    for(size_t i = 0; i <= length; ++i) // i <= length works because of the above length-=flength
    {
      search=s+i;
      if(!memcmp(search,find,flength))
        return search;
    }
    return 0;
  }

  BSS_FORCEINLINE void* bytesearch(void* search, size_t length, void* find, size_t flength)
  {
    return const_cast<void*>(bytesearch((const void*)search,length,(const void*)find,flength));
  }

  // Counts the number of bits in v (up to 128-bit types) using the parallel method detailed here: http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
  template<typename T>
  inline unsigned char BSS_FASTCALL bitcount(T v)
  {
    static_assert(std::is_integral<T>::value,"T must be integral");
    v = v - ((v >> 1) & (T)~(T)0/3);                           // temp
    v = (v & (T)~(T)0/15*3) + ((v >> 2) & (T)~(T)0/15*3);      // temp
    v = (v + (v >> 4)) & (T)~(T)0/255*15;                      // temp
    return (unsigned char)((T)(v * ((T)~(T)0/255)) >> (sizeof(T) - 1) * (sizeof(char)<<3));
  }

  //Unlike FastSqrt, these are useless unless you are on a CPU without SSE instructions, or have a terrible std implementation.
  //// Fast sin function with 0.078% error when extra precision is left in. See http://www.devmaster.net/forums/showthread.php?t=5784
  //inline float BSS_FASTCALL FastSin(float x)
  //{
  //  x-= (int)(x*(1/(float)PI_DOUBLE))*(float)PI_DOUBLE;
  //  const float B = 4/(float)PI;
  //  const float C = -4/(float)PI_DOUBLE;

  //  float y = B * x + C * x * fabs(x);

  //  //const float Q = 0.775f;
  //  const float P = 0.225f;

  //  return (P * (y * fabs(y) - y) + y);   // Q * y + P * y * fabs(y) (extra precision)
  //}

  //inline float BSS_FASTCALL FastCos(float x)
  //{
  //  return FastSin(x+(float)PI_HALF);
  //}

  // Round a number up to the next power of 2 (32 -> 32, 33 -> 64, etc.)
  inline unsigned __int64 BSS_FASTCALL nextpow2(unsigned __int64 v)
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
  inline unsigned int BSS_FASTCALL nextpow2(unsigned int v)
  {
	  v -= 1;
	  v |= (v >> 1);
	  v |= (v >> 2);
	  v |= (v >> 4);
	  v |= (v >> 8);
	  v |= (v >> 16);
	
	  return v + 1;
  }
  inline unsigned short BSS_FASTCALL nextpow2(unsigned short v)
  {
	  v -= 1;
	  v |= (v >> 1);
	  v |= (v >> 2);
	  v |= (v >> 4);
	  v |= (v >> 8);
	
	  return v + 1;
  }
  inline unsigned char BSS_FASTCALL nextpow2(unsigned char v)
  {
	  v -= 1;
	  v |= (v >> 1);
	  v |= (v >> 2);
	  v |= (v >> 4);
	
	  return v + 1;
  }

  // Bit-twiddling hack for base 2 log by Sean Eron Anderson
  inline unsigned int BSS_FASTCALL log2(unsigned char v)
  {
    const unsigned int b[] = {0x2, 0xC, 0xF0};
    const unsigned int S[] = {1, 2, 4};

    register unsigned int r = 0; // result of log2(v) will go here
    if (v & b[2]) { v >>= S[2]; r |= S[2]; } 
    if (v & b[1]) { v >>= S[1]; r |= S[1]; } 
    if (v & b[0]) { v >>= S[0]; r |= S[0]; } 

    return r;
  }
  inline unsigned int BSS_FASTCALL log2(unsigned short v)
  {
    const unsigned int b[] = {0x2, 0xC, 0xF0, 0xFF00};
    const unsigned int S[] = {1, 2, 4, 8};

    register unsigned int r = 0; // result of log2(v) will go here
    if (v & b[3]) { v >>= S[3]; r |= S[3]; } 
    if (v & b[2]) { v >>= S[2]; r |= S[2]; } 
    if (v & b[1]) { v >>= S[1]; r |= S[1]; } 
    if (v & b[0]) { v >>= S[0]; r |= S[0]; } 

    return r;
  }

  inline unsigned int BSS_FASTCALL log2(unsigned int v)
  {
    const unsigned int b[] = {0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000};
    const unsigned int S[] = {1, 2, 4, 8, 16};

    register unsigned int r = 0; // result of log2(v) will go here
    if (v & b[4]) { v >>= S[4]; r |= S[4]; } 
    if (v & b[3]) { v >>= S[3]; r |= S[3]; } 
    if (v & b[2]) { v >>= S[2]; r |= S[2]; } 
    if (v & b[1]) { v >>= S[1]; r |= S[1]; } 
    if (v & b[0]) { v >>= S[0]; r |= S[0]; } 

    return r;
  }
  inline unsigned int BSS_FASTCALL log2(unsigned __int64 v)
  {
    const unsigned __int64 b[] = {0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000, 0xFFFFFFFF00000000 };
    const unsigned int S[] = {1, 2, 4, 8, 16, 32};

    register unsigned int r = 0; // result of log2(v) will go here
    if (v & b[5]) { v >>= S[5]; r |= S[5]; } 
    if (v & b[4]) { v >>= S[4]; r |= S[4]; } 
    if (v & b[3]) { v >>= S[3]; r |= S[3]; } 
    if (v & b[2]) { v >>= S[2]; r |= S[2]; } 
    if (v & b[1]) { v >>= S[1]; r |= S[1]; } 
    if (v & b[0]) { v >>= S[0]; r |= S[0]; } 

    return r;
  }
  inline unsigned int BSS_FASTCALL log2_p2(unsigned int v) //Works only if v is a power of 2
  {
    assert(v && !(v & (v - 1))); //debug version checks to ensure its a power of two
    const unsigned int b[] = {0xAAAAAAAA, 0xCCCCCCCC, 0xF0F0F0F0, 0xFF00FF00, 0xFFFF0000};
    register unsigned int r = (v & b[0]) != 0;
    r |= ((v & b[4]) != 0) << 4;
    r |= ((v & b[3]) != 0) << 3;
    r |= ((v & b[2]) != 0) << 2;
    r |= ((v & b[1]) != 0) << 1;

    return r;
  }

  // Basic lerp function with no bounds checking
  template<class T>
  BSS_FORCEINLINE T BSS_FASTCALL lerp(T a, T b, double amt)
  {
	  return a+((T)((b-a)*amt));
  }
  
  //unique_ptr deleter class that forces the deletion to occur in this DLL
  template<class _Ty>
	struct bssdll_delete
	{	// default deleter for unique_ptr
	  typedef bssdll_delete<_Ty> _Myt;

	  void operator()(_Ty *_Ptr) const {
		  if (0 < sizeof (_Ty))	{ // won't compile for incomplete type
        _Ptr->~_Ty(); // call destructor because delete won't
			  bssdll_delete_delfunc(_Ptr);
		  }
    }
	};
  
  template<class _Ty>
	struct bssdll_delete<_Ty[]>
	{	// default deleter for unique_ptr to array of unknown size
	  typedef bssdll_delete<_Ty> _Myt;

	  void operator()(_Ty *_Ptr) const {
		  if (0 < sizeof (_Ty))	{ // won't compile for incomplete type
        _Ptr->~_Ty(); // call destructor because delete won't
			  bssdll_delete_delfuncarray(_Ptr);
      }
		}
	};

  BSS_COMPILER_DLLEXPORT extern void bssdll_delete_delfunc(void* p);
  BSS_COMPILER_DLLEXPORT extern void bssdll_delete_delfuncarray(void* p);
} 

#endif

  // Simple 128bit integer for x86 instruction set. Replaced with __int128 if possible
//#ifdef BSS32BIT
//  struct int128
//  {
//    inline int128& BSS_FASTCALL operator+=(const int128& right) { _add(right.ints[0],right.ints[1],right.ints[2],right.ints[3]); return *this; }
//
//    __int32 ints[4];
//
//  private:
//    inline void BSS_FASTCALL _add(__int32 a,__int32 b,__int32 c,__int32 d) //Done because I suck with assembly :D
//    {
//      __asm {
//#ifdef BSS_NO_FASTCALL //if we are using fastcall we don't need these instructions
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
//    inline uint128& BSS_FASTCALL operator+=(const uint128& right) { _add(right.ints[0],right.ints[1],right.ints[2],right.ints[3]); return *this; }
//
//    unsigned __int32 ints[4];
//
//  private:
//    inline void BSS_FASTCALL _add(unsigned __int32 a,unsigned __int32 b,unsigned __int32 c,unsigned __int32 d) //Done because I suck with assembly :D
//    {
//      __asm {
//#ifdef BSS_NO_FASTCALL //if we are using fastcall we don't need these instructions
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
