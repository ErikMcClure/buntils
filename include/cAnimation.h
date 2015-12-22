// Copyright ©2015 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_ANIMATION_H__BSS__
#define __C_ANIMATION_H__BSS__

#include "cArraySort.h"
#include "cRefCounter.h"
#include "delegate.h"
#include "cPriorityQueue.h"

namespace bss_util {
  // Abstract base animation class
  class BSS_COMPILER_DLLEXPORT cAniBase : public cRefCounter
  {
  public:
    cAniBase(cAniBase&& mov) : _typesize(mov._typesize), _length(mov._length), _calc(mov._calc), _loop(mov._loop) { Grab(); }
    cAniBase(size_t typesize) : _typesize(typesize), _length(0.0), _calc(0.0), _loop(-1.0) { Grab(); }
    virtual ~cAniBase() {}
    inline void SetLength(double length = 0.0) { _length = length; }
    inline double GetLength() const { return _length == 0.0 ? _calc : _length; }
    inline void SetLoop(double loop = 0.0) { _loop = loop; }
    inline double GetLoop() const { return _loop; }
    inline size_t SizeOf() const { return _typesize; }
    virtual unsigned int GetCapacity() const = 0;
    virtual const void* GetArray() const = 0;
    virtual void* GetFunc() const = 0;

    cAniBase& operator=(cAniBase&& mov)
    {
      _length = mov._length;
      _calc = mov._calc;
      _loop = mov._loop;
      return *this;
    }

  protected:
    double _length;
    double _calc;
    const size_t _typesize;
    double _loop;
  };

  // Represents a single animation frame
  template<typename T, typename D>
  struct cAniFrame
  {
    double time;
    T value;
    D data; // Additional data for animation specific operations, like interpolation
  };

  template<typename T>
  struct cAniFrame<T, void>
  {
    double time;
    T value;
  };

  // Animation class that stores an animation for a specific type.
  template<typename T, typename D = void, ARRAY_TYPE ArrayType = CARRAY_SAFE, typename Alloc = StaticAllocPolicy<cAniFrame<T, D>>>
  class BSS_COMPILER_DLLEXPORT cAnimation : public cAniBase
  {
  public:
    typedef cAniFrame<T, D> FRAME;
    typedef T(BSS_FASTCALL *FUNC)(const FRAME*, unsigned int, unsigned int, double, const T&);

    explicit cAnimation(FUNC f = 0) : cAniBase(sizeof(T)), _f(f) { }
    cAnimation(cAnimation&& mov) : cAniBase(std::move(mov)), _frames(std::move(mov._frames)), _f(std::move(mov._f)) {}
    cAnimation(const FRAME* src, unsigned int len, FUNC f = 0) : cAniBase(sizeof(T)), _f(f) { Set(src, len); }
    virtual ~cAnimation() {}
    inline unsigned int Add(const FRAME& frame) { unsigned int r = _frames.Insert(frame); _calc = _frames.Back().time; return r; }
    inline unsigned int Add(double time, const T& value) { FRAME f = { time, value }; return Add(f); }
    inline void Set(const FRAME* src, unsigned int len) { _frames.SetArray(src, len); _calc = _frames.Back().time; }
    inline const FRAME& Get(unsigned int index) const { return _frames[index]; }
    inline bool Remove(unsigned int index) { return _frames.Remove(index); }
    virtual unsigned int GetCapacity() const { return _frames.Length(); }
    virtual const void* GetArray() const { return _frames.begin(); }
    void SetFunc(FUNC f) { _f = f; }
    virtual void* GetFunc() const { return (void*)_f; }

    cAnimation& operator=(cAnimation&& mov)
    {
      cAniBase::operator=(std::move(mov));
      _frames = std::move(_frames);
      _f = std::move(_f);
      return *this;
    }

    BSS_FORCEINLINE static char CompAniFrame(const FRAME& left, const FRAME& right) { return SGNCOMPARE(left.time, right.time); }

  protected:
    cArraySort<FRAME, CompAniFrame, unsigned int, ArrayType, Alloc> _frames;
    FUNC _f;
  };

