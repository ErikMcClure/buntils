// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/DynArray.h"
#include "buntils/stream.h"

using namespace bun;

struct BAREARRAYTEST
{
  void* p;
  size_t c;
  size_t l;
};

static_assert(!std::is_polymorphic<DynArray<int>>::value, "DynArray shouldn't be polymorphic!");
static_assert(sizeof(DynArray<int>) == sizeof(BAREARRAYTEST), "Alloc isn't zero sized!");

TESTDEF::RETPAIR test_DYNARRAY()
{
  BEGINTEST;
  DynArray<int> x(0);
  TEST(!(int*)x);
  x.SetLength(5);
  x[0] = 1;
  x[1] = 2;
  x[2] = 3;
  x[3] = 4;
  x[4] = 5;
  x.Add(6);
  TEST(x[5] == 6);
  x.Add(7);
  TEST(x[5] == 6);
  TEST(x[6] == 7);
  x.Add(8);
  TEST(x[7] == 8);
  x.Remove(5);
  TEST(x[5] == 7);
  TEST(x[6] == 8);
  x.Insert(6, 5);
  TEST(x[5] == 6);
  TEST(x[6] == 7);
  DynArray<int> y(3);
  y.Add(9);
  y.AddConstruct<int>(10);
  y.Add(11);
  auto z = x + y;
  TEST(z.Length() == 11);
  TEST(z[3] == 4);
  TEST(z[8] == 9);
  TEST(z[9] == 10);
  TEST(z[10] == 11);
  z += y;
  TEST(z.Length() == 14);
  TEST(z[3] == 4);
  TEST(z[11] == 9);
  TEST(z[12] == 10);
  TEST(z[13] == 11);

  int zvars[5] = { 4, 3, 2, 1, 5 };
  y.Set(zvars, 3);
  TEST(y.Length() == 3);
  TEST(y[0] == 4);
  TEST(y[1] == 3);
  TEST(y[2] == 2);

  z = std::span<int>{ zvars, 5 };
  TEST(z.Length() == 5);
  TEST(z[3] == 1);
  DynArray<char> n(2);
  DynArray<bool> m(2);
  auto fverify = [&__testret](const DynArray<bool>& a, DynArray<char>& b) {
    TEST(a.Length() == b.Length());
    for(size_t i = 0; i < b.Length(); ++i)
      TEST(a[i] == (b[i] != 0));

    int c = 0;
    for(auto v : a)
      TEST(v == (b[c++] != 0));
    TEST(c == b.Length());

    for(auto v = a.end(); v != a.begin();)
      TEST(*(--v) == (b[--c] != 0));
  };
  auto fadd = [&](bool v) { m.Add(v); n.Add(v); fverify(m, n); };
  auto fremove = [&](size_t i) { m.Remove(i); n.Remove(i); fverify(m, n); };
  auto fset = [&](size_t i, bool v) { m[i] = v; n[i] = v; fverify(m, n); };
  auto finsert = [&](size_t i, bool v) { m.Insert(v, i); n.Insert(v, i); fverify(m, n); };

  auto iter1 = m.begin();
  auto iter2 = m.end();
  TEST(iter1 == iter2);
  fverify(m, n);
  TEST(m.Capacity() == 8);
  fadd(true);
  fset(0, false);
  fset(0, true);
  TEST(m.Capacity() == 8);
  fadd(false);
  fadd(true);
  fadd(false);
  fadd(true);
  fadd(true);
  fset(5, false);
  fset(5, true);
  fset(5, false);
  fadd(true);
  fadd(false);
  TEST(m.Capacity() == 8);
  fadd(true);
  TEST(m.Capacity() == 16);
  fadd(false);
  fadd(true);
  fadd(false);

  DynArray<bool> mm = { true, false, true, false, false, true };
  DynArray<char> nn = { 1, 0, 1, 0, 0, 1 };
  fverify(mm, nn);
  mm = m;
  nn = n;
  fverify(mm, nn);

  DynArray<bool> mmm(m);
  DynArray<char> nnn(n);
  fverify(mmm, nnn);

  for(size_t i = 0; i < m.Length(); ++i)
  {
    fset(i, false);
    fset(i, true);
    fset(i, false);
  }

  for(size_t i = 0; i < 10; ++i)
    finsert(i, (i % 2) != 0);

  fremove(m.Length() - 1);
  fremove(8);
  fremove(7);
  fremove(0);

  while(m.Length() > 0) fremove((m.Length() * 3) % m.Length());

  TEST(!m.Length());

  auto f = [](DynArray<DEBUG_CDT<true>, uint32_t>& arr)->bool {
    for(size_t i = 0; i < arr.Length(); ++i)
      if(arr[i]._index != i)
        return false;
    return true;
  };
  auto f2 = [](DynArray<DEBUG_CDT<true>, uint32_t>& arr, uint32_t s) { for(uint32_t i = s; i < arr.Length(); ++i) { arr[i]._index = i; } };

  assert(!DEBUG_CDT_SAFE::Tracker.Length());
  {
    DEBUG_CDT<true>::count = 0;
    DynArray<DEBUG_CDT<true>, uint32_t> b(10);
    b.SetLength(10);
    f2(b, 0);
    b.Remove(5);
    for(size_t i = 0; i < 5; ++i) TEST(b[i]._index == i);
    for(size_t i = 5; i < b.Length(); ++i) TEST(b[i]._index == (i + 1));
    TEST(b.Length() == 9);
    TEST(DEBUG_CDT<true>::count == 9);
    f2(b, 0);
    b.SetLength(19);
    f2(b, 9);
    TEST(f(b));
    TEST(DEBUG_CDT<true>::count == 19);
    TEST(b.Length() == 19);
    DynArray<DEBUG_CDT<true>, uint32_t> c(b);
    TEST(f(c));
    TEST(DEBUG_CDT<true>::count == 38);
    b += c;
    for(size_t i = 0; i < 19; ++i) TEST(b[i]._index == i);
    for(size_t i = 19; i < 38; ++i) TEST(b[i]._index == (i - 19));
    TEST(DEBUG_CDT<true>::count == 57);
    b + c;
    f2(b, 0);
    b.Insert(DEBUG_CDT<true>(), 5);
    for(size_t i = 0; i < 5; ++i) TEST(b[i]._index == i);
    for(size_t i = 6; i < b.Length(); ++i) TEST(b[i]._index == (i - 1));
    TEST(DEBUG_CDT<true>::count == 58);
    b.Insert(DEBUG_CDT<true>(), b.Length());
    TEST(DEBUG_CDT<true>::count == 59);
  }
  TEST(!DEBUG_CDT<true>::count);
  TEST(!DEBUG_CDT_SAFE::Tracker.Length());

  auto f3 = [](DynArray<DEBUG_CDT<false>, uint32_t>& arr)->bool {
    for(size_t i = 0; i < arr.Length(); ++i)
      if(arr[i]._index != i)
        return false;
    return true;
  };
  auto f4 = [](DynArray<DEBUG_CDT<false>, uint32_t>& arr, uint32_t s) { for(uint32_t i = s; i < arr.Length(); ++i) { arr[i]._index = i; } };
  {
    DEBUG_CDT<false>::count = 0;
    DynArray<DEBUG_CDT<false>, uint32_t> b(10);
    b.SetLength(10);
    f4(b, 0);
    b.Remove(5);
    for(size_t i = 0; i < 5; ++i) TEST(b[i]._index == i);
    for(size_t i = 5; i < b.Length(); ++i) TEST(b[i]._index == (i + 1));
    TEST(b.Length() == 9);
    TEST(DEBUG_CDT<false>::count == 9);
    f4(b, 0);
    b.SetLength(19);
    f4(b, 9);
    TEST(f3(b));
    TEST(DEBUG_CDT<false>::count == 19);
    TEST(b.Length() == 19);
    DynArray<DEBUG_CDT<false>, uint32_t> c(b);
    TEST(f3(c));
    TEST(DEBUG_CDT<false>::count == 38);
    b += c;
    for(size_t i = 0; i < 19; ++i) TEST(b[i]._index == i);
    for(size_t i = 19; i < 38; ++i) TEST(b[i]._index == (i - 19));
    TEST(DEBUG_CDT<false>::count == 57);
    b + c;
    f4(b, 0);
    b.Insert(DEBUG_CDT<false>(), 5);
    for(size_t i = 0; i < 5; ++i) TEST(b[i]._index == i);
    for(size_t i = 6; i < b.Length(); ++i) TEST(b[i]._index == (i - 1));
    TEST(DEBUG_CDT<false>::count == 58);
    b.Insert(DEBUG_CDT<false>(), b.Length());
    TEST(DEBUG_CDT<false>::count == 59);
  }
  TEST(!DEBUG_CDT<false>::count);
  TEST(!DEBUG_CDT_SAFE::Tracker.Length());

  ArbitraryArray<int> u(0);
  int ua[5] = { 1,2,3,4,5 };
  u.SetElement(ua);
  u.Get<int>(0) = 1;
  u.Get<int>(1) = 2;
  u.Get<int>(2) = 3;
  u.Get<int>(3) = 4;
  u.Get<int>(4) = 5;
  u.Add(6);
  TEST(u.Get<int>(5) == 6);
  u.Add(7);
  TEST(u.Get<int>(5) == 6);
  TEST(u.Get<int>(6) == 7);
  u.Add(8);
  TEST(u.Get<int>(7) == 8);
  u.Remove(5);
  TEST(u.Get<int>(5) == 7);
  TEST(u.Get<int>(6) == 8);
  u.SetElement(7);
  TEST(u.Get<int>(5) == 7);
  TEST(u.Get<int>(6) == 8);

  //cBitArray<uint8_t> bits;
  DynArray<bool> bits;
  bits.Clear();
  bits.SetLength(7);
  bits[5] = true;
  TEST(bits.GetRawByte(0) == 32);
  TEST(bits.CountBits(0, 6) == 1);
  TEST(bits.CountBits(0, 5) == 0);
  bits[5] = true; //bits+=5;
  TEST(bits.CountBits(0, 6) == 1);
  TEST(bits[5]);
  bits[5] = false;
  TEST(bits.GetRawByte(0) == 0);
  TEST(bits.CountBits(0, 6) == 0);
  bits[2] = true;
  TEST(bits.GetRawByte(0) == 4);
  TEST(bits[2]);
  bits[0] = true; //bits+=0;
  TEST(bits.GetRawByte(0) == 5);
  TEST(bits[0]);
  bits[3] = true;
  TEST(bits.GetRawByte(0) == 13);
  TEST(bits.CountBits(0, 6) == 3);
  bits[2] = false; //bits-=2;
  TEST(bits.GetRawByte(0) == 9);
  TEST(bits.CountBits(0, 6) == 2);
  bits.SetLength(31);
  TEST(bits.GetRawByte(0) == 9);
  bits[20] = true;
  TEST(bits.GetRawByte(0) == 9);
  TEST(bits.GetRawByte(2) == 16);
  bits[30] = true;
  TEST(bits[30]);
  TEST(bits.GetRawByte(0) == 9);
  TEST(bits.GetRawByte(2) == 16);
  TEST(bits.GetRawByte(3) == 64);
  bits.SetLength(32);
  TEST(bits.CountBits(0, 32) == 4);
  bits[21] = false;
  TEST(bits.CountBits(0, 32) == 4);
  bits[20] = false;
  TEST(bits.GetRawByte(0) == 9);
  TEST(bits.GetRawByte(2) == 0);
  TEST(bits.GetRawByte(3) == 64);
  TEST(bits.CountBits(0, 32) == 3);

  ENDTEST;
}
