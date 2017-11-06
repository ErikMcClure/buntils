// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "bss-util/BinaryHeap.h"
#include <algorithm>

using namespace bss;

TESTDEF::RETPAIR test_BINARYHEAP()
{
  BEGINTEST;
  int a[] = { 7,33,55,7,45,1,43,4,3243,25,3,6,9,14,5,16,17,22,90,95,99,32 };
  int a3[] = { 7,33,55,7,45,1,43,4,3243,25,3,6,9,14,5,16,17,22,90,95,99,32 };
  int fill[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
  int a2[] = { 7,33,55,7,45,1,43,4,3243,25,3,6,9,14,5,16,17,22,90,95,99,32 };
  const int a2_SZ = sizeof(a2) / sizeof(int);

  auto arrtest = [&](int* a, int* b, size_t count) {
    TESTCOUNTALL(count, a[i] == b[i]);
  };

  std::sort(std::begin(a2), std::end(a2));
  BinaryHeap<int>::HeapSort(a3);
  arrtest(a2, a3, a2_SZ);

  std::sort(std::begin(a2), std::end(a2), [](int x, int y)->bool { return x>y; });
  BinaryHeap<int, uint32_t, CompTInv<int>>::HeapSort(a3);
  arrtest(a2, a3, a2_SZ);

  std::vector<int> b;
  BinaryHeap<int> c;
  for(size_t i = 0; i < a2_SZ; ++i)
  {
    b.push_back(a[i]);
    std::push_heap(b.begin(), b.end());
    c.Insert(a[i]);
    arrtest(&b[0], c, c.Length());
  }

  std::for_each(b.begin(), b.end(), [](int& a) { a += 1; });
  std::for_each(c.begin(), c.end(), [](int& a) { a += 1; });
  arrtest(&b[0], c, c.Length());

  for(size_t i = 0; i < a2_SZ; ++i)
  {
    std::pop_heap(b.begin(), b.end() - i);
    c.Remove(0);

    //for(uint32_t j = 0; j < c.Length(); ++j)
    //  fill[j]=c[j]; //Let's us visualize C's array
    //for(uint32_t j = 0; j < c.Length(); ++j)
    //  assert(c[j]==b[j]);
    arrtest(&b[0], c, c.Length());
  }
  ENDTEST;
}
