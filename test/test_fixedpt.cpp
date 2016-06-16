// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "FixedPt.h"

using namespace bss_util;

TESTDEF::RETPAIR test_bss_FIXEDPT()
{
  BEGINTEST;

  FixedPt<13> fp(23563.2739);
  float res = fp;
  fp += 27.9;
  res += 27.9f;
  TEST(fcompare(res, fp));
  res = fp;
  fp -= 8327.9398437;
  res -= 8327.9398437f;
  TEST(fcompare(res, fp));
  res = fp;
  fp *= 6.847399;
  res *= 6.847399f;
  TEST(fcompare(res, fp, 215)); // We start approaching the edge of our fixed point range here so things predictably get out of whack
  res = fp;
  fp /= 748.9272;
  res /= 748.9272f;
  TEST(fcompare(res, fp, 6));
  ENDTEST;
}