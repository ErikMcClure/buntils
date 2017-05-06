// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/SmartPtr.h"
#include "test.h"

using namespace bss;

struct fooref : public DEBUG_CDT<false>, RefCounter {};

TESTDEF::RETPAIR test_SMARTPTR()
{
  BEGINTEST;

  OwnerPtr<char> p(new char[56]);
  char* pp = p;
  OwnerPtr<char> p2(p);
  OwnerPtr<char> p3(std::move(p));
  TEST(pp == p3);

  ENDTEST;
}