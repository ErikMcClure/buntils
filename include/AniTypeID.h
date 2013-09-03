// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __ANI_TYPEID_H__BSS__
#define __ANI_TYPEID_H__BSS__

#include "delegate.h"
#include "cDef.h"

#define ANI_TID(tdef) typename bss_util::ANI_IDTYPE_EXPAND<bss_util::ANI_IDTYPE<TypeID>>::tdef

namespace bss_util {
  template<unsigned char T>
  struct ANI_IDTYPE {}; //if you need your own type, just insert another explicit specialization in your code

  template<typename V, typename T=void, typename D=V, typename S=void>
  struct ANI_IDTYPE_TYPES { typedef V VALUE; typedef D DATA; typedef T TRAIT; typedef S SAFE; };

  template<typename T, typename D>
  struct ANI_IDTYPE_EXPAND__
  {
    typedef T VALUE;
    typedef VALUE const& VALUECONST;
    typedef VALUE& VALUEREF;
  };

  template<typename T>
  struct ANI_IDTYPE_EXPAND__<T,void>
  {
    typedef T VALUE;
    typedef VALUE VALUECONST;
    typedef VALUE VALUEREF;
  };

  template<typename T> // Expands the typedefs VALUE and DATA to more useful references from ANI_IDTYPE
  struct ANI_IDTYPE_EXPAND
  {
    typedef typename ANI_IDTYPE_EXPAND__<typename T::TYPES::VALUE,typename T::TYPES::TRAIT>::VALUE VALUE;
    typedef typename ANI_IDTYPE_EXPAND__<typename T::TYPES::VALUE,typename T::TYPES::TRAIT>::VALUECONST VALUECONST;
    typedef typename ANI_IDTYPE_EXPAND__<typename T::TYPES::VALUE,typename T::TYPES::TRAIT>::VALUEREF VALUEREF;
    typedef typename T::TYPES::DATA DATA;
    typedef typename T::TYPES::SAFE SAFE;
    typedef DATA const& DATACONST;
    typedef DATA& DATAREF;
    typedef delegate<void,VALUECONST> DELEGATE;
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
