// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "cINIstorage.h"
#include <memory.h>

using namespace bss_util;

cINIentry cINIsection::_entrysentinel;
cFixedAlloc<cINIsection::_NODE,4> cINIsection::_alloc;

cINIsection::cINIsection(const cINIsection& copy) : _name(copy._name),_parent(copy._parent),_index(copy._index),_root(0),_last(0)
{
  _copy(copy);
}
cINIsection::cINIsection(cINIsection&& mov) : _name(std::move(mov._name)),_parent(mov._parent),_index(mov._index),
  _entries(std::move(mov._entries)),_root(mov._root),_last(mov._last)
{
  mov._root=0;
  mov._last=0;
}
cINIsection::cINIsection() : _parent(0), _index((unsigned int)-1), _root(0), _last(0)
{
}
cINIsection::cINIsection(const char* name, cINIstorage* parent, unsigned int index) : _name(name), _parent(parent), _index(index), _root(0),_last(0)
{
}
cINIsection::~cINIsection()
{
  _destroy();
}
void cINIsection::_destroy()
{
  _NODE* t;
  while(_root)
  {
    t=_root->next;
    _root->~_NODE();
    _alloc.dealloc(_root);
    _root=t;
  }
  _last=0;
}
void cINIsection::_copy(const cINIsection& copy)
{
  assert(!_root && !_last);
  _NODE* t=copy._root;
  _NODE* last=0;
  unsigned int c=0;
  while(t)
  {
    _NODE* p=_alloc.alloc(1);
    memset(p,0,sizeof(_NODE));
    new (&p->val) cINIentry(t->val);
    if(!_root)
      _root=_last=p;
    else
      _last=LLAdd(p,_last);

    if(t->instances.Size()!=0) {
      _entries.Insert(p->val.GetKey(),p);
      p->instances.SetSize(c=t->instances.Size());
      last=t;
      --c;
    } else if(c>0) {
      assert(last!=0);
      last->instances[last->instances.Size()-(c--)-1]=p; //This never goes negative because c>0 and is therefore at least 1
    } else
      _entries.Insert(p->val.GetKey(),p);

    t=t->next;
  }
}

void cINIsection::_addentry(const char* key, const char* data)
{
  _NODE* p=_alloc.alloc(1);
  memset(p,0,sizeof(_NODE));
  new (&p->val) cINIentry(key,data);
  khiter_t iter=_entries.GetIterator(key);

  if(iter==_entries.End())
  {
    _entries.Insert(p->val.GetKey(),p);
    if(!_root)
      _root=_last=p;
    else
      _last=LLAdd(p,_last);
  } else {
    assert(_last!=0 && _root!=0);
    _NODE* r=_entries[iter];
    _NODE* t=!r->instances.Size()?r:r->instances.Back();
    LLInsertAfterAssign(p,t);
    LLInsertAfter(p,t,_last);
    r->instances.Insert(p,r->instances.Size());
  }
}

cINIsection& cINIsection::operator=(const cINIsection& right)
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

cINIsection& cINIsection::operator=(cINIsection&& mov)
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

cINIentry* cINIsection::GetEntryPtr(const char* key, unsigned int instance) const
{ 
  khiter_t iter= _entries.GetIterator(key);
  if(iter==_entries.End()) return 0;
  _NODE* n=_entries[iter];
  if(!instance) return &n->val;
  return (instance>n->instances.Size())?0:(&n->instances[instance-1]->val);
}

cINIentry& cINIsection::GetEntry(const char* key, unsigned int instance) const
{ 
  cINIentry* entry=GetEntryPtr(key,instance);
  return !entry?_entrysentinel:*entry;
}

char cINIsection::EditEntry(const char* key, const char* data, unsigned int instance)
{  // We put this down here because the compiler never actually inlines it, so there's no point in going through hoops to keep it inline.
  return !_parent?-1:_parent->EditEntry(_name,key,data,instance,_index); 
}

