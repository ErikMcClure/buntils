// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/StringTable.h"
#include <fstream>

using namespace bun;

TESTDEF::RETPAIR test_STRTABLE()
{
  BEGINTEST;

  const int SZ = sizeof(PANGRAMS) / sizeof(const bun_char*);
  Str pangrams[SZ];
  const char* pstr[SZ];
  for(size_t i = 0; i < SZ; ++i)
    pstr[i] = (pangrams[i] = PANGRAMS[i]).c_str();

  StringTable<char> mbstable(pstr, SZ);
  StringTable<bun_char> wcstable(PANGRAMS, SZ);
  StringTable<char> mbstable2(pstr, 6);

  for(size_t i = 0; i < mbstable.Length(); ++i)
    TEST(!strcmp(mbstable[i], pstr[i]));

  mbstable += mbstable2;
  mbstable.AppendString("append");
  mbstable += "append2";

  for(size_t i = 0; i < SZ; ++i)
    TEST(!strcmp(mbstable[i], pstr[i]));
  for(size_t i = 0; i < 6; ++i)
    TEST(!strcmp(mbstable[i + SZ], pstr[i]));
  TEST(!strcmp(mbstable[SZ + 6], "append"));
  TEST(!strcmp(mbstable[SZ + 7], "append2"));

  std::fstream fs;
  fs.open("dump.txt", std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
  mbstable.DumpToStream(&fs);
  fs.close();
  fs.open("dump.txt", std::ios_base::in | std::ios_base::binary);
  StringTable<char> ldtable(&fs, (size_t)bun_FileSize("dump.txt"));
  for(size_t i = 0; i < SZ; ++i)
    TEST(!strcmp(ldtable[i], pstr[i]));
  for(size_t i = 0; i < 6; ++i)
    TEST(!strcmp(ldtable[i + SZ], pstr[i]));

  mbstable2 = ldtable;
  for(size_t i = 0; i < SZ; ++i)
    TEST(!strcmp(mbstable2[i], pstr[i]));
  for(size_t i = 0; i < 6; ++i)
    TEST(!strcmp(mbstable2[i + SZ], pstr[i]));

  ENDTEST;
}
