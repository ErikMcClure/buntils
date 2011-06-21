// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_MINILIST_H__BSS__ //These are used in case this header file is used by two different projects dependent on each other, resulting in duplicates which cannot be differentiated by #pragma once
#define __C_MINILIST_H__BSS__

#include "cHolder.h"

namespace bss_util
{
  template<class T>
  class cMiniList;

  /* This is a class that must be inherited by the objects that will act as list items */
  template<class T> //T must inherit cMiniListItem<T>
  class __declspec(dllexport) cMiniListItem : protected cHolder<cMiniList<T>>
  {
  public:
    cMiniListItem(const T& copy);
    explicit cMiniListItem(cMiniList<T>* minilist, T* self);
    virtual ~cMiniListItem();
    T* GetPrev() { return _prev; }
    T* GetNext() { return _next; }
    void SetParentList(cMiniList<T>* minilist);
    cMiniList<T>* GetParentList() const { return _holdobject; }
    cMiniListItem<T>& operator=(const T& right);

  protected:
    friend class cMiniList<T>;

    mutable T* _prev;
    mutable T* _next;

  private:
    T* _self;
  };

  /* The minilist is a way of abstracting the construction of a very fast and very simple doubly-linked list. */
  template<class T>
  class __declspec(dllexport) cMiniList
  {
  public:
    cMiniList() : _root(0) {}
    ~cMiniList() { RemoveAll(); }
    void AddListItem(T* item)
    {
      ((cMiniListItem<T>*)item)->SetParentList(this);
      ((cMiniListItem<T>*)item)->_next=_root;
      if(_root) ((cMiniListItem<T>*)_root)->_prev=item;
      else _last=item;
      ((cMiniListItem<T>*)item)->_prev=0;
      _root=item;
    }
    void AddListItemOnEnd(T* item)
    {
      ((cMiniListItem<T>*)item)->SetParentList(this);
      ((cMiniListItem<T>*)item)->_prev=_last;
      if(_last) ((cMiniListItem<T>*)_last)->_next=item;
      else _root=item;
      ((cMiniListItem<T>*)item)->_next=0;
      _last=item;
    }
    void InsertListItem(T* item, T* target)
    {
      if(!target) 
        AddListItemOnEnd(item); //We do this because its possible to replace the root with this method, but not the last element
      else if(!target->_prev) //If this is true your trying to insert at the root, which is the same as AddListItem
        AddListItem(item);
      else
      {
        ((cMiniListItem<T>*)item)->SetParentList(this);
        ((cMiniListItem<T>*)item)->_next=target;
        ((cMiniListItem<T>*)item)->_prev=((cMiniListItem<T>*)target)->_prev;
        ((cMiniListItem<T>*)target)->_prev=item;
      }
    }
    void RemoveListItem(T* item)
    {
      if(((cMiniListItem<T>*)item)->GetParentList() != this)
        return;
      else
        ((cMiniListItem<T>*)item)->_holdobject=0;
      
      if(((cMiniListItem<T>*)item)->_next!=0)
        ((cMiniListItem<T>*)((cMiniListItem<T>*)item)->_next)->_prev=((cMiniListItem<T>*)item)->_prev;
      else //This is the last
        _last=((cMiniListItem<T>*)item)->_prev;

      if(!((cMiniListItem<T>*)item)->_prev) //if this is true this is the root
        _root=((cMiniListItem<T>*)item)->_next;
      else
        ((cMiniListItem<T>*)((cMiniListItem<T>*)item)->_prev)->_next=((cMiniListItem<T>*)item)->_next;
    }
    void RemoveAll() { while(_root!=0) RemoveListItem(_root); }

  protected:
    T* _root;
    T* _last;
  };

#pragma warning(push)
#pragma warning(disable:4355)
  template<class T>
  cMiniListItem<T>::cMiniListItem(const T& copy) : _self((T*)this), cHolder<cMiniList<T>>(copy), _prev(0), _next(0) { if(_holdobject!=0) _holdobject->InsertListItem(_self, copy._self); }

  template<class T>
  cMiniListItem<T>::cMiniListItem(cMiniList<T>* minilist, T* self) : _self(self), cHolder<cMiniList<T>>(minilist), _prev(0), _next(0) { if(_holdobject!=0) _holdobject->AddListItem(_self); }

  template<class T>
  cMiniListItem<T>::~cMiniListItem() { if(_holdobject!=0) _holdobject->RemoveListItem((T*)this); }
#pragma warning(pop)

  template<class T>
  void cMiniListItem<T>::SetParentList(cMiniList<T>* minilist)
  {
    if(_holdobject==minilist) return;
    if(_holdobject!=0) _holdobject->RemoveListItem(_self);
    _holdobject=minilist;
    //if(_holdobject!=0) _holdobject->AddListItem(_self); //This is impossible to do without breaking shit
  }

  template<class T>
  cMiniListItem<T>& cMiniListItem<T>::operator=(const T& right)
  { 
    if(_holdobject!=0) _holdobject->RemoveListItem(_self);
    _holdobject=right._holdobject;
    _self=(T*)this;
    if(_holdobject!=0) _holdobject->InsertListItem(_self, right._self);
  }
}

#endif