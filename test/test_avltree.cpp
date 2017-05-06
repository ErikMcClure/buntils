// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/AVLtree.h"
#include "bss-util/BlockAlloc.h"
#include "bss-util/algo.h"
#include "test.h"

using namespace bss;

int avltestnum[8];
BSS_FORCEINLINE bool AVLACTION(AVL_Node<int>* n) { static int c = 0; avltestnum[c++] = n->_key; return false; }
BSS_FORCEINLINE AVL_Node<int>* LAVLCHILD(AVL_Node<int>* n) { return n->_left; }
BSS_FORCEINLINE AVL_Node<int>* RAVLCHILD(AVL_Node<int>* n) { return n->_right; }

TESTDEF::RETPAIR test_AVLTREE()
{
  BEGINTEST;

  BlockPolicy<AVL_Node<std::pair<int, int>>> fixedavl;
  AVLTree<int, int, CompT<int>, BlockPolicy<AVL_Node<std::pair<int, int>>>> avlblah(&fixedavl);

  //uint64_t prof=HighPrecisionTimer::OpenProfiler();
  for(int i = 0; i<TESTNUM; ++i)
    avlblah.Insert(testnums[i], testnums[i]);
  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;

  Shuffle(testnums);
  //prof=HighPrecisionTimer::OpenProfiler();
  uint32_t c = 0;
  for(int i = 0; i<TESTNUM; ++i)
    c += (avlblah.GetRef(testnums[i]) != 0);
  TEST(c == TESTNUM);
  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;

  Shuffle(testnums);
  //prof=HighPrecisionTimer::OpenProfiler();
  for(int i = 0; i<TESTNUM; ++i)
    avlblah.Remove(testnums[i]);
  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;
  avlblah.Clear();

  c = 0;
  for(int i = 0; i<TESTNUM; ++i) // Test that no numbers are in the tree
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
    BlockAlloc<DEBUG_CDT<false>> dalloc(TESTNUM);
    typedef std::unique_ptr<DEBUG_CDT<false>, std::function<void(DEBUG_CDT<false>*)>> AVL_D;
    AVLTree<int, AVL_D, CompT<int>, BlockPolicy<AVL_Node<std::pair<int, AVL_D>>>> dtree;
    for(int i = 0; i<TESTNUM; ++i)
    {
      auto dp = dalloc.alloc(1);
      new(dp)DEBUG_CDT<false>(testnums[i]);
      dtree.Insert(testnums[i], AVL_D(dp, [&](DEBUG_CDT<false>* p) {p->~DEBUG_CDT<false>(); dalloc.dealloc(p); }));
    }

    Shuffle(testnums);
    c = 0;
    for(int i = 0; i<TESTNUM; ++i)
      c += (dtree.GetRef(testnums[i]) != 0);
    TEST(c == TESTNUM);

    Shuffle(testnums);
    c = 0;
    for(int i = 0; i<TESTNUM; ++i)
      c += dtree.ReplaceKey(testnums[i], testnums[i]);
    TEST(c == TESTNUM);

    Shuffle(testnums);
    c = 0;
    AVL_D* r;
    for(int i = 0; i<TESTNUM; ++i)
    {
      if((r = dtree.GetRef(testnums[i])) != 0)
        c += ((*r->get()) == testnums[i]);
    }
    TEST(c == TESTNUM);

    Shuffle(testnums);
    for(int i = 0; i<TESTNUM; ++i)
      dtree.Remove(testnums[i]);

    c = 0;
    for(int i = 0; i<TESTNUM; ++i)
      c += (dtree.GetRef(testnums[i]) == 0);
    TEST(c == TESTNUM);
    TEST(!DEBUG_CDT<false>::count)
  }
  TEST(!DEBUG_CDT<false>::count)

    AVLTree<int, void, CompT<int>, BlockPolicy<AVL_Node<int>>> avlblah2;

  //uint64_t prof=HighPrecisionTimer::OpenProfiler();
  for(int i = 0; i<TESTNUM; ++i)
    avlblah2.Insert(testnums[i]);
  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;

  Shuffle(testnums);
  //prof=HighPrecisionTimer::OpenProfiler();
  c = 0;
  for(int i = 0; i<TESTNUM; ++i)
    c += ((avlblah2.GetRef(testnums[i]) != 0)&(avlblah2.Get(testnums[i], -1) == testnums[i]));
  TEST(c == TESTNUM);
  //std::cout << HighPrecisionTimer::CloseProfiler(prof) << std::endl;

  Shuffle(testnums);
  //prof=HighPrecisionTimer::OpenProfiler();
  for(int i = 0; i<TESTNUM; ++i)
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
  BreadthFirstTree<AVL_Node<int>, AVLACTION, LAVLCHILD, RAVLCHILD>(avlblah2.GetRoot(), 8);
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
