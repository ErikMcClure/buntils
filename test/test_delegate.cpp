// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/Delegate.h"

using namespace bun;

struct foobar
{
  void nyan(uint32_t cat) { TEST(cat == 5); }
  void nyannyan(int cat, int kitty)
  {
    TEST(cat == 2);
    TEST(kitty == -3);
  }
  void nyannyannyan(int cat, int kitty, bool fluffy)
  {
    TEST(cat == -6);
    TEST(kitty == 0);
    TEST(fluffy);
  }
  void zoidberg()
  {
    TEST(true);
    ++count;
  }
  void nothing() {}
  void nothing2() {}
  void nothing3() {}

  TESTDEF::RETPAIR& __testret;
  int count;
};

void external_zoidberg(foobar* obj) { obj->zoidberg(); }

TESTDEF::RETPAIR test_DELEGATE()
{
  BEGINTEST;
  foobar foo  = { __testret, 0 };
  auto first  = Delegate<void>::From<foobar, &foobar::zoidberg>(&foo);
  auto second = Delegate<void, uint32_t>::From<foobar, &foobar::nyan>(&foo);
  auto three  = Delegate<void, int, int>::From<foobar, &foobar::nyannyan>(&foo);
  auto four   = Delegate<void, int, int, bool>::From<foobar, &foobar::nyannyannyan>(&foo);
  auto five   = Delegate<void>::FromC<foobar, &external_zoidberg>(&foo);

  Delegate<void> copy(first);
  copy = first;
  CPU_Barrier();
  copy();
  CPU_Barrier();
  Delegate<void, uint32_t> copy2(second);
  CPU_Barrier();
  copy2(5);
  CPU_Barrier();
  three(2, -3);
  four(-6, 0, true);

  bool fcalled            = false;
  std::function<void()> f = [&]() { fcalled = true; };
  Delegate<void> d        = f;
  // Delegate<void> d(std::function<void()>([](){})); // This should throw a compiler error
  d();
  TEST(fcalled);
  five();
  TEST(foo.count == 2);

  {
    defer(&external_zoidberg, &foo);
    defer(four, -6, 0, true);
  }
  TEST(foo.count == 3);
  ENDTEST;
}
