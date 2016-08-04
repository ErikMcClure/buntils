// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "cTRBtree.h"
#include "bss_algo.h"
#include "bss_alloc_block.h"
#include <fstream>

using namespace bss_util;

bool verifytree(TRB_Node<int>* front, uint32_t& same)
{
  same = 0;
  uint32_t pass = 0;
  int last = -1;
  for(TRB_Node<int>* pnode = front; pnode != 0; pnode = pnode->next)
  {
    if(pnode->value == last)
      same += 1;
    if(pnode->value<last)
      pass += 1;
    last = pnode->value;
  }
  return !pass;
}
void serialize_testnum(const char* file)
{
  std::fstream f;
  f.open(file, std::ios::trunc | std::ios::out);
  for(int i = 0; i < TESTNUM; ++i)
    f << testnums[i] << std::endl;
  f.close();
}
void deserialize_testnum(const char* file)
{
  std::fstream f;
  f.open(file, std::ios::in);
  for(int i = 0; i < TESTNUM && !f.eof(); ++i)
  {
    f >> testnums[i];
    f.get();
  }
  f.close();
}

bool verify_unique_testnums()
{
  cHash<int, char, false> hash;

  for(int i = 0; i<TESTNUM; ++i)
  {
    if(hash.Exists(testnums[i]))
      return false;
    hash.Insert(testnums[i], 1);
  }
  return true;
}

TESTDEF::RETPAIR test_TRBTREE()
{
  BEGINTEST;
  BlockPolicy<TRB_Node<int>> fixedalloc;
  cTRBtree<int, CompT<int>, BlockPolicy<TRB_Node<int>>> blah(&fixedalloc);

  uint32_t same = 0;
  shuffle(testnums);

  blah.Clear();
  for(int i = 0; i<TESTNUM; ++i)
    blah.Insert(testnums[i]);

  assert(verifytree(blah.Front(), same));
  TEST(!blah.Get(-1))
    TEST(!blah.Get(TESTNUM + 1))

    shuffle(testnums);
  int num = 0;
  for(int i = 0; i<TESTNUM; ++i)
  {
    auto p = blah.Get(testnums[i]);
    if(p != 0)
      num += (p->value == testnums[i]);
  }
  TEST(num == TESTNUM);

  blah.Remove(4);
  TEST(blah.GetNear(4)->value == 3);
  TEST(blah.GetNear(4, false)->value == 5);
  blah.Insert(4);

  shuffle(testnums);
  for(int i = 0; i<TESTNUM; ++i)
    blah.Insert(testnums[i]);
  shuffle(testnums);
  for(int i = 0; i<TESTNUM; ++i)
    blah.Insert(testnums[i]);

  TEST(verifytree(blah.Front(), same));
  TEST(same == (TESTNUM * 2));

  shuffle(testnums);
  num = 0;
  int n2 = 0;
  int n3 = 0;
  for(int i = 0; i<TESTNUM; ++i)
  {
    num += (blah.Get(testnums[i]) != 0);
    n2 += blah.Remove(testnums[i]);
    n2 += blah.Remove(testnums[i]);
    n2 += blah.Remove(testnums[i]);
    n3 += (!blah.Get(testnums[i]));
  }
  TEST(num == TESTNUM);
  TEST(n2 == TESTNUM * 3);
  TEST(n3 == TESTNUM);

  //std::cout << cHighPrecisionTimer::CloseProfiler(prof) << std::endl;
  ENDTEST;
}
