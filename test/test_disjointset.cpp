// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "cDisjointSet.h"
#include "bss_algo.h"

using namespace bss_util;

TESTDEF::RETPAIR test_DISJOINTSET()
{
  BEGINTEST;

  std::pair<uint32_t, uint32_t> E[10]; // Initialize a complete tree with an arbitrary order.
  memset(E, 0, sizeof(std::pair<uint32_t, uint32_t>) * 10);
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
  shuffle(E); // shuffle our edges
  auto tree = cDisjointSet<uint32_t>::MinSpanningTree(5, std::begin(E), std::end(E));
  TEST(tree.Capacity() == 4);

  cDisjointSet<uint32_t> s(5);
  s.Union(2, 3);
  s.Union(2, 4);
  s.Union(1, 2);
  TEST((s.NumElements(3) == 4));
  DYNARRAY(uint32_t, elements, s.NumElements(3));
  TEST((s.GetElements(3, elements) == 4));
  TEST(elements[0] == 1);
  TEST(elements[1] == 2);
  TEST(elements[2] == 3);
  TEST(elements[3] == 4);
  ENDTEST;
}