/*cINIsection::cINIsection(const cINIsection& copy) : _name(copy._name),_parent(copy._parent),_index(copy._index)
{
  _copyhash(copy);
}
cINIsection::cINIsection(cINIsection&& mov) : _name(std::move(mov._name)),_parent(mov._parent),_index(mov._index), _entries(std::move(mov._entries))
{
}
cINIsection::cINIsection() : _parent(0), _index((unsigned int)-1)
{
}
cINIsection::cINIsection(const char* name, cINIstorage* parent, unsigned int index) : _name(name), _parent(parent), _index(index)
{
}
cINIsection::~cINIsection()
{
  _destroyhash();
}
void cINIsection::_destroyhash()
{
  if(!_entries.Nullified())
  {
    __ARR* arr;
    for(auto iter=_entries.begin(); iter.IsValid(); ++iter)
    {
      arr=_entries[*iter];
      unsigned int svar=arr->Size();
      for(unsigned int i =0; i<svar; ++i) (*arr)[i].~cINIentry();
      delete arr;
    }
  }
}
void cINIsection::_copyhash(const cINIsection& copy)
{
  __ARR* old;
  __ARR* arr;
  for(auto iter=copy._entries.begin(); iter.IsValid(); ++iter)
  {
    old=copy._entries[*iter];
    arr=new __ARR(*old);
    unsigned int svar=arr->Size();
    for(unsigned int i =0; i < svar; ++i)
      new ((*arr)+i) cINIentry((*old)[i]);
    _entries.Insert((*arr)[0].GetKey(),arr);
  }
}
void cINIsection::_addentry(const char* key, const char* data)
{
  khiter_t iter=_entries.GetIterator(key);
  __ARR* arr;
  if(iter==_entries.End()) //need to add in an array for this
  {
    arr= new __ARR(1);
    new ((*arr)+0) cINIentry(key,data);
    _entries.Insert((*arr)[0].GetKey(),arr);
  } else {
    arr=_entries[iter];
    unsigned int index=arr->Size();
    arr->SetSize(index+1);
    new ((*arr)+index) cINIentry(key,data);
    _entries.OverrideKeyPtr(iter,(*arr)[0].GetKey());
  }
}

void cINIsection::_BuildEntryList(std::vector<std::pair<std::pair<cStr,cStr>,unsigned int>>& list) const
{
  __ARR* arr;
  unsigned int i =0;
  for(auto iter=_entries.begin(); iter.IsValid(); ++iter)
  {
    arr=_entries[*iter];
    for(i=0; i<arr->Size();++i)
      list.push_back(std::pair<std::pair<cStr,cStr>,unsigned int>(std::pair<cStr,cStr>((*arr)[i].GetKey(),(*arr)[i].GetString()),i));
  }
}

cINIsection& cINIsection::operator=(const cINIsection& right)
{ 
  if(&right == this) return *this;
  _name=right._name;
  _index=right._index;
  _parent=right._parent;
  _destroyhash();
  _entries.Clear();
  _copyhash(right);
  return *this;
}

cINIsection& cINIsection::operator=(cINIsection&& mov)
{
  if(&mov == this) return *this;
  _name=std::move(mov._name);
  _index=mov._index;
  _parent=mov._parent;
  _destroyhash();
  _entries=std::move(mov._entries);
  return *this;
}

cINIentry* cINIsection::GetEntryPtr(const char* key, unsigned int instance) const
{ 
  khiter_t iter= _entries.GetIterator(key);
  if(iter==_entries.End()) return 0;
  __ARR* arr=_entries[iter];
  return instance<arr->Size()?(*arr)+instance:0;
}

cINIentry& cINIsection::GetEntry(const char* key, unsigned int instance) const
{ 
  cINIentry* entry=GetEntryPtr(key,instance);
  return !entry?_entrysentinel:*entry;
}*/

