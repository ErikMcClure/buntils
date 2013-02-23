// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __ANI_TYPEID_H__BSS__
#define __ANI_TYPEID_H__BSS__

#include "delegate.h"
#include "cDef.h"

#define ANI_TID(tdef) typename bss_util::ANI_IDTYPE_EXPAND<bss_util::ANI_IDTYPE<TypeID>>::tdef

namespace bss_util {
  template<unsigned char T>
  struct ANI_IDTYPE {}; //if you need your own type, just insert another explicit specialization in your code

  template<typename T, typename D>
  struct ANI_IDTYPE_EXPAND__
  {
    typedef typename T::VALUE VALUE;
    typedef VALUE const& VALUECONST;
    typedef VALUE& VALUEREF;
  };

  template<typename T>
  struct ANI_IDTYPE_EXPAND__<T,void>
  {
    typedef typename T::VALUE VALUE;
    typedef VALUE VALUECONST;
    typedef VALUE VALUEREF;
  };

  template<typename T, typename D>
  struct ANI_IDTYPE_EXPAND__DEL__ { typedef D DELEGATE; };
  template<typename T>
  struct ANI_IDTYPE_EXPAND__DEL__<T,void> { typedef delegate<void,T> DELEGATE; };

  template<typename T> // Expands the typedefs VALUE and DATA to more useful references from ANI_IDTYPE
  struct ANI_IDTYPE_EXPAND : ANI_IDTYPE_EXPAND__<T,typename T::TRAIT> 
  {
    typedef typename ANI_IDTYPE_EXPAND__::VALUE VALUE;
    typedef typename ANI_IDTYPE_EXPAND__::VALUECONST VALUECONST;
    typedef typename ANI_IDTYPE_EXPAND__::VALUEREF VALUEREF;
    typedef typename T::DATA DATA;
    typedef DATA const& DATACONST;
    typedef DATA& DATAREF;
    typedef typename ANI_IDTYPE_EXPAND__DEL__<VALUECONST,typename T::DEL>::DELEGATE DELEGATE;
  };

  struct AniAttribute;

  // Abstract class designed to be inherited using virtual inheritance so you can call the TypeIDRegFunc from any class 
	struct BSS_COMPILER_DLLEXPORT cAbstractAnim
  {
    virtual AniAttribute* BSS_FASTCALL TypeIDRegFunc(unsigned char TypeID)=0;
    static AniAttribute* BSS_FASTCALL SpawnBase(const cDef<AniAttribute>& p);
    static void* BSS_FASTCALL AnimAlloc(size_t n, void* p=0); // Behaves like Realloc, send in a null pointer for p to behave like malloc
    static void BSS_FASTCALL AnimFree(void* p);
  };
}
  
#endif
