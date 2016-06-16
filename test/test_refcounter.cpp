// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "cRefCounter.h"

using namespace bss_util;

struct REF_TEST : cRefCounter
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
  ENDTEST;
}