// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __FX_ENGINE_H__BSS__
#define __FX_ENGINE_H__BSS__

#include "Animation.h"
#include "Variant.h"
#include "BlockAlloc.h"
#include "LinkedArray.h"

namespace bss {
  // Holds an object instantiation and all the anistates that belong to it.
  template<typename FXMANAGER>
  class FXObject : public AniState
  {
    typedef typename FXMANAGER::OBJ_T OBJ;
    typedef typename FXMANAGER::ANI_T ANI;
    typedef typename FXMANAGER::STATE_T STATE;

    struct AniRef
    {
      ANI* ani;
      double delay;
      AniState* state;
      STATE* var;
    };

  public:
    FXObject(FXMANAGER* manager) : AniState(0), _anistates(0), _length(0), _manager(manager) {}
    virtual ~FXObject() { Reset(); }

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
            p.state = p.var->template convertP<AniState>();
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
          p.state->~AniState();
          _manager->DestroyState(p.var);
        }
      }
      AniState::Reset();
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
    DynArray<AniRef> _anistates; // TODO: make this into a linked list so you can use a fixed-allocator for it.
    FXMANAGER* _manager;
    double _length;
  };

  // Keeps track of persistant objects outside of their parent FX containers.
  template<typename OBJ, typename ANI, typename STATE, void MAP(ANI& ani, STATE& state, OBJ& obj)>
  class FXManager
  {
    typedef BlockPolicy<FXObject<FXManager>> ALLOCOBJ;
    typedef BlockPolicy<STATE> ALLOCSTATE;

  public:
    void Interpolate(double delta)
    {
      InterpolateObjects(delta, _objs, _allocobj, _allocstate);
    }

    static bool InterpolateObjects(double delta, LinkedArray<FXObject<FXManager>*>& objs, ALLOCOBJ& allocobj, ALLOCSTATE& allocstate)
    {
      bool finish = true;

      for(size_t i = objs.Front(); i != -1; )
      {
        size_t hold = i;
        objs.Next(i);

        if(objs[hold]->Interpolate(delta))
        {
          objs[hold]->~FXObject<FXManager>();
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
    LinkedArray<FXObject<FXManager>*> _objs; // A linked array is used to keep track of the objects in O(1) time.
  };

  // DEF is a Variant of object definitions (can be more than the objects, because multiple definitions can spawn the same object)
  // OBJ is a Variant of all instantiated objects
  // Use CREATE to spawn an object from a definition
  // Ani is a Variant of all possible animations used
  // MAP maps from all animations to all object types and is used to create the Delegate needed to create the state object.
  // TODO: Use an actual linked list object spawned via a fixed-size allocator for animation objects to avoid memory allocation
  template<typename DEF, typename OBJ, double CREATE(const DEF& def, OBJ& obj), typename ANI, typename STATE, void MAP(ANI& ani, STATE& state, OBJ& obj)>
  class FXAni : public AniBase
  {
  public:
    typedef FXManager<OBJ, ANI, STATE, MAP> FXMANAGER;

    FXAni(FXMANAGER* manager) : _manager(manager), AniBase(sizeof(Def)) {}
    template<typename T, typename D, ARRAY_TYPE ArrayType = ARRAY_SAFE>
    size_t AddAnimation(const AniFrame<T, D>* src, uint32_t len, typename Animation<T, D>::FUNC f = 0)
    {
      return _animations.AddConstruct(Animation<T, D, ArrayType>(src, len, f));
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
    virtual uint32_t GetSize() const { return (uint32_t)_defs.Length(); }
    virtual const void* GetArray() const { return _defs.begin(); }
    virtual void* GetFunc() const { return 0; }
    virtual AniBase* Clone() const { return new FXAni(*this); }
    FXMANAGER* GetManager() const { return _manager; }

    FXObject<FXMANAGER>* CreateFXObj(size_t def, double time)
    {
      Def& d = _defs[def];
      FXObject<FXMANAGER>* r = _manager->GetAllocObj().allocate(1);
      new (r) FXObject<FXMANAGER>(_manager); // since OBJ is a Variant it doesn't matter that it gets instantiated here
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

    DynArray<Def, size_t, ARRAY_SAFE> _defs;
    DynArray<ANI, size_t, ARRAY_SAFE> _animations;
    ArraySort<AniMap, &AniMap::Comp> _mapping;
    FXMANAGER* _manager;
  };

  // If objects are not set to persist, they will be destroyed upon restarting.
  template<typename FXANI>
  class FXAniState : public AniState
  {
  public:
    typedef typename FXANI::Def Def;
    typedef typename FXANI::DEF_T DEF;
    typedef typename FXANI::OBJ_T OBJ;
    typedef FXObject<typename FXANI::FXMANAGER> FXOBJ;

    FXAniState(AniBase* base) : AniState(base) {}
    virtual ~FXAniState() { Reset(); }
    virtual bool Interpolate(double delta)
    {
      FXANI* fxani = static_cast<FXANI*>(_ani);
      _time += delta;
      Def* def = (Def*)_ani->GetArray();

      while(_cur < _ani->GetSize() && def[_cur].time <= _time)
      {
        if(FXOBJ* obj = fxani->CreateFXObj(_cur++, _time))
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
      AniState::Reset();
    }

  protected:
    LinkedArray<FXOBJ*> _objs;
  };
}

#endif