// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/buntils.h"
#include <sstream>
#include <algorithm>
#include <format>

using namespace bun;

static_assert(std::is_same<make_integral<char>::type, char>::value, "type not preserved");
static_assert(std::is_same<make_integral<void*>::type, void*>::value, "type not preserved");
static_assert(std::is_trivially_copyable_v<std::byte>, "wat");

template<uint8_t B, int64_t SMIN, int64_t SMAX, uint64_t UMIN, uint64_t UMAX, typename T>
inline void TEST_BitLimit()
{
  static_assert(std::is_same<T, typename BitLimit<B>::SIGNED>::value, "BitLimit failure" TXT(B));
  static_assert(std::is_same<typename std::make_unsigned<T>::type, typename BitLimit<B>::UNSIGNED>::value, "BitLimit failure" TXT(B));
#ifndef BUN_COMPILER_CLANG // Clang refuses to recognize this as constant
  static_assert(BitLimit<B>::SIGNED_MIN == SMIN, "BitLimit failure" TXT(B));
#endif
  static_assert(BitLimit<B>::SIGNED_MAX == SMAX, "BitLimit failure" TXT(B));
  static_assert(BitLimit<B>::UNSIGNED_MIN == UMIN, "BitLimit failure" TXT(B));
  static_assert(BitLimit<B>::UNSIGNED_MAX == UMAX, "BitLimit failure" TXT(B));
}

template<class T>
T naivebitcount(T v)
{
  T c;
  for(c = 0; v; v >>= 1)
    c += (v & 1);
  return c;
}

template<class T>
void testbitcount(TESTDEF::RETPAIR& __testret)
{ //Use fibonacci numbers to test this
  for(T i = 0; i < (((T)1) << (sizeof(T) << 2)); i = fbnext(i))
  {
    TEST(naivebitcount<T>(i) == BitCount<T>(i));
  }
}