  template<typename T, ARRAY_TYPE ArrayType = CARRAY_SAFE, typename Alloc = StaticAllocPolicy<cAniFrame<T, double>>>
  class BSS_COMPILER_DLLEXPORT cAnimationInterval : public cAnimation<T, double, ArrayType, Alloc>
  {
    typedef cAnimation<T, double, ArrayType, Alloc> BASE;
    using BASE::_calc;
    using BASE::_frames;
  public:
    explicit cAnimationInterval(typename BASE::FUNC f = 0) : BASE(f) { }
    cAnimationInterval(cAnimationInterval&& mov) : BASE(std::move(mov)) {}
    cAnimationInterval(const typename BASE::FRAME* src, unsigned int len, typename BASE::FUNC f = 0) : BASE(src, len, f) { _recalclength(); }
    virtual ~cAnimationInterval() {}
    inline unsigned int Add(const BASE::FRAME& frame) { unsigned int r = BASE::Add(frame); _checkindex(r); return r; }
    inline unsigned int Add(double time, const T& value, const double& data) { BASE::FRAME f = { time, value, data }; return Add(f); }
    inline void Set(const typename BASE::FRAME* src, unsigned int len) { BASE::Set(src, len); _recalclength(); }
    inline bool Remove(unsigned int index) { bool r = BASE::Remove(index); if(r) _recalclength(); return r; }

    cAnimationInterval& operator=(cAnimationInterval&& mov) { BASE::operator=(std::move(mov)); return *this; }

  protected:
    void _recalclength()
    {
      _calc = 0.0;
      for(unsigned int i = 0; i < _frames.Length(); ++i)
        _checkindex(i);
    }
    void _checkindex(unsigned int i)
    {
      double t = _frames[i].time + _frames[i].data;
      if(t > _calc)
        _calc = t;
    }
  };

  // Animation state object. References an animation that inherits cAniBase
  struct BSS_COMPILER_DLLEXPORT cAniState
  {
    cAniState(const cAniState& copy) : _ani(copy._ani), _cur(copy._cur), _time(copy._time) { if(_ani) _ani->Grab(); }
    cAniState(cAniState&& mov) : _ani(mov._ani), _cur(mov._cur), _time(mov._time) { mov._ani = 0; }
    explicit cAniState(cAniBase* p) : _ani(p), _cur(0), _time(0.0) { if(_ani) _ani->Grab(); }
    virtual ~cAniState() { if(_ani) _ani->Drop(); }
    virtual bool Interpolate(double delta)=0;
    virtual void Reset() { _cur = 0; _time = 0.0; }
    inline double GetTime() const { return _time; }
    inline void SetTime(double time)
    {
      if(_time > time) // If we are rewinding, we have to reset _cur to 0 to preserve a valid state.
        Reset();
      _time = time; // Otherwise it is legal for us to skip ahead an arbitrary length of time.
    }

    cAniState& operator=(const cAniState& copy)
    {
      if(_ani) _ani->Drop();
      _ani = copy._ani;
      _cur = copy._cur;
      _time = copy._time;
      if(_ani) _ani->Grab();
      return *this;
    }
    cAniState& operator=(cAniState&& mov)
    {
      if(_ani) _ani->Drop();
      _ani = mov._ani;
      _cur = mov._cur;
      _time = mov._time;
      mov._ani = 0;
      return *this;
    }

  protected:
    cAniBase* _ani;
    unsigned int _cur;
    double _time;
  };

  template<typename T, typename D = void, typename REF = T, typename AUX = void>
  struct BSS_COMPILER_DLLEXPORT cAniStateDiscrete : public cAniState
  {
    using cAniState::_ani;
    using cAniState::_cur;
    using cAniState::_time;
    cAniStateDiscrete(const cAniStateDiscrete& copy) : cAniState(copy), _set(copy._set) { }
    cAniStateDiscrete(cAniStateDiscrete&& mov) : cAniState(std::move(mov)), _set(std::move(mov._set)) { }
    cAniStateDiscrete(cAniBase* p, delegate<AUX, REF> d) : cAniState(p), _set(d) { assert(_ani->SizeOf() == sizeof(T)); }
    template<ARRAY_TYPE ArrayType, typename Alloc>
    cAniStateDiscrete(cAnimation<T, D, ArrayType, Alloc>* p, delegate<AUX, REF> d) : cAniState(p), _set(d) {}
    cAniStateDiscrete() : _set(0,0) {}

