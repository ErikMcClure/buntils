// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_FX_ENGINE_H__BSS__
#define __C_FX_ENGINE_H__BSS__

#include "cAnimation.h"
#include "variant.h"
#include "bss_alloc_block.h"
#include "cLinkedArray.h"

namespace bss_util {
  // Holds an object instantiation and all the anistates that belong to it.
  template<typename OBJ>
  class cFXObject : public cAniState
  {
  public:
    cFXObject() : cAniState(0) {}
    virtual ~cFXObject() {}
    virtual bool Interpolate(double delta)
    {
      _time += delta;
      bool finish = true;
      for(auto p : _anistates)
        finish = p->Interpolate(delta) && finish;
      return finish;
    }
    virtual void Reset()
    {
      for(auto p : _anistates)
        p.first->Reset();
      cAniState::Reset();
    }
    inline double GetTime() const { return _time; }
    inline void SetTime(double time)
    {
      if(time < _time)
        Reset();
      Interpolate(_time - time);
    }
    OBJ& GetObject() { return _obj; }
    void AddAni(cAniState* ani) { _anistates.Add(ani); }

    template<typename ALLOCSTATE>
    static void DestroyFXObj(cFXObject* o, ALLOCSTATE& alloc)
    {
      for(auto p : o->_anistates)
      {
        p->~cAniState();
        alloc.deallocate(p);
      }
      o->~cFXObject();
    }

  protected:
    OBJ _obj;
    cDynArray<cAniState*> _anistates;
  };

  // Keeps track of persistant objects outside of their parent FX containers.
  template<typename OBJ, typename STATE, typename ALLOCOBJ = BlockPolicy<cFXObject<OBJ>>, typename ALLOCSTATE = BlockPolicy<STATE>>
  class cFXManager
  {
  public:
    void Interpolate(double delta)
    {
      InterpolateObjects(delta, _objs, _allocobj, _allocstate);
    }

    static void InterpolateObjects(double delta, cLinkedArray<cFXObject<OBJ>*>& objs, ALLOCOBJ& allocobj, ALLOCSTATE& allocstate)
    {
      for(size_t i = objs.Front(); i != -1; )
      {
        size_t hold = i;
        objs.Next(i);
        if(objs[hold]->Interpolate(delta))
        {
          cFXObject<OBJ>::DestroyFXObj(objs[hold], allocstate);
          allocobj.deallocate(objs[hold]);
          objs.Remove(hold);
        }
      }
    }

    ALLOCOBJ& GetAllocObj() { return _allocobj; }
    ALLOCSTATE& GetAllocState() { return _allocstate; }

  protected:
    ALLOCOBJ _allocobj;
    ALLOCSTATE _allocstate;
    cLinkedArray<cFXObject<OBJ>*> _objs; // A linked array is used to keep track of the objects in O(1) time.
  };

  // DEF is a variant of object definitions (can be more than the objects, because multiple definitions can spawn the same object)
  // OBJ is a variant of all instantiated objects
  // Use CREATE to spawn an object from a definition
  // Ani is a variant of all possible animations used
  // MAP maps from all animations to all object types and is used to create the delegate needed to create the state object.
  // TODO: Use an actual linked list object spawned via a fixed-size allocator for animation objects to avoid memory allocation
  template<typename DEF, typename OBJ, void CREATE(const DEF& def, OBJ& obj), typename ANI, typename STATE, void MAP(const ANI& ani, STATE& state, OBJ& obj), typename ALLOCOBJ = BlockPolicy<OBJ>, typename ALLOCSTATE = BlockPolicy<STATE>>
  class cFXAni : public cAniBase
  {
  public:
    typedef cFXManager<OBJ, STATE, ALLOCOBJ, ALLOCSTATE> FXMANAGER;

