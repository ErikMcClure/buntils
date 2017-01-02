// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "cLinkedArray.h"

using namespace bss_util;

TESTDEF::RETPAIR test_LINKEDARRAY()
{
  BEGINTEST;
  cLinkedArray<int> _arr;
  uint32_t a = _arr.Add(4);
  _arr.Reserve(2);
  uint32_t b = _arr.InsertAfter(6, a);
  _arr.InsertBefore(5, b);
  TEST(_arr.Length() == 3);
  int v[] = { 4,5,6 };
  uint32_t c = 0;
  for(auto i = _arr.begin(); i != _arr.end(); ++i)
    TEST(*i == v[c++]);
  _arr.Remove(b);
  TEST(_arr.Length() == 2);
  TEST(_arr[a] == 4);
  TEST(*_arr.GetItemPtr(a) == 4);
  c = 0;
  for(auto i = _arr.begin(); i != _arr.end(); ++i)
    TEST(*i == v[c++]);
  _arr.Clear();
  TEST(!_arr.Length());
  ENDTEST;

}
