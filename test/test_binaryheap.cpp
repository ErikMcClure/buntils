// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/BinaryHeap.h"
#include <algorithm>

using namespace bun;

TESTDEF::RETPAIR test_BINARYHEAP()
{
  BEGINTEST;
  int a[]            = { 7, 33, 55, 7, 45, 1, 43, 4, 3243, 25, 3, 6, 9, 14, 5, 16, 17, 22, 90, 95, 99, 32 };
  int a3[]           = { 7, 33, 55, 7, 45, 1, 43, 4, 3243, 25, 3, 6, 9, 14, 5, 16, 17, 22, 90, 95, 99, 32 };
  int fill[]         = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  int a2[]           = { 7, 33, 55, 7, 45, 1, 43, 4, 3243, 25, 3, 6, 9, 14, 5, 16, 17, 22, 90, 95, 99, 32 };
  const size_t a2_SZ = std::ranges::size(a2);

  auto arrtest = [&__testret](int* u, int* v, size_t count) { TESTCOUNTALL(count, u[i] == v[i]); };

  std::sort(std::begin(a2), std::end(a2));
  BinaryHeap<int>::HeapSort(a3, std::compare_three_way{});
  arrtest(a2, a3, a2_SZ);

  std::sort(std::begin(a2), std::end(a2), [](int x, int y) -> bool { return x > y; });
  BinaryHeap<int, inv_three_way, uint32_t>::HeapSort(a3, inv_three_way{});
  arrtest(a2, a3, a2_SZ);

  std::vector<int> b;
  BinaryHeap<int> c;
  for(size_t i = 0; i < a2_SZ; ++i)
  {
    b.push_back(a[i]);
    std::push_heap(b.begin(), b.end());
    c.Insert(a[i]);
    arrtest(&b[0], c.data(), c.size());
  }

  std::for_each(b.begin(), b.end(), [](int& x) { x += 1; });
  std::for_each(c.begin(), c.end(), [](int& x) { x += 1; });
  arrtest(&b[0], c.data(), c.size());

  for(size_t i = 0; i < a2_SZ; ++i)
  {
    std::pop_heap(b.begin(), b.end() - i);
    c.Remove(0);

    // for(uint32_t j = 0; j < c.size(); ++j)
    //   fill[j]=c[j]; //Let's us visualize C's array
    // for(uint32_t j = 0; j < c.size(); ++j)
    //   assert(c[j]==b[j]);
    arrtest(&b[0], c.data(), c.size());
  }
  ENDTEST;
}
