// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "cHash.h"

using namespace bss_util;

TESTDEF::RETPAIR test_HASH()
{
  BEGINTEST;
  //cKhash<int, char,false,KH_INT_HASHFUNC,KH_INT_EQUALFUNC<int>,KH_INT_VALIDATEPTR<int>> hashtest;
  //hashtest.Insert(21354,0);
  //hashtest.Insert(34623,0);
  //hashtest.Insert(52,0);
  //hashtest.Insert(1,0);
  //int r=hashtest.GetIterKey(hashtest.GetIterator(1));
  {
    cHash<int, cLog*> hasherint;
    hasherint.Insert(25, &_failedtests);
    hasherint.Get(25);
    hasherint.Remove(25);
    cHash<const char*, cLog*, true> hasher;
    hasher.Insert("", &_failedtests);
    hasher.Insert("Video", (cLog*)5);
    hasher.SetCapacity(100);
    hasher.Insert("Physics", 0);
    cLog* check = hasher.Get("Video");
    TEST(check == (cLog*)5);
    check = hasher.Get("Video");
    TEST(check == (cLog*)5);
    cHash<const char*, cLog*, true> hasher2(hasher);
    check = hasher2.Get("Video");
    TEST(check == (cLog*)5);
    cHash<const char*, cLog*, true> hasher3(std::move(hasher));
    check = hasher2.Get("Video");
    TEST(check == (cLog*)5);
    //uint64_t diff = cHighPrecisionTimer::CloseProfiler(ID);

    {
      cHash<const void*> set;
      set.Insert(0);
      set.Insert(&check);
      set.Insert(&hasher);
      set.Insert(&hasherint);
      set.Insert(&set);
      set.GetKey(0);
      TEST(set.Exists(0));
    }
    {
      typedef std::pair<uint64_t, int32_t> HASHTESTPAIR;
      cHash<HASHTESTPAIR> set;
      set.Insert(HASHTESTPAIR(0, 0));
      set.Insert(HASHTESTPAIR(1ULL << 34, 3));
      set.Insert(HASHTESTPAIR(2, -5));
      set.Insert(HASHTESTPAIR(0, 0));
      set.GetKey(0);
      TEST(set.Exists(HASHTESTPAIR(0, 0)));
      TEST(set.Exists(HASHTESTPAIR(1ULL << 34, 3)));
      TEST(set.Exists(HASHTESTPAIR(2, -5)));
      TEST(!set.Exists(HASHTESTPAIR(0, -1)));
      TEST(!set.Exists(HASHTESTPAIR(3, -1)));
    }
  }
  ENDTEST;
}
