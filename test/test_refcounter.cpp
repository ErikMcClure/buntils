// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "cRefCounter.h"
#include "test.h"

using namespace bss_util;

struct REF_TEST : DEBUG_CDT<false>, cRefCounter
{
  REF_TEST(TESTDEF::RETPAIR& t) : __testret(t) {}
  ~REF_TEST() { TEST(true); }
  virtual void DestroyThis() { TEST(true); cRefCounter::DestroyThis(); }

  TESTDEF::RETPAIR& __testret;
};

TESTDEF::RETPAIR test_REFCOUNTER()
{
  BEGINTEST;
  RCounter a;
  TEST(a.Grab() == 1);
  RCounter b(a);
  TEST(b.Grab() == 1);
  REF_TEST* c = new REF_TEST(__testret);
  c->Grab();
  c->Drop();

  {
    DEBUG_CDT<false>::count = 0;
    ref_ptr<REF_TEST> p4(new REF_TEST(__testret));
    ref_ptr<REF_TEST> p5(p4);
    ref_ptr<REF_TEST> p6(std::move(p5));
    p5 = p6;
    p6 = std::move(p4);
  }
  TEST(!DEBUG_CDT<false>::count);
  ENDTEST;
}