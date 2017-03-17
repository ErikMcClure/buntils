// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __DATA_H__BSS__
#define __DATA_H__BSS__

#include "cDynArray.h"

namespace bss_util {
  struct DataListener
  {
    virtual void OnPost(const T&) = 0;
    virtual void OnPut(ITER src) = 0;
    virtual void OnPatch(ITER index, const T& data) = 0;
    virtual void OnDelete(ITER index) = 0;
  };

  template<class T>
  struct Datum
  {
    const T& Get();
    void Put(const T& v);

  protected:
    cDynArray<DataListener*> _listeners;
  };

  template<class T>
  struct DataObject
  {
    template<class U>
    const Datum<U>& Get(const char* id);
    void Put(const T&);
    template<class U>
    void Patch(const char* id, const U& data);

  protected:
    cDynArray<DataListener*> _listeners;
  };

  template<class T, class ITER>
  struct DataArray
  {
    ITER Get(size_t offset = 0);
    void Post(const T&);
    void Put(ITER src);
    void Patch(ITER index, const T& data);
    void Delete(ITER index);

  protected:
    cDynArray<DataListener*> _listeners;
  };
}


#endif