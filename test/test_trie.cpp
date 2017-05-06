// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/Trie.h"
#include "bss-util/algo.h"
#include "test.h"

using namespace bss;

TESTDEF::RETPAIR test_TRIE()
{
  BEGINTEST;
  const char* strs[] = { "fail","on","tex","rot","ro","ti","ontick","ondestroy","te","tick" };
  Trie<uint8_t> t(9, "tick", "on", "tex", "rot", "ro", "ti", "ontick", "ondestroy", "te", "tick");
  Trie<uint8_t> t2(9, strs);
  Trie<uint8_t> t3(strs);
  TEST(t3["fail"] == 0);
  TEST(t3["tick"] == 9);

  /*
  Str randcstr[200];
  const char* randstr[200];
  for(uint32_t i = 0; i < 200; ++i)
  {
    for(uint32_t j = bssRandInt(2,20); j>0; --j)
      randcstr[i]+=(char)bssRandInt('a','z');
    randstr[i]=randcstr[i];
  }
  Trie<uint32_t> t0(50,randstr);
  Hash<const char*,uint8_t> hashtest;
  for(uint32_t i = 0; i < 50; ++i)
    hashtest.Insert(randstr[i],i);
  uint32_t dm;
  Shuffle(testnums);
  auto prof = HighPrecisionTimer::OpenProfiler();
  CPU_Barrier();
  for(uint32_t i = 0; i < TESTNUM; ++i)
    dm=hashtest[randstr[testnums[i]%200]];
    //dm= t0[randstr[testnums[i]%200]];
    //dm=t[strs[testnums[i]%10]];
  CPU_Barrier();
  auto res = HighPrecisionTimer::CloseProfiler(prof);
  std::cout << dm << "\nTIME:" << res << std::endl;
  */

  for(uint32_t i = 0; i < 9; ++i)
  {
    switch(t[strs[i]]) // Deliberatly meant to test for one failure
    {
    case 0:
      TEST(false); break;
    case 1:
      TEST(i == 1); break;
    case 2:
      TEST(i == 2); break;
    case 3:
      TEST(i == 3); break;
    case 4:
      TEST(i == 4); break;
    case 5:
      TEST(i == 5); break;
    case 6:
      TEST(i == 6); break;
    case 7:
      TEST(i == 7); break;
    case 8:
      TEST(i == 8); break;
    default:
      TEST(i == 0);
    }
  }
  TEST(t[strs[9]] == 0);

  const char* casestr[] = { "tIck", "On", "tEX", "ROT", "RO", "ti", "ONtick", "ONdestROY", "te", "tick", "fail" };
  Trie<uint8_t, false> tcase(10, casestr);
  Trie<uint8_t, true> tins(10, "tIck", "On", "tEX", "ROT", "RO", "ti", "ONtick", "ONdestROY", "te", "tick");

  for(uint32_t i = 0; i < 11; ++i)
  {
    switch(tcase[casestr[i]]) // Deliberatly meant to test for one failure
    {
    case 0:
      TEST(i == 0); break;
    case 1:
      TEST(i == 1); break;
    case 2:
      TEST(i == 2); break;
    case 3:
      TEST(i == 3); break;
    case 4:
      TEST(i == 4); break;
    case 5:
      TEST(i == 5); break;
    case 6:
      TEST(i == 6); break;
    case 7:
      TEST(i == 7); break;
    case 8:
      TEST(i == 8); break;
    case 9:
      TEST(i == 9); break;
    default:
      TEST(i == 10); break;
    case 10:
      TEST(false); break;
    }
  }
  TEST(tcase[strs[9]] == 9);
  TEST(tins["rot"] == 3);

  for(uint32_t i = 0; i < 9; ++i)
  {
    switch(tins[strs[i]]) // Deliberatly meant to test for one failure
    {
    case 0:
      TEST(false); break;
    case 1:
      TEST(i == 1); break;
    case 2:
      TEST(i == 2); break;
    case 3:
      TEST(i == 3); break;
    case 4:
      TEST(i == 4); break;
    case 5:
      TEST(i == 5); break;
    case 6:
      TEST(i == 6); break;
    case 7:
      TEST(i == 7); break;
    case 8:
      TEST(i == 8); break;
    default:
      TEST(i == 0);
    }
  }
  TEST(tins[strs[9]] == 0);

  ENDTEST;
}
