// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_AUTOLIST_H__BSS__ //These are used in case this header file is used by two different projects dependent on each other, resulting in duplicates which cannot be differentiated by #pragma once
#define __C_AUTOLIST_H__BSS__

#include "bss_defines.h"

namespace bss_util
{
  /* T is the class inheriting this item, P is the class holding the master list */
  template<class T, class P>
  class BSS_COMPILER_DLLEXPORT cAutoListItem
  {
  public:
    inline explicit cAutoListItem(P* srclist) : _srclist(srclist) { _listpos=_srclist->_autolist.Add(static_cast<T*>(this)); }
    inline cAutoListItem(const cAutoListItem& copy) : _srclist(copy._srclist) { _listpos=_srclist->_autolist.Add(static_cast<T*>(this)); }
    virtual inline ~cAutoListItem() { _srclist->_autolist.Remove(_listpos); }

    inline cAutoListItem& operator =(const cAutoListItem& copy)
    { 
      if(_srclist!=copy._srclist)
      {
        _srclist->_removeupdate(_listpos);
        _srclist=copy._srclist;
        _listpos=_srclist->_autolist.Add(static_cast<T*>(this));
      }
      return *this;
    }

  protected:
    P* _srclist;

  private:
		bss_util::cLLNode<T*>* _listpos;
  };

  /* T is the items that will be put into this list, P is the class that inherits this */
  template<class T, class P>
  class BSS_COMPILER_DLLEXPORT cAutoList
  {
    friend class cAutoListItem<T,P>;

  protected:
    bss_util::cLinkedList<T*> _autolist;
  };
}

#endif