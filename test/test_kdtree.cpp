// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "bss-util/KDTree.h"
#include "bss-util/BlockAlloc.h"

using namespace bss;

struct KDtest {
  float rect[4];
  LLBase<int> list;
  KDNode<KDtest>* node;
  static int hits;
};
int KDtest::hits = 0;
BSS_FORCEINLINE const float* KDtest_RECT(KDtest* t) { return t->rect; }
BSS_FORCEINLINE LLBase<KDtest>& KDtest_LIST(KDtest* t) { return *(LLBase<KDtest>*)&t->list; }
BSS_FORCEINLINE void KDtest_ACTION(KDtest* t) { ++KDtest::hits; }
BSS_FORCEINLINE KDNode<KDtest>*& KDtest_NODE(KDtest* t) { return t->node; }

TESTDEF::RETPAIR test_KDTREE()
{
  BEGINTEST;
  BlockPolicy<KDNode<KDtest>> alloc;
  KDTree<KDtest, &KDtest_RECT, &KDtest_LIST, &KDtest_NODE, PolymorphicAllocator<KDNode<KDtest>, BlockPolicy>> tree;
  KDtest r1 = { 0,0,1,1,0,0 };
  KDtest r2 = { 1,1,2,2,0,0 };
  KDtest r3 = { 0,0,2,2,0,0 };
  KDtest r4 = { 0.5,0.5,1.5,1.5,0,0 };
  KDtest r5 = { 0.5,0.5,0.75,0.75,0,0 };
  tree.Insert(&r1);
  tree.Insert(&r2);
  tree.Remove(&r1);
  tree.Insert(&r1);
  tree.Insert(&r3);
  tree.Insert(&r4);
  tree.Remove(&r4);
  tree.Insert(&r5);
  tree.Insert(&r4);
  tree.Clear();
  tree.InsertRoot(&r1);
  tree.InsertRoot(&r2);
  tree.InsertRoot(&r3);
  tree.InsertRoot(&r4);
  tree.InsertRoot(&r5);
  tree.Solve();
  float c1[4] = { -1,-1,-0.5,-0.5 };
  tree.Traverse<&KDtest_ACTION>(c1);
  TEST(KDtest::hits == 0);
  float c2[4] = { -1,-1,0,0 };
  tree.Traverse<&KDtest_ACTION>(c2);
  TEST(KDtest::hits == 2);
  tree.Remove(&r1);
  tree.Remove(&r2);
  tree.Remove(&r3);
  tree.Remove(&r4);
  tree.Remove(&r5);
  tree.Traverse<&KDtest_ACTION>(c1);
  TEST(tree.GetRoot()->num == 0);
  ENDTEST;
}
