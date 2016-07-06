// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "bss_util.h"
#include "bss_algo.h"

using namespace bss_util;

template<uint8_t B, int64_t SMIN, int64_t SMAX, uint64_t UMIN, uint64_t UMAX, typename T>
inline void TEST_BitLimit()
{
  static_assert(std::is_same<T, typename BitLimit<B>::SIGNED>::value, "BitLimit failure" MAKESTRING(B));
  static_assert(std::is_same<typename std::make_unsigned<T>::type, typename BitLimit<B>::UNSIGNED>::value, "BitLimit failure" MAKESTRING(B));
#ifndef BSS_COMPILER_CLANG // Clang refuses to recognize this as constant
  static_assert(BitLimit<B>::SIGNED_MIN == SMIN, "BitLimit failure" MAKESTRING(B));
#endif
  static_assert(BitLimit<B>::SIGNED_MAX == SMAX, "BitLimit failure" MAKESTRING(B));
  static_assert(BitLimit<B>::UNSIGNED_MIN == UMIN, "BitLimit failure" MAKESTRING(B));
  static_assert(BitLimit<B>::UNSIGNED_MAX == UMAX, "BitLimit failure" MAKESTRING(B));
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
    TEST(naivebitcount<T>(i) == bitcount<T>(i));
  }
}

