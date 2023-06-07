// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/defines.h"
#include <time.h>

using namespace bun;

TESTDEF::RETPAIR test_deprecated()
{
  std::vector<bool> test;
  BEGINTEST;
  time_t tmval = TIME64(nullptr);
  TEST(tmval != 0);
  TIME64(&tmval);
  TEST(tmval != 0);
  tm tms;
  TEST([&]()->bool { GMTIMEFUNC(&tmval, &tms); return true; }())
    //TESTERR(GMTIMEFUNC(0,&tms));
    //char buf[12];
    //#define VSPRINTF(dest,length,format,list) _vsnprintf_s(dest,length,length,format,list)
    //#define VSWPRINTF(dest,length,format,list) _vsnwprintf_s(dest,length,length,format,list)
    FILE* f = 0;
  FOPEN(f, "__valtest.txt", "wb");
  TEST(f != 0);
  if(f != 0)
    fclose(f);
  f = 0;
  WFOPEN(f, BUN__L("石石石石shi.txt"), BUN__L("wb"));
  TEST(f != 0);
  if(f != 0)
    fclose(f);
  size_t a = 0;
  size_t b = -1;
  MEMCPY(&a, sizeof(size_t), &b, sizeof(size_t));
  TEST(a == b);
  a = 0;
  MEMCPY(&a, sizeof(size_t) - 1, &b, sizeof(size_t) - 1);
  TEST(a == (b >> 8));

  char buf[256];
  buf[9] = 0;
  STRNCPY(buf, 11, PANGRAM, 10);
  STRCPY(buf, 256, PANGRAM);
  STRCPYx0(buf, PANGRAM);

#ifdef BUN_PLATFORM_WIN32
  wchar_t wbuf[256];
  wbuf[9] = 0;
  WCSNCPY(wbuf, 11, PANGRAMS[3], 10);
  WCSCPY(wbuf, 256, PANGRAMS[2]);
  WCSCPYx0(wbuf, PANGRAMS[4]);
  TEST(!WCSICMP(L"Kæmi ný", L"kæmi ný"));
#endif

  TEST(!STRICMP("fOObAr", "Foobar"));

  //#define STRTOK(str,delim,context) strtok_s(str,delim,context)
  //#define WCSTOK(str,delim,context) wcstok_s(str,delim,context)
  //#define SSCANF sscanf_s

  ENDTEST;
}
