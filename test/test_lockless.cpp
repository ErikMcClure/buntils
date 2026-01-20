// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/lockless.h"

using namespace bun;

TESTDEF::RETPAIR test_LOCKLESS()
{
  BEGINTEST;
  CPU_Barrier();


  ENDTEST;
}