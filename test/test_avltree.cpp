// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/AVLTree.h"
#include "buntils/BlockAlloc.h"
#include "buntils/algo.h"
#include <functional>

using namespace bun;

int avltestnum[8];
BUN_FORCEINLINE bool AVLACTION(AVLNode<int>* n) { static int c = 0; avltestnum[c++] = n->_key; return false; }
BUN_FORCEINLINE AVLNode<int>* LAVLCHILD(AVLNode<int>* n) { return n->_left; }
BUN_FORCEINLINE AVLNode<int>* RAVLCHILD(AVLNode<int>* n) { return n->_right; }

TESTDEF::RETPAIR test_AVLTREE()
{
  BEGINTEST;

  BlockPolicy<AVLNode<std::pair<int, int>>> fixedavl;
  AVLTree<int, int, std::compare_three_way, PolicyAllocator<AVLNode<std::pair<int, int>>, BlockPolicy>> avlblah(PolicyAllocator<AVLNode<std::pair<int, int>>, BlockPolicy>{fixedavl});

  //uint64_t prof=HighPrecisionTimer::OpenProfiler();
  for(size_t i = 0; i<TESTNUM; ++i)
    avlblah.Insert(testnums[i], testnums[i]);
  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;

  shuffle_testnums();
  //prof=HighPrecisionTimer::OpenProfiler();
  uint32_t c = 0;
  for(size_t i = 0; i<TESTNUM; ++i)
    c += (avlblah.GetRef(testnums[i]) != 0);
  TEST(c == TESTNUM);
  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;

  shuffle_testnums();
  //prof=HighPrecisionTimer::OpenProfiler();
  for(size_t i = 0; i<TESTNUM; ++i)
    avlblah.Remove(testnums[i]);
  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;
  avlblah.Clear();

  c = 0;
  for(size_t i = 0; i<TESTNUM; ++i) // Test that no numbers are in the tree
    c += (avlblah.GetRef(testnums[i]) == 0);
  TEST(c == TESTNUM);

  BlockPolicy<AVLNode<std::pair<int, std::pair<int, int>*>>> fixedavlp;
  auto tree = new AVLTree<int, std::pair<int, int>*>(PolicyAllocator<AVLNode<std::pair<int, std::pair<int, int>*>>, BlockPolicy>{fixedavlp});
  std::pair<int, int> test(5, 5);
  tree->Insert(test.first, &test);
  tree->Get(test.first, 0);
  TEST(tree->Near(-1) != 0);
  TEST(tree->Near(4) != 0);
  TEST(tree->Near(6) != 0);
  tree->ReplaceKey(5, 2);
  tree->Remove(test.first);
  tree->Clear();
  delete tree;

  //DEBUG_CDT_SAFE<false>::_testret=&__testret; // Set things up so we can ensure AVLTree handles constructors/destructors properly.
  DEBUG_CDT<false>::count = 0;

  {
    shuffle_testnums();
    BlockPolicy<DEBUG_CDT<false>> dalloc(TESTNUM);
    using AVL_D = std::unique_ptr<DEBUG_CDT<false>, std::function<void(DEBUG_CDT<false>*)>>;
    BlockPolicy<AVLNode<std::pair<int, AVL_D>>> fixedavld;
    AVLTree<int, AVL_D, std::compare_three_way, PolicyAllocator<AVLNode<std::pair<int, AVL_D>>, BlockPolicy>> dtree(PolicyAllocator<AVLNode<std::pair<int, AVL_D>>, BlockPolicy>{fixedavld});
    for(size_t i = 0; i<TESTNUM; ++i)
    {
      auto dp = dalloc.allocate(1);
      new(dp)DEBUG_CDT<false>(testnums[i]);
      dtree.Insert(testnums[i], AVL_D(dp, [&](DEBUG_CDT<false>* p) {p->~DEBUG_CDT<false>(); dalloc.deallocate(p, 1); }));
    }

    shuffle_testnums();
    c = 0;
    for(size_t i = 0; i<TESTNUM; ++i)
      c += (dtree.GetRef(testnums[i]) != 0);
    TEST(c == TESTNUM);

    shuffle_testnums();
    c = 0;
    for(size_t i = 0; i<TESTNUM; ++i)
      c += dtree.ReplaceKey(testnums[i], testnums[i]);
    TEST(c == TESTNUM);

    shuffle_testnums();
    c = 0;
    AVL_D* r;
    for(size_t i = 0; i<TESTNUM; ++i)
    {
      if((r = dtree.GetRef(testnums[i])) != 0)
        c += ((*r->get()) == testnums[i]);
    }
    TEST(c == TESTNUM);

    shuffle_testnums();
    for(size_t i = 0; i<TESTNUM; ++i)
      dtree.Remove(testnums[i]);

    c = 0;
    for(size_t i = 0; i<TESTNUM; ++i)
      c += (dtree.GetRef(testnums[i]) == 0);
    TEST(c == TESTNUM);
    TEST(!DEBUG_CDT<false>::count)
  }
  TEST(!DEBUG_CDT<false>::count)

    BlockPolicy<AVLNode<int>> fixedavlkey;
  AVLTree<int, void, std::compare_three_way, PolicyAllocator<AVLNode<int>, BlockPolicy>> avlblah2{ PolicyAllocator<AVLNode<int>, BlockPolicy>{fixedavlkey} };

  //uint64_t prof=HighPrecisionTimer::OpenProfiler();
  for(size_t i = 0; i<TESTNUM; ++i)
    avlblah2.Insert(testnums[i]);
  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;

  shuffle_testnums();
  //prof=HighPrecisionTimer::OpenProfiler();
  c = 0;
  for(size_t i = 0; i<TESTNUM; ++i)
    c += ((avlblah2.GetRef(testnums[i]) != 0)&(avlblah2.Get(testnums[i], -1) == testnums[i]));
  TEST(c == TESTNUM);
  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;

  shuffle_testnums();
  //prof=HighPrecisionTimer::OpenProfiler();
  for(size_t i = 0; i<TESTNUM; ++i)
    avlblah2.Remove(testnums[i]);
  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;
  avlblah2.Clear();

  avlblah2.Insert(1);
  avlblah2.Insert(4);
  avlblah2.Insert(3);
  avlblah2.Insert(5);
  avlblah2.Insert(2);
  avlblah2.Insert(-1);
  avlblah2.Insert(-2);
  avlblah2.Insert(-3);
  BreadthFirstTree<AVLNode<int>, AVLACTION, LAVLCHILD, RAVLCHILD>(avlblah2.GetRoot(), 8);
  TEST(avltestnum[0] == 3);
  TEST(avltestnum[1] == 4);
  TEST(avltestnum[2] == 1);
  TEST(avltestnum[3] == 5);
  TEST(avltestnum[4] == 2);
  TEST(avltestnum[5] == -2);
  TEST(avltestnum[6] == -1);
  TEST(avltestnum[7] == -3);
  ENDTEST;
}
