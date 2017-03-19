// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "cLocklessQueue.h"
#include <algorithm>
#include "cThread.h"
#include "cHighPrecisionTimer.h"
#include "test.h"

using namespace bss_util;

std::atomic<size_t> lq_c;
uint16_t lq_end[TESTNUM];
std::atomic<uint16_t> lq_pos;

template<class T>
void _locklessqueue_consume(void* p)
{
  while(!startflag.load());
  T* q = (T*)p;
  uint16_t c;
  while((c = lq_pos.fetch_add(1, std::memory_order_relaxed))<TESTNUM)
  {
    while(!q->Pop(lq_end[c]));
  }
}

template<class T>
void _locklessqueue_produce(void* p)
{
  while(!startflag.load());
  T* q = (T*)p;
  size_t c;
  while((c = lq_c.fetch_add(1, std::memory_order_relaxed)) <= TESTNUM)
  {
    q->Push(c);
  }
}
typedef void(*VOIDFN)(void*);

TESTDEF::RETPAIR test_LOCKLESSQUEUE()
{
  BEGINTEST;
  {
    cLocklessQueue<int64_t> q; // Basic sanity test
    q.Push(5);
    int64_t c;
    TEST(q.Pop(c));
    TEST(c == 5);
    TEST(!q.Pop(c));
    TEST(c == 5);
    q.Push(4);
    q.Push(3);
    TEST(q.Pop(c));
    TEST(c == 4);
    q.Push(2);
    q.Push(1);
    TEST(q.Pop(c));
    TEST(c == 3);
    TEST(q.Pop(c));
    TEST(c == 2);
    TEST(q.Pop(c));
    TEST(c == 1);
    TEST(!q.Pop(c));
    TEST(c == 1);
  }

  const int NUMTHREADS = 18;
  cThread threads[NUMTHREADS];

  //typedef cLocklessQueue<uint32_t,true,true,size_t,size_t> LLQUEUE_SCSP; 
  typedef cLocklessQueue<uint16_t, size_t> LLQUEUE_SCSP;
  {
    LLQUEUE_SCSP q; // single consumer single producer test
    uint64_t ppp = cHighPrecisionTimer::OpenProfiler();
    lq_c = 1;
    lq_pos = 0;
    memset(lq_end, 0, sizeof(short)*TESTNUM);
    startflag.store(false);
    threads[0] = cThread((VOIDFN)&_locklessqueue_produce<LLQUEUE_SCSP>, &q);
    threads[1] = cThread((VOIDFN)&_locklessqueue_consume<LLQUEUE_SCSP>, &q);
    startflag.store(true);
    threads[0].join();
    threads[1].join();
    //std::cout << '\n' << cHighPrecisionTimer::CloseProfiler(ppp) << std::endl;
    bool check = true;
    for(int i = 0; i < TESTNUM; ++i)
      check = check && (lq_end[i] == i + 1);
    TEST(check);
  }

  for(int k = 0; k < 1; ++k)
  {
    typedef cMicroLockQueue<uint16_t, size_t> LLQUEUE_MCMP;
    for(int j = 2; j <= NUMTHREADS; j = fbnext(j))
    {
      lq_c = 1;
      lq_pos = 0;
      memset(lq_end, 0, sizeof(short)*TESTNUM);
      LLQUEUE_MCMP q;   // multi consumer multi producer test
      startflag.store(false);
      //threads[0] = std::thread(_locklessqueue_consume<LLQUEUE_MCMP>, &q);
      //for(int i=1; i<j; ++i)
      //  threads[i] = std::thread(_locklessqueue_produce<LLQUEUE_MCMP>, &q);
      for(int i = 0; i<j; ++i)
        threads[i] = cThread((i & 1) ? _locklessqueue_produce<LLQUEUE_MCMP> : _locklessqueue_consume<LLQUEUE_MCMP>, &q);
      startflag.store(true);
      for(int i = 0; i<j; ++i)
        threads[i].join();

      std::sort(std::begin(lq_end), std::end(lq_end));
      bool check = true;
      for(int i = 0; i < TESTNUM - 1; ++i)
      {
        check = check && (lq_end[i] == i + 1);
      }
      TEST(check);

      //std::cout << '\n' << j << " threads: " << q.GetContentions() << std::endl;
    }
  }

  ENDTEST;
}
