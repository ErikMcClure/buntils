// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "cSmartPtr.h"

using namespace bss_util;

struct fooref : public DEBUG_CDT<false>, cRefCounter {};

TESTDEF::RETPAIR test_SMARTPTR()
{
  BEGINTEST;

  cOwnerPtr<char> p(new char[56]);
  char* pp = p;
  cOwnerPtr<char> p2(p);
  cOwnerPtr<char> p3(std::move(p));
  TEST(pp == p3);

  ENDTEST;
}