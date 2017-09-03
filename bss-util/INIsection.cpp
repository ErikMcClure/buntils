// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/INIstorage.h"
#include <memory.h>

using namespace bss;

INIentry INIsection::_entrysentinel;
LocklessBlockAlloc<INIsection::_NODE> INIsection::_alloc;

INIsection::INIsection(const INIsection& copy) : _name(copy._name),_index(copy._index), _parent(copy._parent), _root(0),_last(0)
{
  _copy(copy);
}
INIsection::INIsection(INIsection&& mov) : _name(std::move(mov._name)),_index(mov._index), _parent(mov._parent),
  _entries(std::move(mov._entries)),_root(mov._root),_last(mov._last)
{
  mov._root=0;
  mov._last=0;
}
INIsection::INIsection() : _index((size_t)-1), _parent(0), _root(0), _last(0)
{
}
INIsection::INIsection(const char* name, INIstorage* parent, size_t index) : _name(name), _index(index), _parent(parent), _root(0),_last(0)
{
}
INIsection::~INIsection()
{
  _destroy();
}
void INIsection::_destroy()
{
  _NODE* t;
  while(_root)
  {
    t=_root->next;
    _root->~_NODE();
    _alloc.Dealloc(_root);
    _root=t;
  }
  _last=0;
}
void INIsection::_copy(const INIsection& copy)
{
  assert(!_root && !_last);
  _NODE* t=copy._root;
  _NODE* last=0;
  size_t c=0;
  while(t)
  {
    _NODE* p=_alloc.Alloc(1);
    bssFill(*p, 0);
    new (&p->val) INIentry(t->val);
    if(!_root)
      _root=_last=p;
    else
      _last=LLAddAfter(p,_last);

    if(t->instances.Capacity()!=0) {
      _entries.Insert(p->val.GetKey(),p);
      p->instances.SetCapacity(c=t->instances.Capacity());
      last=t;
      --c;
    } else if(c>0) {
      assert(last!=0);
      last->instances[last->instances.Capacity()-(c--)-1]=p; //This never goes negative because c>0 and is therefore at least 1
    } else
      _entries.Insert(p->val.GetKey(),p);

    t=t->next;
  }
}

void INIsection::_addEntry(const char* key, const char* data)
{
  _NODE* p=_alloc.Alloc(1);
  bssFill(*p, 0);
  new (&p->val) INIentry(key,data);
  khiter_t iter=_entries.Iterator(key);

  if(iter==_entries.End())
  {
    _entries.Insert(p->val.GetKey(),p);
    if(!_root)
      _root=_last=p;
    else
      _last=LLAddAfter(p,_last);
  } else {
    assert(_last!=0 && _root!=0);
    _NODE* r=_entries.UnsafeValue(iter);
    _NODE* t=!r->instances.Capacity()?r:r->instances.Back();
    LLInsertAfter(p,t,_last);
    r->instances.Insert(p,r->instances.Capacity());
  }
}

INIsection& INIsection::operator=(const INIsection& right)
{ 
  if(&right == this) return *this;
  _name=right._name;
  _index=right._index;
  _parent=right._parent;
  _destroy();
  _entries.Clear();
  _copy(right);
  return *this;
}

INIsection& INIsection::operator=(INIsection&& mov)
{
  if(&mov == this) return *this;
  _name=std::move(mov._name);
  _index=mov._index;
  _parent=mov._parent;
  _entries=std::move(mov._entries);
  _root=mov._root;
  _last=mov._last;
  mov._root=0;
  mov._last=0;
  return *this;
}

INIsection::_NODE* INIsection::GetEntryNode(const char* key, size_t instance) const
{
  _NODE* n = _entries[key];
  if(!n) return 0;
  if(!instance) return n;
  return (instance>n->instances.Capacity())?0:(n->instances[instance-1]);
}

INIentry* INIsection::GetEntryPtr(const char* key, size_t instance) const
{ 
  _NODE* entry=GetEntryNode(key,instance);
  return !entry?0:&entry->val;
}

size_t INIsection::GetNumEntries(const char* key) const
{
  _NODE* n = _entries[key];
  if(!n) return 0;
  return n->instances.Capacity()+1;
}

INIentry& INIsection::GetEntry(const char* key, size_t instance) const
{ 
  _NODE* entry=GetEntryNode(key,instance);
  return !entry?_entrysentinel:entry->val;
}

char INIsection::EditEntry(const char* key, const char* data, size_t instance)
{  // We put this down here because the compiler never actually inlines it, so there's no point in going through hoops to keep it inline.
  return !_parent?-1:_parent->EditEntry(_name,key,data,instance,_index); 
}
