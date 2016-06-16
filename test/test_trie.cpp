// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "cTrie.h"
#include "bss_algo.h"

using namespace bss_util;

TESTDEF::RETPAIR test_TRIE()
{
  BEGINTEST;
  const char* strs[] = { "fail","on","tex","rot","ro","ti","ontick","ondestroy","te","tick" };
  cTrie<uint8_t> t(9, "tick", "on", "tex", "rot", "ro", "ti", "ontick", "ondestroy", "te", "tick");
  cTrie<uint8_t> t2(9, strs);
  cTrie<uint8_t> t3(strs);
  TEST(t3["fail"] == 0);
  TEST(t3["tick"] == 9);

  //cStr randcstr[200];
  //const char* randstr[200];
  //for(uint32_t i = 0; i < 200; ++i)
  //{
  //  for(uint32_t j = bssrandint(2,20); j>0; --j)
  //    randcstr[i]+=(char)bssrandint('a','z');
  //  randstr[i]=randcstr[i];
  //}
  //cTrie<uint32_t> t(50,randstr);
  //cKhash_String<uint8_t> hashtest;
  //for(uint32_t i = 0; i < 50; ++i)
  //  hashtest.Insert(randstr[i],i);
  //uint32_t dm;
  //shuffle(testnums);
  //auto prof = cHighPrecisionTimer::OpenProfiler();
  //CPU_Barrier();
  //for(uint32_t i = 0; i < TESTNUM; ++i)
  //  //dm=hashtest[randstr[testnums[i]%200]];
  //  dm=t[randstr[testnums[i]%200]];
  //  //dm=t[strs[testnums[i]%10]];
  //CPU_Barrier();
  //auto res = cHighPrecisionTimer::CloseProfiler(prof);
  //std::cout << dm << "\nTIME:" << res << std::endl;

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

  ENDTEST;
}
