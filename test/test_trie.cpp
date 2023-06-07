// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/Trie.h"
#include "buntils/algo.h"

using namespace bun;

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
  for(size_t i = 0; i < 200; ++i)
  {
    for(uint32_t j = bun_RandInt(2,20); j>0; --j)
      randcstr[i]+=(char)bun_RandInt('a','z');
    randstr[i]=randcstr[i];
  }
  Trie<uint32_t> t0(50,randstr);
  Hash<const char*,uint8_t> hashtest;
  for(size_t i = 0; i < 50; ++i)
    hashtest.Insert(randstr[i],i);
  uint32_t dm;
  Shuffle(testnums);
  auto prof = HighPrecisionTimer::OpenProfiler();
  CPU_Barrier();
  for(size_t i = 0; i < TESTNUM; ++i)
    dm=hashtest[randstr[testnums[i]%200]];
    //dm= t0[randstr[testnums[i]%200]];
    //dm=t[strs[testnums[i]%10]];
  CPU_Barrier();
  auto res = HighPrecisionTimer::CloseProfiler(prof);
  std::cout << dm << "\nTIME:" << res << std::endl;
  */

  /*
  constexpr char* randcstr[10] = { "aj32k4SX90xj", "k24ht4ki2@j", "fa9udj", "ljafkl3a2k3K", "zkjslww", "xnwmwnps", "lhisknw", "tjdksnx", "nhjnxiw", "yhwwonzlk28" };
  Trie<uint32_t> t0(9, randcstr);
  uint32_t dm;
  Shuffle(testnums);
  auto prof = HighPrecisionTimer::OpenProfiler();
  Hash<const char*, uint8_t> hashtest;
  for(size_t i = 0; i < 9; ++i)
    hashtest.Insert(randcstr[i], i);
  CPU_Barrier();
  for(size_t i = 0; i < TESTNUM; ++i)
  {
    switch(hash_fnv1a(randcstr[testnums[i] % 10]))
    {
    case hash_fnv1a(randcstr[0]): dm = 8; break;
    case hash_fnv1a(randcstr[1]): dm = 28; break;
    case hash_fnv1a(randcstr[2]): dm = 16; break;
    case hash_fnv1a(randcstr[3]): dm = 3; break;
    case hash_fnv1a(randcstr[4]): dm = 5; break;
    case hash_fnv1a(randcstr[5]): dm = 1; break;
    case hash_fnv1a(randcstr[6]): dm = 0; break;
    case hash_fnv1a(randcstr[7]): dm = 17; break;
    case hash_fnv1a(randcstr[8]): dm = 29; break;
    default: dm = -1; break;
    }
    //dm= t0[randcstr[testnums[i]%10]];
    //dm = hashtest[randcstr[testnums[i] % 10]];
  }

  CPU_Barrier();
  auto res = HighPrecisionTimer::CloseProfiler(prof);
  std::cout << dm << "\nTIME:" << res << std::endl;*/

  for(size_t i = 0; i < 9; ++i)
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

  for(size_t i = 0; i < 11; ++i)
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

  for(size_t i = 0; i < 9; ++i)
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
