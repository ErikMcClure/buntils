// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_FX_ENGINE_H__BSS__
#define __C_FX_ENGINE_H__BSS__

#include "cAnimation.h"
#include "variant.h"
#include "bss_alloc_block.h"
#include "cLinkedArray.h"

namespace bss_util {
  // Holds an object instantiation and all the anistates that belong to it.
  template<typename FXMANAGER>
  class cFXObject : public cAniState
  {
    typedef typename FXMANAGER::OBJ_T OBJ;
    typedef typename FXMANAGER::ANI_T ANI;
    typedef typename FXMANAGER::STATE_T STATE;

    struct AniRef
    {
      ANI* ani;
      double delay;
      cAniState* state;
      STATE* var;
    };

  public:
    cFXObject(FXMANAGER* manager) : cAniState(0), _anistates(0), _length(0), _manager(manager) {}
    virtual ~cFXObject() { Reset(); }

    virtual bool Interpolate(double delta)
    {
      _time += delta;
      bool finish = (_length == 0.0);
      for(auto& p : _anistates)
      {
        if(_time >= p.delay)
        {
          if(p.state)
            finish = p.state->Interpolate(delta) && finish;
          else
          {
            p.var = _manager->SpawnState(_obj, *p.ani);
            p.state = p.var->template convertP<cAniState>();
            p.state->Interpolate(_time - p.delay);
          }
        }
        else // if we haven't even created the state it obviously isn't finished
          finish = false;
      }
      return finish || (_length > 0 && _time >= _length);    
    }

    virtual void Reset()
    {
      for(auto& p : _anistates)
      {
        if(p.state != 0) // We HAVE to delete all of these even if they start at 0 because of the initialization value.
        {
          p.state->~cAniState();
          _manager->DestroyState(p.var);
        }
      }
      cAniState::Reset();
    }

    OBJ& GetObject() { return _obj; }
    inline void SetLength(double length) { _length = length; }
    inline double GetLength() const { return _length; }
    inline double GetTime() const { return _time; }

    inline void SetTime(double time)
    {
      if(time < _time)
        Reset();
      Interpolate(_time - time);
    }

    void AddAni(ANI* ani, double delay)
    { 
      assert(ani != 0);
      AniRef ref = { ani, delay, 0 };
      _anistates.Add(ref);
    }

  protected:
    OBJ _obj;
    cDynArray<AniRef> _anistates; // TODO: make this into a linked list so you can use a fixed-allocator for it.
    FXMANAGER* _manager;
    double _length;
  };

  // Keeps track of persistant objects outside of their parent FX containers.
  template<typename OBJ, typename ANI, typename STATE, void MAP(ANI& ani, STATE& state, OBJ& obj)>
  class cFXManager
  {
    typedef BlockPolicy<cFXObject<cFXManager>> ALLOCOBJ;
    typedef BlockPolicy<STATE> ALLOCSTATE;

  public:
    void Interpolate(double delta)
    {
      InterpolateObjects(delta, _objs, _allocobj, _allocstate);
    }

    static bool InterpolateObjects(double delta, cLinkedArray<cFXObject<cFXManager>*>& objs, ALLOCOBJ& allocobj, ALLOCSTATE& allocstate)
    {
      bool finish = true;
      for(size_t i = objs.Front(); i != -1; )
      {
        size_t hold = i;
        objs.Next(i);
        if(objs[hold]->Interpolate(delta))
        {
          objs[hold]->~cFXObject<cFXManager>();
          allocobj.deallocate(objs[hold]);
          objs.Remove(hold);
        }
        else
          finish = false;
      }
      return finish;
    }
    STATE* SpawnState(OBJ& obj, ANI& ani)
    {
      STATE* state = _allocstate.allocate(1);
      new(state) STATE();
      MAP(ani, *state, obj);
      return state;
    }
    void DestroyState(STATE* s)
    {
      _allocstate.deallocate((STATE*)s); // todo: is there a typesafe way to do this?
    }
    ALLOCOBJ& GetAllocObj() { return _allocobj; }
    ALLOCSTATE& GetAllocState() { return _allocstate; }

    typedef OBJ OBJ_T;
    typedef ANI ANI_T;
    typedef STATE STATE_T;

  protected:
    ALLOCOBJ _allocobj;
    ALLOCSTATE _allocstate;
    cLinkedArray<cFXObject<cFXManager>*> _objs; // A linked array is used to keep track of the objects in O(1) time.
  };

