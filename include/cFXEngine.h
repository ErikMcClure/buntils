// Copyright ©2015 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_FX_ENGINE_H__BSS__
#define __C_FX_ENGINE_H__BSS__

#include "cAnimation.h"
#include "variant.h"

namespace bss_util {
  template<typename DEF, typename OBJ>
  class cFXAni : public cAniBase
  {
    struct Def
    {
      cAniBase* ani;
      double time;
      size_t obj;
    };
  public:
    size_t CloneAnimation(size_t obj, double time, cAniBase* src) { Def d = { src->Clone(), time, obj }; return _animations.Add(d); }
    template<typename T, typename D, ARRAY_TYPE ArrayType = CARRAY_SAFE>
    size_t AddAnimation(size_t obj, double time, const cAniFrame<T, D>* src, unsigned int len, typename cAnimation<T, D>::FUNC f = 0)
    { 
      Def d = { new cAnimation<T, D, ArrayType>(src, len, f), time, obj };
      return _animations.Add(d);
    }
    template<typename... A>
    size_t AddFX(size_t obj, double time)
    {
      Def d = { new cFXAni<A...>(), time, obj };
      return _animations.Add(d);
    }


    cDynArray<DEF, size_t, CARRAY_SAFE> _defs;
    cDynArray<Def> _animations;
  };

  class cFXObjectBase
  {
  public:
    cFXObjectBase() : _time(0.0) {}
    virtual ~cFXObjectBase()
    { 
      Reset();
      for(auto& p : _anistates)
        p.first->~cAniState();
    }
    virtual void Interpolate(double delta)
    {
      _time += delta;
      for(auto& p : _anistates)
      {
        if(p.second <= _time)
          p.first->Interpolate(delta); // TODO: put in a check here so the initial call compensates for a delta that didn't start at the beginning of this frame
      }
    }
    virtual void Reset()
    {
      for(auto& p : _anistates)
      {
        if(p.second <= _time)
          p.first->Reset();
      }
      _time = 0.0;
    }
    inline double GetTime() const { return _time; }
    inline void SetTime(double time)
    { 
      if(time < _time)
        Reset();
      _time = time;
    }

  protected:
    cArraySort<std::pair<cAniState*, double>> _anistates;
    double _time;
  };

  template<typename T>
  class cFXObject : public cFXObjectBase
  {
    cFXObject() {}
    T _obj;
  };

  template<typename VAR, void (*INIT)(const VAR&), void(*DEINIT)(const VAR&)>
  class cFXState : public cFXStateBase
  {
  public:
    cFXState() : _init(0) {}
    ~cFXState()
    {
    }
    virtual void Interpolate(double delta)
    {
      double t = _time += delta;
      while(_objs[_init].second <= t)
        INIT(_objs[_init++].first);
      cFXStateBase::Interpolate(delta);
    }
    virtual void Reset()
    {
      // Only deinit objects that have been initialized
      for(unsigned int i = 0; i < _init; ++i)
        DEINIT(_objs[i].first);
      _init = 0;
      cFXStateBase::Reset();
    }

    cDynArray<std::pair<VAR, double>, size_t, CARRAY_SAFE> _objs;
    std::unique_ptr<char[]> _anistore; //Used as a memory dump to store the animation states - actual pointers are stored in _anistates
    unsigned int _init; // Tracks how many objects we've initialized
  };
}

#endif