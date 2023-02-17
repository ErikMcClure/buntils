// Copyright ©2018 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/LinkedArray.h"

using namespace bun;

TESTDEF::RETPAIR test_LINKEDARRAY()
{
  BEGINTEST;
  LinkedArray<int> _arr;
  size_t a = _arr.Add(4);
  _arr.SetCapacity(2);
  size_t b = _arr.InsertAfter(6, a);
  _arr.InsertBefore(5, b);
  TEST(_arr.Length() == 3);
  int v[] = { 4,5,6 };
  size_t c = 0;
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