    cFXAni(FXMANAGER* manager) : _manager(manager) {}
    template<typename T, typename D, ARRAY_TYPE ArrayType = CARRAY_SAFE>
    size_t AddAnimation(const cAniFrame<T, D>* src, unsigned int len, typename cAnimation<T, D>::FUNC f = 0)
    { 
      return _animations.AddConstruct(src, len, f);
    }
    template<typename T>
    size_t AddAnimation(T && ani)
    {
      return _animations.Add(std::forward<T>(ani));
    }
    template<typename T>
    size_t AddDef(T && def, double time = 0.0, bool persist = false)
    {
      return _defs.AddConstruct(std::forward<T>(def), time, persist);
    }
    virtual unsigned int GetSize() const { return _defs.Length(); }
    virtual const void* GetArray() const { return _defs.begin(); }
    virtual void* GetFunc() const { return &_create; }
    virtual cAniBase* Clone() const { return new cFXAni(*this); }
    FXMANAGER* GetManager() const { return _manager; }

    struct Def
    {
      Def(const DEF& d, double t, bool p) : def(d), time(t), persist(p) {}
      Def(DEF&& d, double t, bool p) : def(std::move(d)), time(t), persist(p) {}
      DEF def;
      double time;
      bool persist; // if true, the object being spawned will not go away until it's own animation has completed.
    };

    typedef DEF DEF_T;
    typedef OBJ OBJ_T;

  protected:
    static cFXObject<OBJ>* _create(cAniBase* p, size_t def, double time)
    {
      cFXObject<OBJ>* r = _manager->GetObjAlloc().allocate(1);
      new (r) cFXObject<OBJ>(); // since OBJ is a variant it doesn't matter that it gets instantiated here
      CREATE(_defs[def], r->GetObject());

      std::pair<size_t, size_t>* p = std::lower_bound<std::pair<size_t, size_t>*, size_t>(_mapping.begin(), _mapping.end(), def);
      while(p != _mapping.end() && p->first == def)
      {
        STATE* state = _manager->GetAllocState()->allocate(1);
        new(state) STATE();
        MAP(_animations[p->second], state, r->GetObject());
        r->AddAni(&state->Convert<cAniState>());
      }

      r->Interpolate(def.time - time);
      return r;
    }

    cDynArray<Def, size_t, CARRAY_SAFE> _defs;
    cDynArray<ANI, size_t, CARRAY_SAFE> _animations;
    cArraySort<std::pair<size_t, size_t>, CompTFirst<std::pair<size_t, size_t>, CompT<size_t>>> _mapping; // Stores Obj ID, Ani ID, sorted by Obj ID.
    FXMANAGER* _manager;
  };

  // If objects are not set to persist, they will be destroyed upon restarting.
  template<typename FXANI>
  class cFXAniState : public cAniState
  {
    typedef typename FXANI::Def Def;
    typedef typename FXANI::DEF_T DEF;
    typedef typename FXANI::OBJ_T OBJ;
    typedef cFXObject<OBJ>* (*FUNC)(cAniBase*, const Def&, double);

    cFXAniState(cAniBase* base) : cAniState(base) {}
    virtual ~cFXAniState() { Reset(); }
    virtual bool Interpolate(double delta)
    {
      _time += delta;
      Def* def = (Def*)_ani->GetArray();
      while(_cur < _ani->GetSize() && def[_cur].second <= _time)
      {
        cFXObject<OBJ>* obj = (*(FUNC)_ani->GetFunc())(_ani, _cur++, time);
        if(obj)
          _objs.Add(obj);
      }

      FXANI::FXMANAGER::InterpolateObjects(delta, _objs, ((FXANI*)_ani)->GetManager()->GetAllocObj, ((FXANI*)_ani)->GetManager()->GetAllocState);
    }

    virtual void Reset()
    {
      for(cFXObject<OBJ>* p : _objs)
      {
        cFXObject<OBJ>::DestroyFXObj(p, ((FXANI*)_ani)->GetManager()->GetAllocState);
        ((FXANI*)_ani)->GetManager()->GetAllocObj.deallocate(p);
      }
      cAniState::Reset();
    }

  protected:
    cLinkedArray<cFXObject<OBJ>*> _objs;
  };
}

#endif