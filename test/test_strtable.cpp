// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "cStrTable.h"
#include <fstream>

using namespace bss_util;

TESTDEF::RETPAIR test_STRTABLE()
{
  BEGINTEST;

  const int SZ = sizeof(PANGRAMS) / sizeof(const bsschar*);
  cStr pangrams[SZ];
  const char* pstr[SZ];
  for(uint32_t i = 0; i < SZ; ++i)
    pstr[i] = (pangrams[i] = PANGRAMS[i]).c_str();

  cStrTable<char> mbstable(pstr, SZ);
  cStrTable<bsschar> wcstable(PANGRAMS, SZ);
  cStrTable<char> mbstable2(pstr, 6);

  for(uint32_t i = 0; i < mbstable.Length(); ++i)
    TEST(!strcmp(mbstable[i], pstr[i]));

  mbstable += mbstable2;
  mbstable.AppendString("append");
  mbstable += "append2";

  for(int i = 0; i < SZ; ++i)
    TEST(!strcmp(mbstable[i], pstr[i]));
  for(int i = 0; i < 6; ++i)
    TEST(!strcmp(mbstable[i + SZ], pstr[i]));
  TEST(!strcmp(mbstable[SZ + 6], "append"));
  TEST(!strcmp(mbstable[SZ + 7], "append2"));

  std::fstream fs;
  fs.open("dump.txt", std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
  mbstable.DumpToStream(&fs);
  fs.close();
  fs.open("dump.txt", std::ios_base::in | std::ios_base::binary);
  cStrTable<char> ldtable(&fs, (size_t)bssFileSize("dump.txt"));
  for(int i = 0; i < SZ; ++i)
    TEST(!strcmp(ldtable[i], pstr[i]));
  for(int i = 0; i < 6; ++i)
    TEST(!strcmp(ldtable[i + SZ], pstr[i]));

  mbstable2 = ldtable;
  for(int i = 0; i < SZ; ++i)
    TEST(!strcmp(mbstable2[i], pstr[i]));
  for(int i = 0; i < 6; ++i)
    TEST(!strcmp(mbstable2[i + SZ], pstr[i]));

  ENDTEST;
}
