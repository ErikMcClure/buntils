// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss_stream.h"
#include "test.h"

using namespace bss_util;

TESTDEF::RETPAIR test_STREAMSPLITTER()
{
  BEGINTEST;
  std::stringstream ss1;
  std::stringstream ss2;
  std::stringstream ss3;
  ss1 << "1 ";
  ss2 << "2 ";
  ss3 << "3 ";

  StreamSplitter splitter;
  std::ostream split(&splitter);
  splitter.AddTarget(&ss1);
  split << "a ";
  splitter.AddTarget(&ss2);
  split << "b ";
  splitter.AddTarget(&ss3);
  split << "c " << std::flush;
  splitter.ClearTargets();
  split << "d " << std::flush;
  splitter.AddTarget(&ss1);
  split << "e " << std::flush;

  TEST(ss1.str() == "1 a b c e ");
  TEST(ss2.str() == "2 b c ");
  TEST(ss3.str() == "3 c ");
  ENDTEST;
}//*/