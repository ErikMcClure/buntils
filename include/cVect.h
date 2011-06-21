// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_VECT_H__BSS__
#define __C_VECT_H__BSS__

#include <vector>
#include "bss_call.h"
#include "bss_traits.h"

namespace bss_util {  
  /* this implements a special method where objects are able to keep track of their position inside the array, eliminating the need to search for them and providing superfast lookups */
  template<class T, class Traits=RefTraits<T>>
  class cVectUnique : protected Traits//This is NOT exported because it exposes a seriously bizzare DLL-boundary problem in std::vector.
  {
    typedef typename Traits::pointer pointer;
    typedef typename Traits::const_pointer const_pointer;
    typedef typename Traits::reference reference;
    typedef typename Traits::const_reference const_reference;
    typedef typename Traits::value_type value_type;

  public:
    typedef const unsigned int& VECTID;
    typedef std::pair<T,size_t> VECTHOLD;

    inline cVectUnique() {}
    inline cVectUnique(const cVectUnique<T,Traits>& copy) { operator =(copy); }
    inline ~cVectUnique() { Clear(); }
    inline VECTID BSS_FASTCALL AddVect(const_reference obj)
    {
      size_t retval = _vector.size();
      _vector.push_back(new VECTHOLD(obj,retval));
      return _vector[retval]->second;
    }
    inline VECTID BSS_FASTCALL AddVect(const_reference obj, size_t pos)
    {
      //if(pos >= _vector.size()) return AddVect(obj); //No point "inserting" after all of the items //No point making this check
      
      unsigned int svar=_vector.size(); //size doesn't change
      for(unsigned int i = pos; i < svar; ++i)
        ++_vector[i]->second; //This allows us to keep all of the IDs valid

      _vector.insert(_vector.begin()+pos,new VECTHOLD(obj,pos));
      return _vector[pos]->second;
    }
    inline value_type BSS_FASTCALL RemoveVect(VECTID index)
    {
      value_type retval = _vector[index]->first;
      delete _vector[index];

      unsigned int svar=_vector.size(); //size doesn't change
      for(unsigned int i = index+1; i < svar; ++i)
        --_vector[i]->second; //This allows us to keep all of the IDs valid

      _vector.erase(_vector.begin()+index);

      return retval;
    }
    inline const_reference BSS_FASTCALL GetVect(VECTID index) const { return _vector[index]->first; }
    inline reference BSS_FASTCALL GetVect(VECTID index) { return _vector[index]->first; }
    inline VECTID GetVectID(unsigned int index) const { return _vector[index]->second; }
    inline void Clear() { unsigned int svar=_vector.size(); for(unsigned int i = 0; i < svar; ++i) delete _vector[i];  _vector.clear(); }
    inline size_t Size() const { return _vector.size(); }

    inline const_reference operator [](VECTID index) const { return _vector[index]->first; }
    inline reference operator [](VECTID index) { return _vector[index]->first; }
    inline cVectUnique<T,Traits>& operator =(const cVectUnique<T,Traits>& right) { Clear(); unsigned int svar=right._vector.size(); for(unsigned int i=0; i<svar; ++i) AddVect(right._vector[i]->first); return *this; }
  
  protected:
#pragma warning(push)
#pragma warning(disable:4251)
    std::vector<VECTHOLD*> _vector; //These have to be pointers because otherwise the vector moves the memory around and the pointers lose coherence
#pragma warning(pop)
  };
}

#endif