    cAniStateDiscrete& operator=(const cAniStateDiscrete& copy) { cAniState::operator=(copy); _set = copy._set; return *this; }
    cAniStateDiscrete& operator=(cAniStateDiscrete&& mov) { cAniState::operator=(std::move(mov)); _set = std::move(mov._set); return *this; }

    virtual bool Interpolate(double delta)
    {
      _time += delta;
      auto svar = _ani->GetCapacity();
      auto v = (typename cAnimation<T, D>::FRAME*)_ani->GetArray();
      double loop = _ani->GetLoop();
      double length = _ani->GetLength();
      while(_cur < svar && v[_cur].time <= _time)
        _set(v[_cur++].value); // We call all the discrete values because many discrete values are interdependent on each other.
      if(_time >= length && loop >= 0.0) // We do the loop check down here because we need to finish calling all the discrete values on the end of the animation before looping
      {
        _cur = 0; // We can't call Reset() here because _time contains information we need.
        _time = fmod(_time - length, length - loop);// + loop; // instead of adding loop here we just pass loop into Interpolate, which adds it.
        return Interpolate(loop); // because we used fmod, it is impossible for this to result in another loop.
      }
      return _time < length;
    }

  protected:
    delegate<AUX, REF> _set;
  };

  template<typename T, typename D = void, typename REF = T, typename AUX = void>
  struct BSS_COMPILER_DLLEXPORT cAniStateSmooth : public cAniStateDiscrete<T, D, REF, AUX>
  {
    typedef cAniStateDiscrete<T, D, REF, AUX> BASE;
    using BASE::_ani;
    using BASE::_cur;
    using BASE::_time;
    cAniStateSmooth(const cAniStateSmooth& copy) : BASE(copy), _init(copy._init) { }
    cAniStateSmooth(cAniStateSmooth&& mov) : BASE(std::move(mov)), _init(std::move(mov._init)) { }
    cAniStateSmooth(cAniBase* p, delegate<AUX, REF> d, T init = T()) : BASE(p, d), _init(init) { assert(_ani->GetFunc() != 0); }
    template<ARRAY_TYPE ArrayType, typename Alloc>
    cAniStateSmooth(cAnimation<T, D, ArrayType, Alloc>* p, delegate<AUX, REF> d, T init = T()) : BASE(p, d), _init(init) {}
    cAniStateSmooth() {}

    cAniStateSmooth& operator=(const cAniStateSmooth& copy) { BASE::operator=(copy); _init = copy._init; return *this; }
    cAniStateSmooth& operator=(cAniStateSmooth&& mov) { BASE::operator=(std::move(mov)); _init = std::move(mov._init); return *this; }

    virtual bool Interpolate(double delta)
    {
      _time += delta;
      auto svar = _ani->GetCapacity();
      auto v = (typename cAnimation<T, D>::FRAME*)_ani->GetArray();
      auto f = (typename cAnimation<T, D>::FUNC)_ani->GetFunc();
      double loop = _ani->GetLoop();
      double length = _ani->GetLength();
      if(_time >= length && loop >= 0.0)
      {
        _cur = 0;
        _time = fmod(_time - length, length - loop) + loop;
      }

      while(_cur<svar && v[_cur].time <= _time) ++_cur;
      if(_cur >= svar)
      { //Resolve the animation, but only if there was more than 1 keyframe, otherwise we'll break it.
        if(svar>1)
          BASE::_set(f(v, svar, svar - 1, 1.0, _init));
      }
      else
      {
        double hold = !_cur?0.0:v[_cur - 1].time;
        BASE::_set(f(v, svar, _cur, (_time - hold) / (v[_cur].time - hold), _init));
      }
      return _time < length;
    }

