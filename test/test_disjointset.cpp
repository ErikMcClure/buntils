// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/DisjointSet.h"
#include "buntils/algo.h"

using namespace bun;

TESTDEF::RETPAIR test_DISJOINTSET()
{
  BEGINTEST;

  std::pair<uint32_t, uint32_t> E[10]; // Initialize a complete tree with an arbitrary order.
  bun_Fill(E, 0);
  E[0].first = 1;
  E[1].first = 2;
  E[2].first = 3;
  E[3].first = 4;
  E[4].first = 2;
  E[5].first = 3;
  E[6].first = 4;
  E[7].first = 3;
  E[8].first = 4;
  E[9].first = 4;
  E[4].second = 1;
  E[5].second = 1;
  E[6].second = 1;
  E[7].second = 2;
  E[8].second = 2;
  E[9].second = 3;
  Shuffle(E); // Shuffle our edges
  auto tree = DisjointSet<uint32_t>::MinSpanningTree(5, E);
  TEST(tree.Capacity() == 4);

  DisjointSet<uint32_t> s(5);
  s.Union(2, 3);
  s.Union(2, 4);
  s.Union(1, 2);
  TEST((s.NumElements(3) == 4));
  VARARRAY(uint32_t, elements, s.NumElements(3));
  std::vector<uint32_t> abc;

  s.GetElements(3, abc);
  TEST((s.GetElements(3, elements) == 4));
  TEST(elements[0] == 1);
  TEST(elements[1] == 2);
  TEST(elements[2] == 3);
  TEST(elements[3] == 4);
  ENDTEST;
}
