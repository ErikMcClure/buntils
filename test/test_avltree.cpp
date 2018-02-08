// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "bss-util/AVLTree.h"
#include "bss-util/BlockAlloc.h"
#include "bss-util/algo.h"
#include <functional>

using namespace bss;

int avltestnum[8];
BSS_FORCEINLINE bool AVLACTION(AVLNode<int>* n) { static int c = 0; avltestnum[c++] = n->_key; return false; }
BSS_FORCEINLINE AVLNode<int>* LAVLCHILD(AVLNode<int>* n) { return n->_left; }
BSS_FORCEINLINE AVLNode<int>* RAVLCHILD(AVLNode<int>* n) { return n->_right; }

TESTDEF::RETPAIR test_AVLTREE()
{
  BEGINTEST;

  BlockPolicy<AVLNode<std::pair<int, int>>> fixedavl;
  AVLTree<int, int, CompT<int>, PolymorphicAllocator<AVLNode<std::pair<int, int>>, BlockPolicy>> avlblah(&fixedavl);

  //uint64_t prof=HighPrecisionTimer::OpenProfiler();
  for(size_t i = 0; i<TESTNUM; ++i)
    avlblah.Insert(testnums[i], testnums[i]);
  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;

  Shuffle(testnums);
  //prof=HighPrecisionTimer::OpenProfiler();
  uint32_t c = 0;
  for(size_t i = 0; i<TESTNUM; ++i)
    c += (avlblah.GetRef(testnums[i]) != 0);
  TEST(c == TESTNUM);
  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;

  Shuffle(testnums);
  //prof=HighPrecisionTimer::OpenProfiler();
  for(size_t i = 0; i<TESTNUM; ++i)
    avlblah.Remove(testnums[i]);
  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;
  avlblah.Clear();

  c = 0;
  for(size_t i = 0; i<TESTNUM; ++i) // Test that no numbers are in the tree
    c += (avlblah.GetRef(testnums[i]) == 0);
  TEST(c == TESTNUM);

  AVLTree<int, std::pair<int, int>*>* tree = new AVLTree<int, std::pair<int, int>*>();
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
    Shuffle(testnums);
    BlockPolicy<DEBUG_CDT<false>> dalloc(TESTNUM);
    typedef std::unique_ptr<DEBUG_CDT<false>, std::function<void(DEBUG_CDT<false>*)>> AVL_D;
    AVLTree<int, AVL_D, CompT<int>, PolymorphicAllocator<AVLNode<std::pair<int, AVL_D>>, BlockPolicy>> dtree;
    for(size_t i = 0; i<TESTNUM; ++i)
    {
      auto dp = dalloc.allocate(1);
      new(dp)DEBUG_CDT<false>(testnums[i]);
      dtree.Insert(testnums[i], AVL_D(dp, [&](DEBUG_CDT<false>* p) {p->~DEBUG_CDT<false>(); dalloc.deallocate(p, 1); }));
    }

    Shuffle(testnums);
    c = 0;
    for(size_t i = 0; i<TESTNUM; ++i)
      c += (dtree.GetRef(testnums[i]) != 0);
    TEST(c == TESTNUM);

    Shuffle(testnums);
    c = 0;
    for(size_t i = 0; i<TESTNUM; ++i)
      c += dtree.ReplaceKey(testnums[i], testnums[i]);
    TEST(c == TESTNUM);

    Shuffle(testnums);
    c = 0;
    AVL_D* r;
    for(size_t i = 0; i<TESTNUM; ++i)
    {
      if((r = dtree.GetRef(testnums[i])) != 0)
        c += ((*r->get()) == testnums[i]);
    }
    TEST(c == TESTNUM);

    Shuffle(testnums);
    for(size_t i = 0; i<TESTNUM; ++i)
      dtree.Remove(testnums[i]);

    c = 0;
    for(size_t i = 0; i<TESTNUM; ++i)
      c += (dtree.GetRef(testnums[i]) == 0);
    TEST(c == TESTNUM);
    TEST(!DEBUG_CDT<false>::count)
  }
  TEST(!DEBUG_CDT<false>::count)

    AVLTree<int, void, CompT<int>, PolymorphicAllocator<AVLNode<int>, BlockPolicy>> avlblah2;

  //uint64_t prof=HighPrecisionTimer::OpenProfiler();
  for(size_t i = 0; i<TESTNUM; ++i)
    avlblah2.Insert(testnums[i]);
  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;

  Shuffle(testnums);
  //prof=HighPrecisionTimer::OpenProfiler();
  c = 0;
  for(size_t i = 0; i<TESTNUM; ++i)
    c += ((avlblah2.GetRef(testnums[i]) != 0)&(avlblah2.Get(testnums[i], -1) == testnums[i]));
  TEST(c == TESTNUM);
  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;

  Shuffle(testnums);
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
