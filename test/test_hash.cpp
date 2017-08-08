// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/Hash.h"
#include "test.h"

using namespace bss;

static_assert(std::is_member_pointer<void(TESTDEF::*)()>::value, "member failure");

TESTDEF::RETPAIR test_HASH()
{
  BEGINTEST;
  Hash<int, void(TESTDEF::*)()> membertest;
  void(TESTDEF::*a)() = membertest[0];

  //cKhash<int, char,false,KH_INT_HASHFUNC,KH_INT_EQUALFUNC<int>,KH_INT_VALIDATEPTR<int>> hashtest;
  //hashtest.Insert(21354,0);
  //hashtest.Insert(34623,0);
  //hashtest.Insert(52,0);
  //hashtest.Insert(1,0);
  //int r=hashtest.GetIterKey(hashtest.GetIterator(1));
  {
    Hash<int, Logger*> hasherint;
    hasherint.Insert(25, &_failedtests);
    hasherint.Get(25);
    hasherint.Remove(25);
    Hash<const char*, Logger*, true> hasher;
    TEST(!hasher[""]);
    hasher.Insert("", &_failedtests);
    hasher.Insert("Video", (Logger*)5);
    hasher.SetCapacity(100);
    hasher.Insert("Physics", 0);
    Logger* check = hasher.Get("Video");
    TEST(check == (Logger*)5);
    check = hasher.Get("Video");
    TEST(check == (Logger*)5);
    Hash<const char*, Logger*, true> hasher2(hasher);
    check = hasher2.Get("Video");
    TEST(check == (Logger*)5);
    Hash<const char*, Logger*, true> hasher3(std::move(hasher));
    check = hasher2.Get("Video");
    TEST(check == (Logger*)5);
    //uint64_t diff = HighPrecisionTimer::CloseProfiler(ID);

    {
      Hash<const void*> set;
      TEST(!set.Exists(0));
      set.Insert(0);
      set.Insert(&check);
      set.Insert(&hasher);
      set.Insert(&hasherint);
      set.Insert(&set);
      set.GetKey(0);
      TEST(set.Exists(0));
    }
    {
      Hash<int, int> val;
      TEST(val[0] == -1);
      val.Insert(0, 1);
      TEST(val[0] == 1);
      TEST(val[-3] == -1);
      val.Insert(-3, 2);
      TEST(val[-3] == 2);
      TEST(val[4] == -1);
      val.Insert(4, -2);
      TEST(val[4] == -2);
      val.MutableValue(val.Iterator(4)) -= 1;
      TEST(val[4] == -3);
      val.Remove(-3);
      TEST(val[-3] == -1);
    }
    {
      typedef std::pair<uint64_t, int32_t> HASHTESTPAIR;
      Hash<HASHTESTPAIR> set;
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
    {
      Hash<int, DEBUG_CDT<true>, false, ARRAY_SAFE> safe;
      TEST(!safe[2]);
      safe.Insert(0, DEBUG_CDT<true>());
      safe.Insert(2, DEBUG_CDT<true>());
      safe.Insert(-3, DEBUG_CDT<true>());
      safe.Insert(1234, DEBUG_CDT<true>());
      safe.Insert(2874984, DEBUG_CDT<true>());
      safe.Insert(-28383, DEBUG_CDT<true>());
      safe.Insert(8, DEBUG_CDT<true>());
      TEST(safe[2] != 0);
      TEST(safe(0));
      TEST(safe(2));
      TEST(safe(-3));
      TEST(safe(1234));
      TEST(safe(2874984));
      TEST(safe(-28383));
      TEST(safe(8));
      safe.Insert(9, DEBUG_CDT<true>());
      safe.Insert(10, DEBUG_CDT<true>());
      TEST(safe(0));
      TEST(safe(2));
      TEST(safe(10));
      TEST(safe(9));
      TEST(safe(2874984));
      TEST(safe(-28383));
      TEST(safe(8));
      safe.Remove(8);
      TEST(!safe(8));
      TEST(safe(2));
      TEST(safe(10));
      TEST(safe(9));

      auto safe2 = safe;
      safe2.Insert(4, DEBUG_CDT<true>());
      TEST(!safe2(8));
      TEST(safe2(2));
      TEST(safe2(10));
      TEST(safe2(9));
      TEST(safe2(4));
      safe.Remove(10);
      TEST(!safe(8));
      TEST(safe(2));
      TEST(!safe(10));
      TEST(safe(9));

      auto safe3(std::move(safe));
      TEST(!safe(2));
      TEST(!safe3(8));
      TEST(safe3(2));
      TEST(!safe3(10));
      TEST(safe3(9));
      TEST(safe3(-3));
      safe3.Remove(-3);
      TEST(!safe3(8));
      TEST(safe3(2));
      TEST(!safe3(10));
      TEST(safe3(9));
      TEST(!safe3(-3));
    }
    int count = DEBUG_CDT<true>::count;
    TEST(DEBUG_CDT<true>::count == 0);
  }
  ENDTEST;
}
