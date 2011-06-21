// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_AUTOPTR_H__BSS__
#define __C_AUTOPTR_H__BSS__

namespace bss_util
{
  template<class _Ty>
  class cAutoPtr_Ex : public std::auto_ptr<_Ty>
  {
  public:
    inline cAutoPtr_Ex<_Ty>(_Ty* _newptr) : std::auto_ptr<_Ty>(_newptr) //redeclares as implicit
    {
    }

    inline cAutoPtr_Ex<_Ty>& operator =(_Ty* _newptr)
    {
      reset(_newptr);
		  return (*this);
    }

    inline bool operator !()
    {
      return !get();
    }
  };
}

#endif