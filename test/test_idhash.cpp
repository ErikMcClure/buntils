// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "bss-util/IDHash.h"

using namespace bss;

TESTDEF::RETPAIR test_IDHASH()
{
  BEGINTEST;

  {
    IDHash<ptrdiff_t> hash(3);
    TEST(hash.Length() == 0);
    TEST(hash.MaxID() == 0);
    ptrdiff_t a = hash.Add(5);
    ptrdiff_t b = hash.Add(6);
    ptrdiff_t c = hash.Add(7);
    ptrdiff_t d = hash.Add(8);
    TEST(hash.Length() == 4);
    TEST(hash.MaxID() == 3);
    TEST(a == 0);
    TEST(b == 1);
    TEST(c == 2);
    TEST(d == 3);
    TEST(hash[a] == 5);
    TEST(hash[b] == 6);
    TEST(hash[c] == 7);
    TEST(hash.Get(d) == 8);
    hash.Remove(b);
    TEST(hash.Length() == 3);
    b = hash.Add(9);
    TEST(hash.Length() == 4);
    TEST(hash.MaxID() == 3);
    TEST(b == 1);
    TEST(hash[b] == 9);
    IDHash<ptrdiff_t> hash2(hash);
    TEST(hash2.Length() == 4);
    TEST(hash2.MaxID() == 3);
    TEST(hash2[b] == 9);
    IDHash<ptrdiff_t> hash3(std::move(hash));
    TEST(hash3.Length() == 4);
    TEST(hash3.MaxID() == 3);
    TEST(hash3[b] == 9);
  }

  {
    IDReverse<int, uint32_t, StaticAllocPolicy<int>, -1> hash;
    uint32_t a = hash.Add(1);
    uint32_t b = hash.Add(2);
    uint32_t c = hash.Add(3);
    uint32_t d = hash.Add(4);
    uint32_t e = hash.Add(5);
    uint32_t f = hash.Add(6);
    uint32_t g = hash.Add(7);
    uint32_t h = hash.Add(8);
    TEST(hash.Length() == 8);
    TEST(hash.MaxID() == 7);
    for(size_t i = 0; i<8; ++i) TEST(hash[i] == (i + 1));
    for(size_t i = 0; i<8; ++i) TEST(hash.Lookup(i + 1) == i);
    hash.Remove(b);
    hash.Remove(d);
    hash.Remove(e);
    TEST(hash.Length() == 5);
    TEST(hash.MaxID() == 7);
    hash.Compress();
    TEST(hash.Length() == 5);
    TEST(hash.MaxID() == 4);
    TEST(hash[0] == 1);
    TEST(hash[1] == 8);
    TEST(hash[2] == 3);
    TEST(hash[3] == 7);
    TEST(hash[4] == 6);
    TEST(hash.Lookup(1) == 0);
    TEST(hash.Lookup(2) == (uint32_t)-1);
    TEST(hash.Lookup(3) == 2);
    TEST(hash.Lookup(4) == (uint32_t)-1);
    TEST(hash.Lookup(5) == (uint32_t)-1);
    TEST(hash.Lookup(6) == 4);
    TEST(hash.Lookup(7) == 3);
    TEST(hash.Lookup(8) == 1);
  }
  ENDTEST;
}