TESTDEF::RETPAIR test_buntils()
{
  BEGINTEST;
  TESTNOERROR(SetWorkDirToCur());
  TEST(bun_FileSize(GetProgramPath()) != 0);
  //TEST(bun_FileSize(StrW(fbuf))!=0);
  TESTNOERROR(GetTimeZoneMinutes());

  static_assert(std::is_same<BitLimit<sizeof(uint8_t) << 3>::SIGNED, char>::value, "Test Failure Line #" TXT(__LINE__));
  static_assert(std::is_same<BitLimit<sizeof(uint16_t) << 3>::SIGNED, short>::value, "Test Failure Line #" TXT(__LINE__));
  static_assert(std::is_same<BitLimit<sizeof(int) << 3>::SIGNED, int>::value, "Test Failure Line #" TXT(__LINE__));
  static_assert(std::is_same<BitLimit<sizeof(char) << 3>::UNSIGNED, uint8_t>::value, "Test Failure Line #" TXT(__LINE__));
  static_assert(std::is_same<BitLimit<sizeof(short) << 3>::UNSIGNED, uint16_t>::value, "Test Failure Line #" TXT(__LINE__));
  static_assert(std::is_same<BitLimit<sizeof(uint32_t) << 3>::UNSIGNED, uint32_t>::value, "Test Failure Line #" TXT(__LINE__));
  static_assert(std::is_same<BitLimit<sizeof(double) << 3>::SIGNED, int64_t>::value, "Test Failure Line #" TXT(__LINE__));
  static_assert(std::is_same<BitLimit<sizeof(float) << 3>::SIGNED, int>::value, "Test Failure Line #" TXT(__LINE__));
  static_assert(std::is_same<BitLimit<sizeof(double) << 3>::UNSIGNED, uint64_t>::value, "Test Failure Line #" TXT(__LINE__));
  static_assert(std::is_same<BitLimit<sizeof(float) << 3>::UNSIGNED, uint32_t>::value, "Test Failure Line #" TXT(__LINE__));
  //static_assert(std::is_same<BitLimit<sizeof(long double)<<3>::SIGNED, int64_t>::value, "Test Failure Line #" TXT(__LINE__)); // long double is not a well defined type
  //static_assert(std::is_same<BitLimit<sizeof(long double)<<3>::UNSIGNED, uint64_t>::value, "Test Failure Line #" TXT(__LINE__));
  static_assert(T_CHARGETMSB(0) == 0, "Test Failure Line #" TXT(__LINE__));
  static_assert(T_CHARGETMSB(1) == 1, "Test Failure Line #" TXT(__LINE__));
  static_assert(T_CHARGETMSB(2) == 2, "Test Failure Line #" TXT(__LINE__));
  static_assert(T_CHARGETMSB(3) == 2, "Test Failure Line #" TXT(__LINE__));
  static_assert(T_CHARGETMSB(4) == 4, "Test Failure Line #" TXT(__LINE__));
  static_assert(T_CHARGETMSB(7) == 4, "Test Failure Line #" TXT(__LINE__));
  static_assert(T_CHARGETMSB(8) == 8, "Test Failure Line #" TXT(__LINE__));
  static_assert(T_CHARGETMSB(20) == 16, "Test Failure Line #" TXT(__LINE__));
  static_assert(T_CHARGETMSB(84) == 64, "Test Failure Line #" TXT(__LINE__));
  static_assert(T_CHARGETMSB(189) == 128, "Test Failure Line #" TXT(__LINE__));
  static_assert(T_CHARGETMSB(255) == 128, "Test Failure Line #" TXT(__LINE__));

  //Note that these tests CAN fail if trying to compile to an unsupported or buggy platform that doesn't have two's complement.
  TEST(TBitLimit<long long>::SIGNED_MIN == std::numeric_limits<long long>::min());
  TEST(TBitLimit<long>::SIGNED_MIN == std::numeric_limits<long>::min());
  TEST(TBitLimit<int>::SIGNED_MIN == std::numeric_limits<int>::min());
  TEST(TBitLimit<short>::SIGNED_MIN == std::numeric_limits<short>::min());
  TEST(TBitLimit<char>::SIGNED_MIN == std::numeric_limits<char>::min());
  TEST(TBitLimit<long long>::SIGNED_MAX == std::numeric_limits<long long>::max());
  TEST(TBitLimit<long>::SIGNED_MAX == std::numeric_limits<long>::max());
  TEST(TBitLimit<int>::SIGNED_MAX == std::numeric_limits<int>::max());
  TEST(TBitLimit<short>::SIGNED_MAX == std::numeric_limits<short>::max());
  TEST(TBitLimit<char>::SIGNED_MAX == std::numeric_limits<char>::max());
  TEST(TBitLimit<long long>::UNSIGNED_MAX == std::numeric_limits<unsigned long long>::max());
  TEST(TBitLimit<long>::UNSIGNED_MAX == std::numeric_limits<unsigned long>::max());
  TEST(TBitLimit<int>::UNSIGNED_MAX == std::numeric_limits<uint32_t>::max());
  TEST(TBitLimit<short>::UNSIGNED_MAX == std::numeric_limits<uint16_t>::max());
  TEST(TBitLimit<char>::UNSIGNED_MAX == std::numeric_limits<uint8_t>::max());
  TEST(TBitLimit<long long>::UNSIGNED_MIN == std::numeric_limits<unsigned long long>::min());
  TEST(TBitLimit<long>::UNSIGNED_MIN == std::numeric_limits<unsigned long>::min());
  TEST(TBitLimit<int>::UNSIGNED_MIN == std::numeric_limits<uint32_t>::min());
  TEST(TBitLimit<short>::UNSIGNED_MIN == std::numeric_limits<uint16_t>::min());
  TEST(TBitLimit<char>::UNSIGNED_MIN == std::numeric_limits<uint8_t>::min());

  // These tests assume twos complement. This is ok because the previous tests would have caught errors relating to that anyway.
  TEST_BitLimit<1, -1, 0, 0, 1, char>();
  TEST_BitLimit<2, -2, 1, 0, 3, char>();
  TEST_BitLimit<7, -64, 63, 0, 127, char>();
  TEST_BitLimit<sizeof(char) << 3, CHAR_MIN, CHAR_MAX, 0, UCHAR_MAX, char>();
  TEST_BitLimit<9, -256, 255, 0, 511, short>();
  TEST_BitLimit<15, -16384, 16383, 0, 32767, short>();
  TEST_BitLimit<sizeof(short) << 3, SHRT_MIN, SHRT_MAX, 0, USHRT_MAX, short>();
  TEST_BitLimit<17, -65536, 65535, 0, 131071, int>();
  TEST_BitLimit<63, -4611686018427387904LL, 4611686018427387903LL, 0, 9223372036854775807ULL, int64_t>();
  TEST_BitLimit<64, std::numeric_limits<int64_t>::min(), 9223372036854775807LL, 0, 18446744073709551615ULL, int64_t>();
  // For reference, the above strange bit values are used in fixed-point arithmetic found in bun_fixedpt.h

  TEST(GetBitMask<uint8_t>(4) == 0x10); // 0001 0000
  TEST(GetBitMask<uint8_t>(2, 4) == 0x1C); // 0001 1100
  TEST(GetBitMask<uint8_t>(-2, 2) == 0xC7); // 1100 0111
  TEST(GetBitMask<uint8_t>(-2, -2) == 0x40); // 0100 0000
  TEST(GetBitMask<uint8_t>(0, 0) == 0x01); // 0000 0001
  TEST(GetBitMask<uint8_t>(0, 5) == 0x3F); // 0011 1111
  TEST(GetBitMask<uint8_t>(0, 7) == 0xFF); // 1111 1111
  TEST(GetBitMask<uint8_t>(-7, 0) == 0xFF); // 1111 1111
  TEST(GetBitMask<uint8_t>(-5, 0) == 0xF9); // 1111 1001
  TEST(GetBitMask<uint8_t>(-5, -1) == 0xF8); // 1111 1000
  TEST(GetBitMask<uint8_t>(-6, -3) == 0x3C); // 0011 1100
  TEST(GetBitMask<uint32_t>(0, 0) == 0x00000001);
  TEST(GetBitMask<uint32_t>(0, 16) == 0x0001FFFF);
  TEST(GetBitMask<uint32_t>(12, 30) == 0x7FFFF000);
  TEST(GetBitMask<uint32_t>(-10, 0) == 0xFFC00001);
  TEST(GetBitMask<uint32_t>(-30, 0) == 0xFFFFFFFD);
  TEST(GetBitMask<uint32_t>(-12, -1) == 0xFFF00000);
  TEST(GetBitMask<uint32_t>(-15, -12) == 0x001e0000);
  for(int i = 0; i < 8; ++i)
    TEST(GetBitMask<uint8_t>(i) == (1 << i));
  for(int i = 0; i < 32; ++i)
    TEST(GetBitMask<uint32_t>(i) == (1 << i));
  for(int i = 0; i < 64; ++i)
    TEST(GetBitMask<unsigned long long>(i) == (((uint64_t)1) << i));

  int bits = 9;
  bits = bun_SetBit(bits, 4, true);
  TEST(bits == 13);
  bits = bun_SetBit(bits, 8, false);
  TEST(bits == 5);

  std::string cpan(PANGRAM);

  strreplace(cpan.data(), 'm', '?');
  TEST(!strchr(cpan.c_str(), 'm') && strchr(cpan.c_str(), '?') != 0);
  std::basic_string<bun_char, std::char_traits<bun_char>, std::allocator<bun_char>> pan;
  for(size_t i = 0; i < NPANGRAMS; ++i)
  {
    pan = PANGRAMS[i];
    bun_char f = pan[((i + 7) << 3) % pan.length()];
    bun_char r = pan[((((i * 13) >> 3) + 13) << 3) % pan.length()];
    if(f == r) r = pan[pan.length() - 1];
    strreplace<bun_char>(pan.data(), f, r);
#ifdef BUN_PLATFORM_WIN32
    TEST(!wcschr(pan.c_str(), f) && wcschr(pan.c_str(), r) != 0);
#else
    TEST(!strchr(pan.c_str(), f) && strchr(pan.c_str(), r) != 0);
#endif
  }
  TEST(strccount<char>("10010010101110001", '1') == 8);
  TEST(strccount<char>("0100100101011100010", '1') == 8);
  // Linux really hates wchar_t, so getting it to assign the right character is exceedingly difficult. We manually assign 1585 instead.
  TEST(strccount<wchar_t>(TESTUNICODESTR, (wchar_t)1585) == 5);

  {
    int ia = 0;
    int ib = 1;
    std::pair<int, int> sa(1, 2);
    std::pair<int, int> sb(2, 1);
    std::unique_ptr<int[]> ua(new int[2]);
    std::unique_ptr<int[]> ub((int*)0);
    std::string ta("first");
    std::string tb("second");
    std::swap(ia, ib);
    TEST(ia == 1);
    TEST(ib == 0);
    std::swap(sa, sb);
    TEST((sa == std::pair<int, int>(2, 1)));
    TEST((sb == std::pair<int, int>(1, 2)));
    std::swap(ua, ub);
    TEST(ua.get() == 0);
    TEST(ub.get() != 0);
    std::swap(ta, tb);
    TEST(ta == "second");
    TEST(tb == "first");
  }

  TEST((IntDiv<int, int>(10, 5) == 2));
  TEST((IntDiv<int, int>(9, 5) == 1));
  TEST((IntDiv<int, int>(5, 5) == 1));
  TEST((IntDiv<int, int>(4, 5) == 0));
  TEST((IntDiv<int, int>(0, 5) == 0));
  TEST((IntDiv<int, int>(-1, 5) == -1));
  TEST((IntDiv<int, int>(-5, 5) == -1));
  TEST((IntDiv<int, int>(-6, 5) == -2));
  TEST((IntDiv<int, int>(-10, 5) == -2));
  TEST((IntDiv<int, int>(-11, 5) == -3));

  TEST((bun_Mod(-1, 7) == 6));
  TEST((bun_Mod(-90, 7) == 1));
  TEST((bun_Mod(6, 7) == 6));
  TEST((bun_Mod(7, 7) == 0));
  TEST((bun_Mod(0, 7) == 0));
  TEST((bun_Mod(1, 7) == 1));
  TEST((bun_Mod(8, 7) == 1));
  TEST((bun_Mod(71, 7) == 1));
  TEST((bun_Mod(1, 1) == 0));
  TEST((bun_Mod(-1, 2) == 1));

  TEST(fCompare(bun_FMod(-1.0f, PI_DOUBLEf), 5.2831853f, 10));
  TEST(fCompare(bun_FMod(-4.71f, PI_DOUBLEf), 1.57319f, 100));
  TEST(fCompare(bun_FMod(1.0f, PI_DOUBLEf), 1.0f));
  TEST((bun_FMod(0.0f, PI_DOUBLEf)) == 0.0f);
  TEST(fCompare(bun_FMod(12.0f, PI_DOUBLEf), 5.716814f, 10));
  TEST(fCompare(bun_FMod(90.0f, PI_DOUBLEf), 2.0354057f, 100));
  TEST(fCompare(bun_FMod(-90.0f, PI_DOUBLEf), 4.2477796f, 10));

  {
    int r[] = { -1,0,2,3,4,5,6 };
    int rr[] = { 6,5,4,3,2,0,-1 };
    std::reverse(std::begin(r), std::end(r));
    TESTARRAY(r, return (r[0] == rr[0]);)
  }

  const char* LTRIM = "    trim ";
  TEST(!strcmp(strltrim(LTRIM), "trim "));
  char RTRIM[] = { ' ','t','r','i','m',' ',' ',0 }; // :|
  TEST(!strcmp(strrtrim(RTRIM), " trim"));
  RTRIM[5] = ' ';
  TEST(!strcmp(strtrim(RTRIM), "trim"));

  uint32_t nsrc[] = { 0,1,2,3,4,5,10,13,21,2873,3829847,2654435766 };
  uint32_t num[] = { 1,2,4,5,7,8,17,21,34,4647,6193581,4292720341 };
  std::transform(std::begin(nsrc), std::end(nsrc), nsrc, &fbnext<uint32_t>);
  TESTARRAY(nsrc, return nsrc[i] == num[i];)

    int value = 8;
  int exact = value;
  int exactbefore = value;

  while(value < 100000)
  {
    exact += exactbefore;
    exactbefore = exact - exactbefore;
    value = fbnext(value);
  }

  TEST(tsign((int64_t)0) == 1)
    TEST(tsign(-28738597) == -1)
    TEST(tsign(INT_MIN) == -1)
    TEST(tsign(INT_MAX) == 1)
    TEST(tsignzero(2.8) == 1)
    TEST(tsignzero(-2.8) == -1)
    TEST(tsignzero(23897523987453.8f) == 1)
    TEST(tsignzero((int64_t)0) == 0)
    TEST(tsignzero(0.0) == 0)
    TEST(tsignzero(0.0f) == 0)
    TEST(tsignzero(-28738597) == -1)
    TEST(tsignzero(INT_MIN) == -1)
    TEST(tsignzero(INT_MAX) == 1)

    TEST(fCompare(AngleDist(PI, PI_HALF), PI_HALF))
    TEST(fSmall(AngleDist(PI, PI + PI_DOUBLE)))
    TEST(fCompare(AngleDist(PI_DOUBLE + PI, PI + PI_HALF*7.0), PI_HALF))
    TEST(fCompare(AngleDist(PI + PI_HALF*7.0, PI_DOUBLE + PI), PI_HALF))
    TEST(fCompare(AngleDist(PIf, PI_HALFf), PI_HALFf))
    TEST(fSmall(AngleDist(PIf, PIf + PI_DOUBLEf), FLT_EPS * 4))
    TEST(fCompare(AngleDist(PI_DOUBLEf + PIf, PIf + PI_HALFf*7.0f), PI_HALFf, 9))
    TEST(fCompare(AngleDist(PIf + PI_HALFf*7.0f, PI_DOUBLEf + PIf), PI_HALFf, 9))
    TEST(fCompare(AngleDist(PIf + PI_HALFf*7.0f, PI_DOUBLEf + PIf), PI_HALFf, 9))
    TEST(fCompare(AngleDist(1.28f, -4.71f), 0.2931f, 10000))
    TEST(fCompare(AngleDist(1.28f, -4.71f + PI_DOUBLEf), 0.2931f, 10000))

    TEST(fCompare(AngleDistSigned(PI, PI_HALF), -PI_HALF))
    TEST(fSmall(AngleDistSigned(PI, PI + PI_DOUBLE)))
    TEST(fCompare(AngleDistSigned(PI_DOUBLE + PI, PI + PI_HALF*7.0), -PI_HALF))
    TEST(fCompare(AngleDistSigned(PI_HALF, PI), PI_HALF))
    TEST(fCompare(AngleDistSigned(PIf, PI_HALFf), -PI_HALFf))
    TEST(fSmall(AngleDistSigned(PIf, PIf + PI_DOUBLEf), -FLT_EPS * 4))
    TEST(fCompare(AngleDistSigned(PI_DOUBLEf + PIf, PIf + PI_HALFf*7.0f), -PI_HALFf, 9))
    TEST(fCompare(AngleDistSigned(PI_HALFf, PIf), PI_HALFf))
    TEST(fCompare(AngleDistSigned(1.28f, -4.71f), 0.2931f, 10000))
    TEST(fCompare(AngleDistSigned(1.28f, -4.71f + PI_DOUBLEf), 0.2931f, 10000))

    const float flt = FLT_EPSILON;
  int32_t fi = *(int32_t*)(&flt);
  TEST(fSmall(*(float*)(&(--fi))))
    TEST(fSmall(*(float*)(&(++fi))))
    TEST(!fSmall(*(float*)(&(++fi))))
    const double dbl = DBL_EPSILON;
  int64_t di = *(int64_t*)(&dbl);
  TEST(fSmall(*(double*)(&(--di))))
    TEST(fSmall(*(double*)(&(++di))))
    TEST(!fSmall(*(double*)(&(++di))))

    TEST(fCompare(1.0f, 1.0f))
    TEST(fCompare(1.0f, 1.0f + FLT_EPSILON))
    TEST(fCompare(10.0f, 10.0f + FLT_EPSILON * 10))
    TEST(fCompare(10.0f, 10.0f))
    TEST(!fCompare(0.1f, 0.1f + FLT_EPSILON*0.1f))
    TEST(!fCompare(0.1f, FLT_EPSILON))
    TEST(fCompare(1.0, 1.0))
    TEST(fCompare(1.0, 1.0 + DBL_EPSILON))
    TEST(fCompare(10.0, 10.0 + DBL_EPSILON * 10))
    TEST(fCompare(10.0, 10.0))
    TEST(!fCompare(0.1, 0.1 + DBL_EPSILON*0.1))
    TEST(!fCompare(0.1, DBL_EPSILON))

    TEST(fsign(0.0f) == 1.0f);
  TEST(fsign(1.0f) == 1.0f);
  TEST(fsign(5.0f) == 1.0f);
  TEST(fsign(987421594.2387f) == 1.0f);
  TEST(fsign(0.0001) == 1.0f);
  TEST(fsign(-1.0f) == -1.0f);
  TEST(fsign(-5.0f) == -1.0f);
  TEST(fsign(-4857928.2738f) == -1.0f);
  TEST(fsign(-0.0001f) == -1.0f);
  TEST(fsign(0.0) == 1.0);
  TEST(fsign(1.0) == 1.0);
  TEST(fsign(5.0) == 1.0);
  TEST(fsign(987421594.2387) == 1.0);
  TEST(fsign(0.0001) == 1.0);
  TEST(fsign(-1.0) == -1.0);
  TEST(fsign(-5.0) == -1.0);
  TEST(fsign(-4857928.2738) == -1.0);
  TEST(fsign(-0.0001) == -1.0);

  // This tests our average aggregation formula, which lets you average extremely large numbers while maintaining a fair amount of precision.
  uint64_t total = 0;
  uint32_t nc;
  double avg = 0;
  double diff = 0.0;
  for(nc = 1; nc < 10000; ++nc)
  {
    total += nc*nc;
    avg = bun_Avg<double>(avg, (double)(nc*nc), nc);
    diff = bun_max(diff, fabs((total / (double)nc) - avg));
  }
  TEST(diff < FLT_EPSILON * 2);

  // FastSqrt testing ground
  //
  //float a=2;
  //float b;
  //double sqrt_avg=0;
  //float NUMBERS[100000];
  //for(size_t i = 0; i < 100000; ++i)
  //  NUMBERS[i]=bun_RandReal(2,4);

  //uint64_t p=HighPrecisionTimer::OpenProfiler();
  //CPU_Barrier();
  //for(uint32_t j = 0; j < 10; ++j)
  //{
  //for(size_t i = 0; i < 100000; ++i)
  //{
  //  a=NUMBERS[i];
  //  b=std::sqrtf(a);
  //}
  //for(size_t i = 0; i < 100000; ++i)
  //{
  //  a=NUMBERS[i];
  //  b=FastSqrtsse(a);
  //}
  //for(size_t i = 0; i < 100000; ++i)
  //{
  //  a=NUMBERS[i];
  //  b=FastSqrt(a);
  //}
  //}
  //CPU_Barrier();
  //sqrt_avg=HighPrecisionTimer::CloseProfiler(p);
  //
  //TEST(b==a); //keep things from optimizing out
  //cout << sqrt_avg << std::endl;
  //CPU_Barrier();
  double ddbl = fabs(FastSqrt(2.0) - sqrt(2.0));
#ifdef BUN_COMPILER_GCC
  TEST(fabs(FastSqrt(2.0f) - sqrt(2.0f)) <= FLT_EPSILON * 3);
#else
  TEST(fabs(FastSqrt(2.0f) - sqrt(2.0f)) <= FLT_EPSILON * 2);
#endif
  TEST(fabs(FastSqrt(2.0) - sqrt(2.0)) <= (DBL_EPSILON * 100)); // Take note of the 100 epsilon error here on the fastsqrt for doubles.
  uint32_t nmatch;
  for(nmatch = 1; nmatch < 200000; ++nmatch)
    if(FastSqrt(nmatch) != (uint32_t)sqrtl(nmatch))
      break;
  TEST(nmatch == 200000);

  //static const int NUM=100000;
  //float _numrand[NUM];
  //for(size_t i = 0; i < NUM; ++i)
  //  _numrand[i]=bun_RandReal(0,100.0f);

  //int add=0;
  //uint64_t prof = HighPrecisionTimer::OpenProfiler();
  //CPU_Barrier();
  //for(size_t i = 0; i < NUM; ++i)
  //  //add+=(int)_numrand[i];
  //  add+=fFastTruncate(_numrand[i]);
  //CPU_Barrier();
  //auto res = HighPrecisionTimer::CloseProfiler(prof);
  //double avg = res/(double)NUM;
  //TEST(add>-1);
  //std::cout << "\n" << avg << std::endl;

  TEST(fFastRound(5.0f) == 5);
  TEST(fFastRound(5.000001f) == 5);
  TEST(fFastRound(4.999999f) == 5);
  TEST(fFastRound(4.500001f) == 5);
  TEST(fFastRound(4.5f) == 4);
  TEST(fFastRound(5.5f) == 6);
  TEST(fFastRound(5.9f) == 6);
  TEST(fFastRound(5.0) == 5);
  TEST(fFastRound(5.000000000) == 5);
  TEST(fFastRound(4.999999999) == 5);
  TEST(fFastRound(4.500001) == 5);
  TEST(fFastRound(4.5) == 4);
  TEST(fFastRound(5.5) == 6);
  TEST(fFastRound(5.9) == 6);
  TEST(fFastTruncate(5.0f) == 5);
  TEST(fFastTruncate(5.000001f) == 5);
  TEST(fFastTruncate(4.999999f) == 4);
  TEST(fFastTruncate(4.5f) == 4);
  TEST(fFastTruncate(5.5f) == 5);
  TEST(fFastTruncate(5.9f) == 5);
  TEST(fFastTruncate(5.0) == 5);
  TEST(fFastTruncate(5.000000000) == 5);
  TEST(fFastTruncate(4.999999999) == 4);
  TEST(fFastTruncate(4.5) == 4);
  TEST(fFastTruncate(5.5) == 5);
  TEST(fFastTruncate(5.9) == 5);

  //TEST(fFastDoubleRound(5.0)==(int)5.0);
  //TEST(fFastDoubleRound(5.0000000001f)==(int)5.0000000001f);
  //TEST(fFastDoubleRound(4.999999999f)==(int)4.999999999f);
  //TEST(fFastDoubleRound(4.5f)==(int)4.5f);
  //TEST(fFastDoubleRound(5.9f)==(int)5.9f); //This test fails, so don't use fFastDoubleRound for precision-critical anything.

  TEST(fCompare(DistSqr(2.0f, 2.0f, 5.0f, 6.0f), 25.0f));
  TEST(fCompare(Dist(2.0f, 2.0f, 5.0f, 6.0f), 5.0f, 40));
  TEST(fCompare(DistSqr(2.0, 2.0, 5.0, 6.0), 25.0));
  TEST(fCompare(Dist(2.0, 2.0, 5.0, 6.0), 5.0, (int64_t)150000)); // Do not use this for precision-critical anything.
  TEST(DistSqr(2, 2, 5, 6) == 5 * 5);
  TEST(Dist(2, 2, 5, 6) == 5); // Yes, you can actually do distance calculations using integers, since we use FastSqrt's integer extension.

  {
    int64_t stuff = 2987452983472384720;
    uint16_t find = 43271;
    TEST(ByteSearch(&stuff, 8, &find, 1) == (((char*)&stuff) + 3));
    TEST(ByteSearch(&stuff, 8, &find, 2) == (((char*)&stuff) + 3));
    TEST(ByteSearch(&stuff, 5, &find, 1) == (((char*)&stuff) + 3));
    TEST(ByteSearch(&stuff, 5, &find, 2) == (((char*)&stuff) + 3));
    TEST(ByteSearch(&stuff, 4, &find, 1) == (((char*)&stuff) + 3));
    TEST(!ByteSearch(&stuff, 4, &find, 2));
    TEST(!ByteSearch(&stuff, 3, &find, 2));
    TEST(!ByteSearch(&stuff, 0, &find, 1));
    TEST(!ByteSearch(&stuff, 2, &find, 3));
    find = 27344;
    TEST(ByteSearch(&stuff, 2, &find, 2));
    find = 41;
    TEST(ByteSearch(&stuff, 8, &find, 1) == (((char*)&stuff) + 7));
  }

  testbitcount<uint8_t>(__testret);
  testbitcount<uint16_t>(__testret);
  testbitcount<uint32_t>(__testret);
  testbitcount<uint64_t>(__testret);

  auto flog = [](int i) -> int { int r = 0; while(i >>= 1) ++r; return r; };
  for(nmatch = 1; nmatch < 200000; ++nmatch)
  {
    if(bun_Log2(nmatch) != flog(nmatch))
      break;
  }
  TEST(nmatch == 200000);
  for(nmatch = 2; nmatch < INT_MAX; nmatch <<= 1) // You have to do INT_MAX here even though its unsigned, because 10000... is actually less than 1111... and you overflow.
  {
    if(bun_Log2_p2(nmatch) != flog(nmatch))
      break;
  }
  TEST(nmatch == (1 << 31));

  TEST(fCompare(lerp<double>(3, 4, 0.5), 3.5))
    TEST(fCompare(lerp<double>(3, 4, 0), 3.0))
    TEST(fCompare(lerp<double>(3, 4, 1), 4.0))
    TEST(fSmall(lerp<double>(-3, 3, 0.5)))
    TEST(fCompare(lerp<double>(-3, -4, 0.5), -3.5))
    TEST(fCompare(lerp<float>(3, 4, 0.5f), 3.5f))
    TEST(fCompare(lerp<float>(3, 4, 0), 3.0f))
    TEST(fCompare(lerp<float>(3, 4, 1), 4.0f))
    TEST(fSmall(lerp<float>(-3, 3, 0.5f)))
    TEST(fCompare(lerp<float>(-3, -4, 0.5f), -3.5f))

    TEST((IntDiv<int, int>(-5, 2) == -3));
  TEST((IntDiv<int, int>(5, 2) == 2));
  TEST((IntDiv<int, int>(0, 2) == 0));
  TEST((IntDiv<int, int>(-218937542, 378) == -579200));
  TEST((IntDiv<int, int>(INT_MIN, 3) == ((INT_MIN / 3) - 1)));
  TEST((IntDiv<int, int>(INT_MAX, 3) == (INT_MAX / 3)));

  int a = -1;
  FlipEndian(&a);
  TEST(a == -1);
  a = 1920199968;
  FlipEndian(&a);
  TEST(a == 552432498);

  FILE* f;
  FOPEN(f, "testwrite.txt", "w");
  fwrite("test", 1, 4, f);
  fclose(f);

  {
    auto g = LoadFile<char, false>("testwrite.txt");
    TEST(g.second == 4);
    TEST(!strncmp(g.first.get(), "test", 4));
    g = LoadFile<char, true>("testwrite.txt");
    TEST(g.second == 5);
    TEST(!strncmp(g.first.get(), "test", 5));
  }

  TEST(bun_Abs<int8_t>(-1) == 1);
  TEST(bun_Abs<int8_t>(-128) == 128);
  TEST(bun_Abs<int8_t>(127) == 127);
  TEST(bun_Abs<int8_t>(126) == 126);
  TEST(bun_Abs<int8_t>(0) == 0);
  TEST(bun_Abs<int8_t>(1) == 1);
  TEST(bun_Abs<int8_t>(-127) == 127);
  TEST(bun_Abs<uint8_t>(127) == 127);
  TEST(bun_Abs<uint8_t>(126) == 126);
  TEST(bun_Abs<uint8_t>(0) == 0);
  TEST(bun_Abs<uint8_t>(1) == 1);

  TEST(bun_Negate<uint8_t>(1, 1) == -1);
  TEST(bun_Negate<uint8_t>(128, 1) == -128);
  TEST(bun_Negate<uint8_t>(127, 0) == 127);
  TEST(bun_Negate<uint8_t>(126, 0) == 126);
  TEST(bun_Negate<uint8_t>(0, 0) == 0);
  TEST(bun_Negate<uint8_t>(1, 0) == 1);
  TEST(bun_Negate<uint8_t>(127, 1) == -127);

  using internal::_bunmultiplyextract;

  for(uint16_t i = 0; i <= 255; ++i)
    for(uint16_t j = 0; j <= 255; ++j)
      for(uint8_t k = 0; k <= 16; ++k)
      {
        TEST(_bunmultiplyextract<uint8_t>((uint8_t)i, (uint8_t)j, k) == bun_MultiplyExtract<uint8_t>((uint8_t)i, (uint8_t)j, k));
      }


  for(int16_t i = -128; i <= 127; ++i)
    for(int16_t j = -128; j <= 127; ++j)
      for(int8_t k = 0; k <= 16; ++k)
      {
        TEST(_bunmultiplyextract<int8_t>((int8_t)i, (int8_t)j, k) == bun_MultiplyExtract<int8_t>((int8_t)i, (int8_t)j, k));
      }

  TEST(_bunmultiplyextract<uint64_t>(0, 0, 0) == 0);
  TEST(_bunmultiplyextract<uint64_t>(1, 1, 0) == 1);
  TEST(_bunmultiplyextract<uint64_t>(1, 1, 1) == 0);
  TEST(_bunmultiplyextract<uint64_t>(1, 1, 64) == 0);
  TEST(_bunmultiplyextract<uint64_t>(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0) == 1);
  TEST(_bunmultiplyextract<uint64_t>(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 64) == 0xFFFFFFFFFFFFFFFE);
  TEST(_bunmultiplyextract<uint64_t>(0xFFFFFFFFFFFFFFFF, 0, 0) == 0);
  TEST(_bunmultiplyextract<uint64_t>(0xFFFFFFFFFFFFFFFF, 0, 64) == 0);
  TEST(_bunmultiplyextract<uint64_t>(0xFFFFFFFFFFFFFFFF, 1, 0) == 0xFFFFFFFFFFFFFFFF);
  TEST(_bunmultiplyextract<uint64_t>(0xFFFFFFFFFFFFFFFF, 1, 64) == 0);
  TEST(_bunmultiplyextract<int64_t>(0, 0, 0) == 0);
  TEST(_bunmultiplyextract<int64_t>(1, 1, 0) == 1);
  TEST(_bunmultiplyextract<int64_t>(1, -1, 0) == -1);
  TEST(_bunmultiplyextract<int64_t>(-1, 1, 0) == -1);
  TEST(_bunmultiplyextract<int64_t>(-1, -1, 0) == 1);
  TEST(_bunmultiplyextract<int64_t>(1, 1, 1) == 0);
  TEST(_bunmultiplyextract<int64_t>(-1, -1, 1) == 0);
  TEST(_bunmultiplyextract<int64_t>(1, -1, 1) == -1);
  TEST(_bunmultiplyextract<int64_t>(1, 1, 64) == 0);
  TEST(_bunmultiplyextract<int64_t>(-1, -1, 64) == 0);
  TEST(_bunmultiplyextract<int64_t>(-1, 1, 64) == -1);
  TEST(_bunmultiplyextract<int64_t>(0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF, 0) == 1);
  TEST(_bunmultiplyextract<int64_t>(0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF, 64) == 0x3FFFFFFFFFFFFFFF);
  TEST(_bunmultiplyextract<int64_t>(0x7FFFFFFFFFFFFFFF, 0, 0) == 0);
  TEST(_bunmultiplyextract<int64_t>(0x7FFFFFFFFFFFFFFF, 0, 64) == 0);
  TEST(_bunmultiplyextract<int64_t>(0x7FFFFFFFFFFFFFFF, 1, 0) == 0x7FFFFFFFFFFFFFFF);
  TEST(_bunmultiplyextract<int64_t>(0x7FFFFFFFFFFFFFFF, 1, 64) == 0);
  TEST(_bunmultiplyextract<int64_t>(0x8000000000000000, 1, 0) == 0x8000000000000000);
  TEST(_bunmultiplyextract<int64_t>(0x8000000000000000, 1, 1) == 0xC000000000000000);
  TEST(_bunmultiplyextract<int64_t>(0x8000000000000000, 1, 64) == -1);
  TEST(_bunmultiplyextract<int64_t>(0x8000000000000000, 0, 0) == 0);
  TEST(_bunmultiplyextract<int64_t>(0x8000000000000000, 0, 64) == 0);

  std::string s = std::format("{10} {0}{1} {2}{3}{4}{{5}}{6}0{7}00{8} {9}", 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
  TEST(s == "10 01 234{5}607008 9");
  s = std::format("0{{000}} {{%}}{0}", "{s0}");
  TEST(s == "0{000} {%}{s0}");
  s = std::format("{0}", 0, 1, 2);
  TEST(s == "0");

  ENDTEST;
}