TESTDEF::RETPAIR test_bss_util()
{
  BEGINTEST;
  TESTNOERROR(SetWorkDirToCur());
  TEST(bssFileSize(GetProgramPath()) != 0);
  //TEST(bssFileSize(cStrW(fbuf))!=0);
  TESTNOERROR(GetTimeZoneMinutes());

  static_assert(std::is_same<BitLimit<sizeof(uint8_t) << 3>::SIGNED, char>::value, "Test Failure Line #" MAKESTRING(__LINE__));
  static_assert(std::is_same<BitLimit<sizeof(uint16_t) << 3>::SIGNED, short>::value, "Test Failure Line #" MAKESTRING(__LINE__));
  static_assert(std::is_same<BitLimit<sizeof(int) << 3>::SIGNED, int>::value, "Test Failure Line #" MAKESTRING(__LINE__));
  static_assert(std::is_same<BitLimit<sizeof(char) << 3>::UNSIGNED, uint8_t>::value, "Test Failure Line #" MAKESTRING(__LINE__));
  static_assert(std::is_same<BitLimit<sizeof(short) << 3>::UNSIGNED, uint16_t>::value, "Test Failure Line #" MAKESTRING(__LINE__));
  static_assert(std::is_same<BitLimit<sizeof(uint32_t) << 3>::UNSIGNED, uint32_t>::value, "Test Failure Line #" MAKESTRING(__LINE__));
  static_assert(std::is_same<BitLimit<sizeof(double) << 3>::SIGNED, int64_t>::value, "Test Failure Line #" MAKESTRING(__LINE__));
  static_assert(std::is_same<BitLimit<sizeof(float) << 3>::SIGNED, int>::value, "Test Failure Line #" MAKESTRING(__LINE__));
  static_assert(std::is_same<BitLimit<sizeof(double) << 3>::UNSIGNED, uint64_t>::value, "Test Failure Line #" MAKESTRING(__LINE__));
  static_assert(std::is_same<BitLimit<sizeof(float) << 3>::UNSIGNED, uint32_t>::value, "Test Failure Line #" MAKESTRING(__LINE__));
  //static_assert(std::is_same<BitLimit<sizeof(long double)<<3>::SIGNED, int64_t>::value, "Test Failure Line #" MAKESTRING(__LINE__)); // long double is not a well defined type
  //static_assert(std::is_same<BitLimit<sizeof(long double)<<3>::UNSIGNED, uint64_t>::value, "Test Failure Line #" MAKESTRING(__LINE__));
  static_assert(T_CHARGETMSB(0) == 0, "Test Failure Line #" MAKESTRING(__LINE__));
  static_assert(T_CHARGETMSB(1) == 1, "Test Failure Line #" MAKESTRING(__LINE__));
  static_assert(T_CHARGETMSB(2) == 2, "Test Failure Line #" MAKESTRING(__LINE__));
  static_assert(T_CHARGETMSB(3) == 2, "Test Failure Line #" MAKESTRING(__LINE__));
  static_assert(T_CHARGETMSB(4) == 4, "Test Failure Line #" MAKESTRING(__LINE__));
  static_assert(T_CHARGETMSB(7) == 4, "Test Failure Line #" MAKESTRING(__LINE__));
  static_assert(T_CHARGETMSB(8) == 8, "Test Failure Line #" MAKESTRING(__LINE__));
  static_assert(T_CHARGETMSB(20) == 16, "Test Failure Line #" MAKESTRING(__LINE__));
  static_assert(T_CHARGETMSB(84) == 64, "Test Failure Line #" MAKESTRING(__LINE__));
  static_assert(T_CHARGETMSB(189) == 128, "Test Failure Line #" MAKESTRING(__LINE__));
  static_assert(T_CHARGETMSB(255) == 128, "Test Failure Line #" MAKESTRING(__LINE__));

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
  // For reference, the above strange bit values are used in fixed-point arithmetic found in bss_fixedpt.h

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
  for(uint32_t i = 0; i < 8; ++i)
    TEST(GetBitMask<uint8_t>(i) == (1 << i));
  for(uint32_t i = 0; i < 32; ++i)
    TEST(GetBitMask<uint32_t>(i) == (1 << i));
  for(uint32_t i = 0; i < 64; ++i)
    TEST(GetBitMask<unsigned long long>(i) == (((uint64_t)1) << i));

  int bits = 9;
  bits = bssSetBit(bits, 4, true);
  TEST(bits == 13);
  bits = bssSetBit(bits, 8, false);
  TEST(bits == 5);

  std::string cpan(PANGRAM);

  strreplace(const_cast<char*>(cpan.c_str()), 'm', '?');
  TEST(!strchr(cpan.c_str(), 'm') && strchr(cpan.c_str(), '?') != 0);
  std::basic_string<bsschar, std::char_traits<bsschar>, std::allocator<bsschar>> pan;
  for(uint32_t i = 0; i < NPANGRAMS; ++i)
  {
    pan = PANGRAMS[i];
    bsschar f = pan[((i + 7) << 3) % pan.length()];
    bsschar r = pan[((((i * 13) >> 3) + 13) << 3) % pan.length()];
    if(f == r) r = pan[pan.length() - 1];
    strreplace<bsschar>(const_cast<bsschar*>(pan.c_str()), f, r);
#ifdef BSS_PLATFORM_WIN32
    TEST(!wcschr(pan.c_str(), f) && wcschr(pan.c_str(), r) != 0);
#else
    TEST(!strchr(pan.c_str(), f) && strchr(pan.c_str(), r) != 0);
#endif
  }
  TEST(strccount<char>("10010010101110001", '1') == 8);
  TEST(strccount<char>("0100100101011100010", '1') == 8);
  // Linux really hates wchar_t, so getting it to assign the right character is exceedingly difficult. We manually assign 1585 instead.
  TEST(strccount<wchar_t>(TESTUNICODESTR, (wchar_t)1585) == 5);

  int ia = 0;
  int ib = 1;
  std::pair<int, int> sa(1, 2);
  std::pair<int, int> sb(2, 1);
  std::unique_ptr<int[]> ua(new int[2]);
  std::unique_ptr<int[]> ub((int*)0);
  std::string ta("first");
  std::string tb("second");
  rswap(ia, ib);
  TEST(ia == 1);
  TEST(ib == 0);
  rswap(sa, sb);
  TEST((sa == std::pair<int, int>(2, 1)));
  TEST((sb == std::pair<int, int>(1, 2)));
  rswap(ua, ub);
  TEST(ua.get() == 0);
  TEST(ub.get() != 0);
  rswap(ta, tb);
  TEST(ta == "second");
  TEST(tb == "first");

  TEST((intdiv<int, int>(10, 5) == 2));
  TEST((intdiv<int, int>(9, 5) == 1));
  TEST((intdiv<int, int>(5, 5) == 1));
  TEST((intdiv<int, int>(4, 5) == 0));
  TEST((intdiv<int, int>(0, 5) == 0));
  TEST((intdiv<int, int>(-1, 5) == -1));
  TEST((intdiv<int, int>(-5, 5) == -1));
  TEST((intdiv<int, int>(-6, 5) == -2));
  TEST((intdiv<int, int>(-10, 5) == -2));
  TEST((intdiv<int, int>(-11, 5) == -3));

  TEST((bssmod(-1, 7) == 6));
  TEST((bssmod(-90, 7) == 1));
  TEST((bssmod(6, 7) == 6));
  TEST((bssmod(7, 7) == 0));
  TEST((bssmod(0, 7) == 0));
  TEST((bssmod(1, 7) == 1));
  TEST((bssmod(8, 7) == 1));
  TEST((bssmod(71, 7) == 1));
  TEST((bssmod(1, 1) == 0));
  TEST((bssmod(-1, 2) == 1));

  TEST(fcompare(bssfmod(-1.0f, PI_DOUBLEf), 5.2831853f, 10));
  TEST(fcompare(bssfmod(-4.71f, PI_DOUBLEf), 1.57319f, 100));
  TEST(fcompare(bssfmod(1.0f, PI_DOUBLEf), 1.0f));
  TEST((bssfmod(0.0f, PI_DOUBLEf)) == 0.0f);
  TEST(fcompare(bssfmod(12.0f, PI_DOUBLEf), 5.716814f, 10));
  TEST(fcompare(bssfmod(90.0f, PI_DOUBLEf), 2.0354057f, 100));
  TEST(fcompare(bssfmod(-90.0f, PI_DOUBLEf), 4.2477796f, 10));

  int r[] = { -1,0,2,3,4,5,6 };
  int rr[] = { 6,5,4,3,2,0,-1 };
  bssreverse(r);
  TESTARRAY(r, return (r[0] == rr[0]);)

    const char* LTRIM = "    trim ";
  TEST(!strcmp(strltrim(LTRIM), "trim "));
  char RTRIM[] = { ' ','t','r','i','m',' ',' ',0 }; // :|
  TEST(!strcmp(strrtrim(RTRIM), " trim"));
  RTRIM[5] = ' ';
  TEST(!strcmp(strtrim(RTRIM), "trim"));

  uint32_t nsrc[] = { 0,1,2,3,4,5,10,13,21,2873,3829847,2654435766 };
  uint32_t num[] = { 1,2,4,5,7,8,17,21,34,4647,6193581,4292720341 };
  transform(nsrc, &fbnext<uint32_t>);
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

  TEST(tsign(2.8) == 1)
    TEST(tsign(-2.8) == -1)
    TEST(tsign(23897523987453.8f) == 1)
    TEST(tsign((int64_t)0) == 1)
    TEST(tsign(0.0) == 1)
    TEST(tsign(0.0f) == 1)
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

    TEST(fcompare(angledist(PI, PI_HALF), PI_HALF))
    TEST(fsmall(angledist(PI, PI + PI_DOUBLE)))
    TEST(fcompare(angledist(PI_DOUBLE + PI, PI + PI_HALF*7.0), PI_HALF))
    TEST(fcompare(angledist(PI + PI_HALF*7.0, PI_DOUBLE + PI), PI_HALF))
    TEST(fcompare(angledist(PIf, PI_HALFf), PI_HALFf))
    TEST(fsmall(angledist(PIf, PIf + PI_DOUBLEf), FLT_EPS * 4))
    TEST(fcompare(angledist(PI_DOUBLEf + PIf, PIf + PI_HALFf*7.0f), PI_HALFf, 9))
    TEST(fcompare(angledist(PIf + PI_HALFf*7.0f, PI_DOUBLEf + PIf), PI_HALFf, 9))
    TEST(fcompare(angledist(PIf + PI_HALFf*7.0f, PI_DOUBLEf + PIf), PI_HALFf, 9))
    TEST(fcompare(angledist(1.28f, -4.71f), 0.2931f, 10000))
    TEST(fcompare(angledist(1.28f, -4.71f + PI_DOUBLEf), 0.2931f, 10000))

    TEST(fcompare(angledistsgn(PI, PI_HALF), -PI_HALF))
    TEST(fsmall(angledistsgn(PI, PI + PI_DOUBLE)))
    TEST(fcompare(angledistsgn(PI_DOUBLE + PI, PI + PI_HALF*7.0), -PI_HALF))
    TEST(fcompare(angledistsgn(PI_HALF, PI), PI_HALF))
    TEST(fcompare(angledistsgn(PIf, PI_HALFf), -PI_HALFf))
    TEST(fsmall(angledistsgn(PIf, PIf + PI_DOUBLEf), -FLT_EPS * 4))
    TEST(fcompare(angledistsgn(PI_DOUBLEf + PIf, PIf + PI_HALFf*7.0f), -PI_HALFf, 9))
    TEST(fcompare(angledistsgn(PI_HALFf, PIf), PI_HALFf))
    TEST(fcompare(angledistsgn(1.28f, -4.71f), 0.2931f, 10000))
    TEST(fcompare(angledistsgn(1.28f, -4.71f + PI_DOUBLEf), 0.2931f, 10000))

    const float flt = FLT_EPSILON;
  int32_t fi = *(int32_t*)(&flt);
  TEST(fsmall(*(float*)(&(--fi))))
    TEST(fsmall(*(float*)(&(++fi))))
    TEST(!fsmall(*(float*)(&(++fi))))
    const double dbl = DBL_EPSILON;
  int64_t di = *(int64_t*)(&dbl);
  TEST(fsmall(*(double*)(&(--di))))
    TEST(fsmall(*(double*)(&(++di))))
    TEST(!fsmall(*(double*)(&(++di))))

    TEST(fcompare(1.0f, 1.0f))
    TEST(fcompare(1.0f, 1.0f + FLT_EPSILON))
    TEST(fcompare(10.0f, 10.0f + FLT_EPSILON * 10))
    TEST(fcompare(10.0f, 10.0f))
    TEST(!fcompare(0.1f, 0.1f + FLT_EPSILON*0.1f))
    TEST(!fcompare(0.1f, FLT_EPSILON))
    TEST(fcompare(1.0, 1.0))
    TEST(fcompare(1.0, 1.0 + DBL_EPSILON))
    TEST(fcompare(10.0, 10.0 + DBL_EPSILON * 10))
    TEST(fcompare(10.0, 10.0))
    TEST(!fcompare(0.1, 0.1 + DBL_EPSILON*0.1))
    TEST(!fcompare(0.1, DBL_EPSILON))

    // This tests our average aggregation formula, which lets you average extremely large numbers while maintaining a fair amount of precision.
    uint64_t total = 0;
  uint32_t nc;
  double avg = 0;
  double diff = 0.0;
  for(nc = 1; nc < 10000; ++nc)
  {
    total += nc*nc;
    avg = bssavg<double>(avg, (double)(nc*nc), nc);
    diff = bssmax(diff, fabs((total / (double)nc) - avg));
  }
  TEST(diff<FLT_EPSILON * 2);

  // FastSqrt testing ground
  //
  //float a=2;
  //float b;
  //double sqrt_avg=0;
  //float NUMBERS[100000];
  //for(uint32_t i = 0; i < 100000; ++i)
  //  NUMBERS[i]=bssrandreal(2,4);

  //uint64_t p=cHighPrecisionTimer::OpenProfiler();
  //CPU_Barrier();
  //for(uint32_t j = 0; j < 10; ++j)
  //{
  //for(uint32_t i = 0; i < 100000; ++i)
  //{
  //  a=NUMBERS[i];
  //  b=std::sqrtf(a);
  //}
  //for(uint32_t i = 0; i < 100000; ++i)
  //{
  //  a=NUMBERS[i];
  //  b=FastSqrtsse(a);
  //}
  //for(uint32_t i = 0; i < 100000; ++i)
  //{
  //  a=NUMBERS[i];
  //  b=FastSqrt(a);
  //}
  //}
  //CPU_Barrier();
  //sqrt_avg=cHighPrecisionTimer::CloseProfiler(p);
  //
  //TEST(b==a); //keep things from optimizing out
  //cout << sqrt_avg << std::endl;
  //CPU_Barrier();
  double ddbl = fabs(FastSqrt(2.0) - sqrt(2.0));
#ifdef BSS_COMPILER_GCC
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
  //for(uint32_t i = 0; i < NUM; ++i)
  //  _numrand[i]=bssrandreal(0,100.0f);

  //int add=0;
  //uint64_t prof = cHighPrecisionTimer::OpenProfiler();
  //CPU_Barrier();
  //for(uint32_t i = 0; i < NUM; ++i)
  //  //add+=(int)_numrand[i];
  //  add+=fFastTruncate(_numrand[i]);
  //CPU_Barrier();
  //auto res = cHighPrecisionTimer::CloseProfiler(prof);
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

  TEST(fcompare(distsqr(2.0f, 2.0f, 5.0f, 6.0f), 25.0f));
  TEST(fcompare(dist(2.0f, 2.0f, 5.0f, 6.0f), 5.0f, 40));
  TEST(fcompare(distsqr(2.0, 2.0, 5.0, 6.0), 25.0));
  TEST(fcompare(dist(2.0, 2.0, 5.0, 6.0), 5.0, (int64_t)150000)); // Do not use this for precision-critical anything.
  TEST(distsqr(2, 2, 5, 6) == 5 * 5);
  TEST(dist(2, 2, 5, 6) == 5); // Yes, you can actually do distance calculations using integers, since we use FastSqrt's integer extension.

  int64_t stuff = 2987452983472384720;
  uint16_t find = 43271;
  TEST(bytesearch(&stuff, 8, &find, 1) == (((char*)&stuff) + 3));
  TEST(bytesearch(&stuff, 8, &find, 2) == (((char*)&stuff) + 3));
  TEST(bytesearch(&stuff, 5, &find, 1) == (((char*)&stuff) + 3));
  TEST(bytesearch(&stuff, 5, &find, 2) == (((char*)&stuff) + 3));
  TEST(bytesearch(&stuff, 4, &find, 1) == (((char*)&stuff) + 3));
  TEST(!bytesearch(&stuff, 4, &find, 2));
  TEST(!bytesearch(&stuff, 3, &find, 2));
  TEST(!bytesearch(&stuff, 0, &find, 1));
  TEST(!bytesearch(&stuff, 2, &find, 3));
  find = 27344;
  TEST(bytesearch(&stuff, 2, &find, 2));
  find = 41;
  TEST(bytesearch(&stuff, 8, &find, 1) == (((char*)&stuff) + 7));

  testbitcount<uint8_t>(__testret);
  testbitcount<uint16_t>(__testret);
  testbitcount<uint32_t>(__testret);
  testbitcount<uint64_t>(__testret);

  auto flog = [](int i) -> int { int r = 0; while(i >>= 1) ++r; return r; };
  for(nmatch = 1; nmatch < 200000; ++nmatch)
  {
    if(bsslog2(nmatch) != flog(nmatch))
      break;
  }
  TEST(nmatch == 200000);
  for(nmatch = 2; nmatch < INT_MAX; nmatch <<= 1) // You have to do INT_MAX here even though its unsigned, because 10000... is actually less than 1111... and you overflow.
  {
    if(bsslog2_p2(nmatch) != flog(nmatch))
      break;
  }
  TEST(nmatch == (1 << 31));

  TEST(fcompare(lerp<double>(3, 4, 0.5), 3.5))
    TEST(fcompare(lerp<double>(3, 4, 0), 3.0))
    TEST(fcompare(lerp<double>(3, 4, 1), 4.0))
    TEST(fsmall(lerp<double>(-3, 3, 0.5)))
    TEST(fcompare(lerp<double>(-3, -4, 0.5), -3.5))
    TEST(fcompare(lerp<float>(3, 4, 0.5f), 3.5f))
    TEST(fcompare(lerp<float>(3, 4, 0), 3.0f))
    TEST(fcompare(lerp<float>(3, 4, 1), 4.0f))
    TEST(fsmall(lerp<float>(-3, 3, 0.5f)))
    TEST(fcompare(lerp<float>(-3, -4, 0.5f), -3.5f))

    TEST((intdiv<int, int>(-5, 2) == -3));
  TEST((intdiv<int, int>(5, 2) == 2));
  TEST((intdiv<int, int>(0, 2) == 0));
  TEST((intdiv<int, int>(-218937542, 378) == -579200));
  TEST((intdiv<int, int>(INT_MIN, 3) == ((INT_MIN / 3) - 1)));
  TEST((intdiv<int, int>(INT_MAX, 3) == (INT_MAX / 3)));

  int a = -1;
  flipendian(&a);
  TEST(a == -1);
  a = 1920199968;
  flipendian(&a);
  TEST(a == 552432498);

  FILE* f;
  FOPEN(f, "testwrite.txt", "w");
  fwrite("test", 1, 4, f);
  fclose(f);

  {
    auto g = bssloadfile<char, false>("testwrite.txt");
    TEST(g.second == 4);
    TEST(!strncmp(g.first.get(), "test", 4));
    g = bssloadfile<char, true>("testwrite.txt");
    TEST(g.second == 5);
    TEST(!strncmp(g.first.get(), "test", 5));
  }


  ENDTEST;
}