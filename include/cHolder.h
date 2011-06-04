// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_HOLDER_H__
#define __C_HOLDER_H__

#define COP(cls) cHolder<cls>::_holdobject //shorthand for easy access in multiple inheritance cases
#define COR(cls) (*cHolder<cls>::_holdobject) //shorthand for easy access in multiple inheritance cases

namespace bss_util {
  /* A class to provide the common functionality of storing a pointer to an object of a certain class */
  template<class T>
  class __declspec(dllexport) cHolder
  {
  public:
    /* Constructor - takes a pointer to the object */
    inline cHolder<T>(T* object = 0) { _holdobject = object; }
    inline ~cHolder<T>() {} //I think this is required because destructors can't be generated for a template class (which is why errors occur otherwise)

  protected:
    T* _holdobject; //Stores a pointer to the object
  };
}

#endif