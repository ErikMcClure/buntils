// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "cThreadPool.h"
#include <algorithm>
#include "test.h"

using namespace bss_util;

std::atomic<size_t> pq_c;
uint16_t pq_end[TESTNUM];
std::atomic<uint16_t> pq_pos;

void pooltest(int i) {
  while(!startflag.load(std::memory_order_relaxed));
  pq_end[pq_c.fetch_add(1, std::memory_order_relaxed)] = i;
}
TESTDEF::RETPAIR test_THREADPOOL()
{
  BEGINTEST;
  static const int NUM = 8;
  cThreadPool pool(NUM);
#ifdef BSS_VARIADIC_TEMPLATES
  memset(pq_end, 0, sizeof(uint16_t)*TESTNUM);
  pq_c = 0;
  startflag.store(false, std::memory_order_relaxed);

  for(int i = 0; i < TESTNUM; ++i)
    pool.AddFunc(pooltest, i);

  startflag.store(true, std::memory_order_relaxed);
  pool.Wait();

  TEST(pq_c == TESTNUM);

  std::sort(std::begin(pq_end), std::end(pq_end));
  bool check = true;
  for(int i = 0; i < TESTNUM; ++i)
    check = (pq_end[i] == i) && check;
  TEST(check);
#endif

  ENDTEST;
}