  // DEF is a variant of object definitions (can be more than the objects, because multiple definitions can spawn the same object)
  // OBJ is a variant of all instantiated objects
  // Use CREATE to spawn an object from a definition
  // Ani is a variant of all possible animations used
  // MAP maps from all animations to all object types and is used to create the delegate needed to create the state object.
  // TODO: Use an actual linked list object spawned via a fixed-size allocator for animation objects to avoid memory allocation
  template<typename DEF, typename OBJ, double CREATE(const DEF& def, OBJ& obj), typename ANI, typename STATE, void MAP(ANI& ani, STATE& state, OBJ& obj)>
  class cFXAni : public cAniBase
  {
  public:
    typedef cFXManager<OBJ, ANI, STATE, MAP> FXMANAGER;

    cFXAni(FXMANAGER* manager) : _manager(manager), cAniBase(sizeof(Def)) {}
    template<typename T, typename D, ARRAY_TYPE ArrayType = CARRAY_SAFE>
    size_t AddAnimation(const cAniFrame<T, D>* src, uint32_t len, typename cAnimation<T, D>::FUNC f = 0)
    {
      return _animations.AddConstruct(cAnimation<T, D, ArrayType>(src, len, f));
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
    void AddMapping(size_t def, size_t ani, double delay = 0.0)
    {
      AniMap map = { def, ani, delay };
      _mapping.Insert(map);
    }
    virtual uint32_t GetSize() const { return _defs.Length(); }
    virtual const void* GetArray() const { return _defs.begin(); }
    virtual void* GetFunc() const { return 0; }
    virtual cAniBase* Clone() const { return new cFXAni(*this); }
    FXMANAGER* GetManager() const { return _manager; }

    cFXObject<FXMANAGER>* CreateFXObj(size_t def, double time)
    {
      Def& d = _defs[def];
      cFXObject<FXMANAGER>* r = _manager->GetAllocObj().allocate(1);
      new (r) cFXObject<FXMANAGER>(_manager); // since OBJ is a variant it doesn't matter that it gets instantiated here
      r->SetLength(CREATE(d.def, r->GetObject()));

      AniMap* p = std::lower_bound<AniMap*, size_t>(_mapping.begin(), _mapping.end(), def, &AniMap::CompL);
      while(p != _mapping.end() && p->objID == def)
      {
        r->AddAni(&_animations[p->aniID], p->delay);
        ++p;
      }

      r->Interpolate(d.time - time);
      return r;
    }

    struct Def
    {
      Def() {}
      Def(const DEF& d, double t, bool p) : def(d), time(t), persist(p) {}
      Def(DEF&& d, double t, bool p) : def(std::move(d)), time(t), persist(p) {}
      DEF def;
      double time;
      bool persist; // if true, the object being spawned will not go away until it's own animation has completed.
    };

    typedef DEF DEF_T;
    typedef OBJ OBJ_T;

  protected:

    struct AniMap
    {
      size_t objID;
      size_t aniID;
      double delay;

      static BSS_FORCEINLINE char Comp(const AniMap& left, const AniMap& right) { return SGNCOMPARE(left.objID, right.objID); }
      static BSS_FORCEINLINE bool CompL(const AniMap& left, size_t right) { return left.objID < right; }
    };

    cDynArray<Def, size_t, CARRAY_SAFE> _defs;
    cDynArray<ANI, size_t, CARRAY_SAFE> _animations;
    cArraySort<AniMap, &AniMap::Comp> _mapping;
    FXMANAGER* _manager;
  };

  // If objects are not set to persist, they will be destroyed upon restarting.
  template<typename FXANI>
  class cFXAniState : public cAniState
  {
  public:
    typedef typename FXANI::Def Def;
    typedef typename FXANI::DEF_T DEF;
    typedef typename FXANI::OBJ_T OBJ;
    typedef cFXObject<typename FXANI::FXMANAGER> FXOBJ;

    cFXAniState(cAniBase* base) : cAniState(base) {}
    virtual ~cFXAniState() { Reset(); }
    virtual bool Interpolate(double delta)
    {
      FXANI* fxani = static_cast<FXANI*>(_ani);
      _time += delta;
      Def* def = (Def*)_ani->GetArray();
      while(_cur < _ani->GetSize() && def[_cur].time <= _time)
      {
        FXOBJ* obj = fxani->CreateFXObj(_cur++, _time);
        if(obj)
          _objs.Add(obj);
      }

      return _cur >= _ani->GetSize() && FXANI::FXMANAGER::InterpolateObjects(delta, _objs, fxani->GetManager()->GetAllocObj(), fxani->GetManager()->GetAllocState());
    }

    virtual void Reset()
    {
      for(FXOBJ* p : _objs)
      {
        p->~FXOBJ();
        ((FXANI*)_ani)->GetManager()->GetAllocObj().deallocate(p);
      }
      cAniState::Reset();
    }

  protected:
    cLinkedArray<FXOBJ*> _objs;
  };
}

#endif