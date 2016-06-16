// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "lockless.h"

using namespace bss_util;

TESTDEF::RETPAIR test_LOCKLESS()
{
  BEGINTEST;
  CPU_Barrier();

  {//Sanity checks for atomic_inc
    int a = 1;
    atomic_xadd(&a);
    TEST(a == 2);
    CPU_Barrier();
    int* b = &a;
    atomic_xadd(b);
    CPU_Barrier();
    TEST(a == 3);
    volatile int* c = &a;
    atomic_xadd<int>(c);
    atomic_xadd<int>(c = b);
    CPU_Barrier();
    TEST(a == 5);
  }
  {//Sanity checks for atomic_xchg
    int a = 1;
    int b = 2;
    b = atomic_xchg<int>(&a, b);
    TEST(a == 2);
    TEST(b == 1);
    atomic_xchg<int>(&b, a);
    TEST(a == 2);
    TEST(b == 2);
    int* c = &a;
    atomic_xchg<int>(c, 3);
    TEST(a == 3);
    volatile int* d = &b;
    a = atomic_xchg<int>(d, 5);
    TEST(a == 2);
    TEST(b == 5);
  }
  ENDTEST;
}