    static inline T BSS_FASTCALL NoInterpolate(const typename cAnimation<T, D>::FRAME* v, unsigned int s, unsigned int cur, double t, const T& init) { unsigned int i = cur - (t != 1.0); return (i<0) ? init : v[i].value; }
    static inline T BSS_FASTCALL NoInterpolateRel(const typename cAnimation<T, D>::FRAME* v, unsigned int s, unsigned int cur, double t, const T& init) { assert(cur > 0); return init + v[cur - (t != 1.0)].value; }
    static inline T BSS_FASTCALL LerpInterpolate(const typename cAnimation<T, D>::FRAME* v, unsigned int s, unsigned int cur, double t, const T& init) { return lerp<T>(!cur ? init : v[cur - 1].value, v[cur].value, t); }
    static inline T BSS_FASTCALL LerpInterpolateRel(const typename cAnimation<T, D>::FRAME* v, unsigned int s, unsigned int cur, double t, const T& init) { assert(cur > 0); return init + lerp<T>(v[cur - 1].value, v[cur].value, t); }
    static inline T BSS_FASTCALL CubicInterpolateRel(const typename cAnimation<T, D>::FRAME* v, unsigned int s, unsigned int cur, double t, const T& init) { assert(cur > 0); return init + CubicBSpline<T>(t, v[cur - 1 - (cur != 1)].value, v[cur - 1].value, v[cur].value, v[cur + ((cur + 1) != v.Capacity())].value); }
    //static inline T BSS_FASTCALL QuadInterpolate(const PAIR* v, unsigned int s, unsigned int cur, double t, const T& init) {}
    //typedef T(BSS_FASTCALL *TIME_FNTYPE)(const TVT_ARRAY_T& a, IDTYPE i, double t); // VC++ 2010 can't handle this being in the template itself
    //template<TIME_FNTYPE FN, double(*TIME)(DATA&)>
    //static inline T BSS_FASTCALL TimeInterpolate(const TVT_ARRAY_T& a, IDTYPE i, double t) { return (*FN)(a, i, UniformQuadraticBSpline<double, double>(t, (*TIME)(a[i - 1 - (i>2)]), (*TIME)(a[i - 1]), (*TIME)(a[i]))); }

  protected:
    T _init; // This is referenced by an index of -1
  };
  
  template<typename T, typename AUX, typename REF = T, typename QUEUEALLOC = StaticAllocPolicy<std::pair<double, AUX>>>
  struct BSS_COMPILER_DLLEXPORT cAniStateInterval : public cAniStateDiscrete<T, double, REF, AUX>
  {
    typedef cAniStateDiscrete<T, double, REF, AUX> BASE;
    using BASE::_ani;
    using BASE::_cur;
    using BASE::_time;
    cAniStateInterval(cAniBase* p, delegate<AUX, REF> d, delegate<void, AUX> rm) : BASE(p, d), _remove(rm) { assert(_ani->SizeOf() == sizeof(T)); }
    template<ARRAY_TYPE ArrayType, typename Alloc>
    cAniStateInterval(cAnimationInterval<T, ArrayType, Alloc>* p, delegate<AUX, REF> d, delegate<void, AUX> rm) : BASE(p, d), _remove(rm) {}
    inline ~cAniStateInterval() { _clearqueue(); }
    virtual void Reset() { _clearqueue(); BASE::Reset(); }
    virtual bool Interpolate(double delta)
    {
      _time += delta;
      auto svar = _ani->GetCapacity();
      auto v = (typename cAnimation<T, double>::FRAME*)_ani->GetArray();
      double loop = _ani->GetLoop();
      double length = _ani->GetLength();

      while(_cur < svar && v[_cur].time <= _time)
        _addtoqueue(v[_cur++]); // We call all the discrete values because many discrete values are interdependent on each other.
      if(_time >= length && loop >= 0.0) // We do the loop check down here because we need to finish calling all the discrete values on the end of the animation before looping
      {
        _cur = 0; // We can't call Reset() here because _time contains information we need.
        _clearqueue(); // Ensure the queue is cleared before looping or Bad Things will happen.
        _time = fmod(_time - length, length - loop);// + loop; // instead of adding loop here we just pass loop into Interpolate, which adds it.
        return Interpolate(loop); // because we used fmod, it is impossible for this to result in another loop.
      }
      while(!_queue.Empty() && _queue.Peek().first <= _time)
        _remove(_queue.Pop().second);
      return _time < length;
    }
  protected:
    inline void _addtoqueue(const typename cAnimation<T, double>::FRAME& v) { _queue.Push(v.data, BASE::_set(v.value)); }

    void _clearqueue()
    {
      while(!_queue.Empty()) // Correctly remove everything currently on the queue
        _remove(_queue.Pop().second);
    }
    delegate<void, AUX> _remove; //delegate for removal
    cPriorityQueue<double, AUX, CompT<double>, unsigned int, CARRAY_SIMPLE, QUEUEALLOC> _queue;
  };
}

#endif