// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/bss_util_c.h"
#include <functional>
#include "bss-util/bss_algo.h"
#include "test.h"

using namespace bss;

template<typename CHAR>
bool arbitrarycomp(const CHAR* l, const CHAR* r, ptrdiff_t n = -1)
{
  assert(l && r);
  while(*l && *r && --n)
  {
    if(*l != *r)
      return false;
    ++l;
    ++r;
  }

  return *l == *r;
}

template<typename CHAR>
std::string arbitraryview(const CHAR* src, size_t len)
{
  std::string r;
  for(size_t i = 0; i < len; ++i)
    r += (char)src[i];
  return r;
}
template<typename FROM, typename TO>
void testconvfunc(const FROM* from, size_t szfrom, const TO* to, size_t(f)(const FROM*BSS_RESTRICT, ptrdiff_t, TO*BSS_RESTRICT, size_t), TESTDEF::RETPAIR& __testret)
{
  size_t len = f(from, -1, 0, 0);
  DYNARRAY(TO, target1, len);
  len = f(from, -1, target1, len);
  TEST(len == szfrom + 1);
  TEST(target1[len - 1] == 0);
  TEST(arbitrarycomp<TO>(target1, to));

  len = f(from, szfrom + 1, 0, 0);
  DYNARRAY(TO, target2, len);
  len = f(from, szfrom + 1, target2, len);
  TEST(len == szfrom + 1);
  TEST(target2[len - 1] == 0);
  TEST(arbitrarycomp<TO>(target2, to));

  len = f(from, szfrom, 0, 0);
  DYNARRAY(TO, target3, len);
  len = f(from, szfrom, target3, len);
  TEST(len == szfrom);
  TEST(target3[len - 1] != 0);
  TEST(arbitrarycomp<TO>(target3, to, len));

  len = f(from, szfrom - 1, 0, 0);
  DYNARRAY(TO, target4, len);
  len = f(from, szfrom - 1, target4, len);
  TEST(len == szfrom - 1);
  TEST(target4[len - 1] != 0);
  TEST(arbitrarycomp<TO>(target4, to, len));
}

TESTDEF::RETPAIR test_bss_util_c()
{
  BEGINTEST;
  //_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
  //int a=0;
  //uint64_t prof = HighPrecisionTimer::OpenProfiler();
  //CPU_Barrier();
  //for(int i = 0; i < 1000000; ++i) {
  //  a+= bssRandInt(9,12);
  //}
  //CPU_Barrier();
  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << " :( " << a << std::endl;

  //{
  //  float aaaa = 0;
  //  uint64_t prof = HighPrecisionTimer::OpenProfiler();
  //  CPU_Barrier();
  //  for(int i = 0; i < TESTNUM; ++i)
  //    aaaa += bssFMod<float>(-1.0f, testnums[i]+PIf);
  //  CPU_Barrier();
  //  std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;
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
  for(int i = 0; i < 1023; ++i) instr[i] = (char)bssRandInt(0, 256);
  instr[1023] = 0;

  size_t len = UTF8toUTF32(instr, -1, 0, 0);
  TEST(len < 1024);
  TEST(UTF8toUTF32(instr, -1, outstr, len) <= len);

  for(int i = 0; i < 255; ++i) outstr[i] = (int)bssRandInt(0, 0xFFFFFFFF);
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

  const char* conversion1 = "conversion";
  const int conversion3[] = { 'c', 'o', 'n','v','e','r','s','i','o','n',0 };

  testconvfunc<char, int>(conversion1, 10, conversion3, &UTF8toUTF32, __testret);
  testconvfunc<int, char>(conversion3, 10, conversion1, &UTF32toUTF8, __testret);

#ifdef BSS_PLATFORM_WIN32
  const wchar_t* conversion2 = L"conversion";

  testconvfunc<wchar_t, int>(conversion2, 10, conversion3, &UTF16toUTF32, __testret);
  testconvfunc<int, wchar_t>(conversion3, 10, conversion2, &UTF32toUTF16, __testret);
  testconvfunc<wchar_t, char>(conversion2, 10, conversion1, &UTF16toUTF8, __testret);
  testconvfunc<char, wchar_t>(conversion1, 10, conversion2, &UTF8toUTF16, __testret);
#endif

  ENDTEST;
}