// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss_util_c.h"
#include <functional>
#include "bss_algo.h"
#include "test.h"

using namespace bss_util;

TESTDEF::RETPAIR test_bss_util_c()
{
  BEGINTEST;
  //_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
  //int a=0;
  //uint64_t prof = cHighPrecisionTimer::OpenProfiler();
  //CPU_Barrier();
  //for(int i = 0; i < 1000000; ++i) {
  //  a+= bssrandint(9,12);
  //}
  //CPU_Barrier();
  //std::cout << cHighPrecisionTimer::CloseProfiler(prof) << " :( " << a << std::endl;

  //{
  //  float aaaa = 0;
  //  uint64_t prof = cHighPrecisionTimer::OpenProfiler();
  //  CPU_Barrier();
  //  for(int i = 0; i < TESTNUM; ++i)
  //    aaaa += bssfmod<float>(-1.0f, testnums[i]+PIf);
  //  CPU_Barrier();
  //  std::cout << cHighPrecisionTimer::CloseProfiler(prof) << std::endl;
  //  std::cout << aaaa;
  //}

  std::function<void()> b([]() { int i = 0; i += 1; return; });
  b = std::move(std::function<void()>([]() { return; }));

  bssCPUInfo info = bssGetCPUInfo();
  TEST(info.cores>0);
  TEST(info.SSE>2); // You'd better support at least SSE2
#ifdef BSS_64BIT
  TEST(info.flags & 2); // You also have to support cmpxchg16b or lockless.h will explode
#else
  TEST(info.flags & 1); // You also have to support cmpxchg8b or lockless.h will explode
#endif

  char buf[6];
  TEST(itoa_r(238907, 0, 0, 10) == 22);
  TEST(itoa_r(238907, buf, 0, 10) == 22);
  TEST(itoa_r(238907, buf, -2, 10) == 22);
  TEST(itoa_r(238907, buf, 1, 10) == 22);
  itoa_r(238907, buf, 2, 10);
  TEST(!strcmp(buf, "7"));
  itoa_r(-238907, buf, 2, 10);
  TEST(!strcmp(buf, "-"));
  itoa_r(238907, buf, 5, 10);
  TEST(!strcmp(buf, "8907"));
  itoa_r(238907, buf, 6, 10);
  TEST(!strcmp(buf, "38907"));
  _itoa_r(238907, buf, 10);
  TEST(!strcmp(buf, "38907"));
  _itoa_r(907, buf, 10);
  TEST(!strcmp(buf, "907"));
  _itoa_r(-238907, buf, 10);
  TEST(!strcmp(buf, "-8907"));
  _itoa_r(-907, buf, 10);
  TEST(!strcmp(buf, "-907"));
  _itoa_r(-0, buf, 10);
  TEST(!strcmp(buf, "0"));
  _itoa_r(1, buf, 10);
  TEST(!strcmp(buf, "1"));
  _itoa_r(-1, buf, 10);
  TEST(!strcmp(buf, "-1"));

  TEST(GetWorkingSet() != 0);
  TEST(GetPeakWorkingSet() != 0);

  int outstr[1024];
  wchar_t wstr[1024];
  char instr[1024];
  for(int i = 0; i < 1023; ++i) instr[i] = (char)bssrandint(0, 256);
  instr[1023] = 0;

  size_t len = UTF8toUTF32(instr, -1, 0, 0);
  TEST(len < 1024);
  TEST(UTF8toUTF32(instr, -1, outstr, len) <= len);

  for(int i = 0; i < 255; ++i) outstr[i] = (int)bssrandint(0, 0xFFFFFFFF);
  outstr[255] = 0;

  len = UTF32toUTF8(outstr, -1, 0, 0);
  TEST(len < 1024);
  TEST(UTF32toUTF8(outstr, -1, instr, len) <= len);

#ifdef BSS_PLATFORM_WIN32
  for(int i = 0; i < sizeof(PANGRAMS) / sizeof(const bsschar*); ++i)
  {
    size_t l = UTF16toUTF32(PANGRAMS[i], -1, 0, 0);
    assert(l < 1024);
    UTF16toUTF32(PANGRAMS[i], -1, outstr, l);
    l = UTF32toUTF8(outstr, -1, 0, 0);
    assert(l < 1024);
    UTF32toUTF8(outstr, -1, instr, l);
    l = UTF8toUTF32(instr, -1, 0, 0);
    assert(l < 1024);
    UTF8toUTF32(instr, -1, outstr, l);
    l = UTF32toUTF16(outstr, -1, 0, 0);
    assert(l < 1024);
    UTF32toUTF16(outstr, -1, wstr, l);
    TEST(!wcscmp(wstr, PANGRAMS[i]));
  }
#endif
  ENDTEST;
}