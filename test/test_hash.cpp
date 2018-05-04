// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "bss-util/Hash.h"

using namespace bss;

static_assert(std::is_member_pointer<void(TESTDEF::*)()>::value, "member failure");

TESTDEF::RETPAIR test_HASH()
{
  BEGINTEST;
  Hash<int, void(TESTDEF::*)()> membertest;
  void(TESTDEF::*a)() = membertest[0];

  {
    Hash<int, Logger*> hasherint;
    hasherint.Insert(25, &_failedtests);
    hasherint.Get(25);
    hasherint.Remove(25);
    HashIns<const char*, Logger*> hasher;
    TEST(!hasher[""]);
    hasher.Insert("", &_failedtests);
    hasher.Insert("Video", (Logger*)5);
    hasher.SetCapacity(100);
    hasher.Insert("Physics", 0);

    Logger* check = hasher.Get("Video");
    TEST(check == (Logger*)5);
    check = hasher.Get("Video");
    TEST(check == (Logger*)5);
    HashIns<const char*, Logger*> hasher2(hasher);
    check = hasher2.Get("Video");
    TEST(check == (Logger*)5);
    HashIns<const char*, Logger*> hasher3(std::move(hasher));
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
      size_t count = 0;
      for([[maybe_unused]] auto k : set)
        count++;
      TEST(set.Length() == 5);
      TEST(set.Length() == count);
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
      val.Value(val.Iterator(4)) -= 1;
      TEST(val[4] == -3);
      val.Remove(-3);
      TEST(val[-3] == -1);
      size_t count = 0;
      for([[maybe_unused]] auto [k, v] : val)
        count++;
      TEST(val.Length() == count);
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
      Hash<int, DEBUG_CDT<true>, ARRAY_SAFE> safe;
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

  {
  Hash<int, std::unique_ptr<int>, ARRAY_MOVE> safe;
  TEST(!safe[2]);
  safe.Insert(0, nullptr);
  safe.Insert(2, nullptr);
  safe.Insert(-3, nullptr);
  safe.Insert(1234, nullptr);
  safe.Insert(2874984, nullptr);
  safe.Insert(-28383, nullptr);
  safe.Insert(8, nullptr);
  TEST(safe.Exists(2));
  TEST(safe(0));
  TEST(safe(2));
  TEST(safe(-3));
  TEST(safe(1234));
  TEST(safe(2874984));
  TEST(safe(-28383));
  TEST(safe(8));
  safe.Insert(9, nullptr);
  safe.Insert(10, nullptr);
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

  safe.Remove(10);
  TEST(!safe(8));
  TEST(safe(2));
  TEST(!safe(10));
  TEST(safe(9));

  Hash<int, std::unique_ptr<int>, ARRAY_MOVE> safe3(std::move(safe));
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
  safe = std::move(safe3);
  TEST(safe(2));
  TEST(safe(9));
  }
  {
    Hash<int, Str> h;
    h.Insert(0, "test");
    h.Insert(1, "test2");
    TEST(!strcmp(h[0], "test"));
    TEST(!strcmp(h[1], "test2"));
    TEST(h[2] == 0);
    static_assert(std::is_same<decltype(h[0]), const char*>::value, "wrong GET type");
  }
  {
    Hash<int, std::string> h;
    h.Insert(0, "test");
    h.Insert(1, "test2");
    TEST(!strcmp(h[0], "test"));
    TEST(!strcmp(h[1], "test2"));
    TEST(h[2] == 0);
    static_assert(std::is_same<decltype(h[0]), const char*>::value, "wrong GET type");
  }
  {
    Hash<int, void*> h;
    h.Insert(0, 0);
    h.Insert(1, &h);
    TEST(h[0] == 0);
    TEST(h[1] == &h);
    TEST(h[2] == 0);
    static_assert(std::is_same<decltype(h[0]), void*>::value, "wrong GET type");
  }
  {
    Hash<int, std::unique_ptr<int>, ARRAY_MOVE> h;
    h.Insert(0, std::unique_ptr<int>(new int(1)));
    h.Insert(1, std::unique_ptr<int>(new int(2)));
    TEST(*h[0] == 1);
    TEST(*h[1] == 2);
    TEST(h[2] == 0);
    static_assert(std::is_same<decltype(h[0]), int*>::value, "wrong GET type");
  }
  ENDTEST;
}
