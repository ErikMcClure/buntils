// Copyright �2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __ANIMATION_H__BSS__
#define __ANIMATION_H__BSS__

#include "ArraySort.h"
#include "RefCounter.h"
#include "Delegate.h"
#include "PriorityQueue.h"

namespace bss {
  // Abstract base animation class
  class BSS_COMPILER_DLLEXPORT AniBase : public RefCounter
  {
  public:
    AniBase(const AniBase& copy) : _length(copy._length), _calc(copy._calc), _typesize(copy._typesize), _loop(copy._loop) { Grab(); }
    AniBase(AniBase&& mov) : _length(mov._length), _calc(mov._calc), _typesize(mov._typesize), _loop(mov._loop) { Grab(); }
    AniBase(size_t typesize) : _length(-1.0), _calc(0.0), _typesize(typesize), _loop(-1.0) { Grab(); }
    virtual ~AniBase() {}
    inline void SetLength(double length = -1.0) { _length = length; }
    inline double GetLength() const { return _length < 0.0 ? _calc : _length; }
    inline void SetLoop(double loop = 0.0) { _loop = loop; }
    inline double GetLoop() const { return _loop; }
    inline size_t SizeOf() const { return _typesize; }
    virtual uint32_t GetSize() const = 0;
    virtual const void* GetArray() const = 0;
    virtual void* GetFunc() const = 0;
    virtual AniBase* Clone() const = 0;

    AniBase& operator=(AniBase&& mov)
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
  struct AniFrame
  {
    double time;
    T value;
    D data; // Additional data for animation specific operations, like interpolation
  };

  template<typename T>
  struct AniFrame<T, void>
  {
    double time;
    T value;
  };

  // Animation class that stores an animation for a specific type.
  template<typename T, typename D = void, ARRAY_TYPE ArrayType = ARRAY_SAFE, typename Alloc = StaticAllocPolicy<AniFrame<T, D>>>
  class BSS_COMPILER_DLLEXPORT Animation : public AniBase
  {
  public:
    typedef AniFrame<T, D> FRAME;
    typedef T(*FUNC)(const FRAME*, uint32_t, uint32_t, double, const T&);

    explicit Animation(FUNC f = 0) : AniBase(sizeof(T)), _f(f) {}
    Animation(Animation&& mov) : AniBase(std::move(mov)), _frames(std::move(mov._frames)), _f(std::move(mov._f)) {}
    Animation(const Animation& copy) : AniBase(copy), _frames(copy._frames), _f(copy._f) {}
    Animation(const FRAME* src, uint32_t len, FUNC f = 0) : AniBase(sizeof(T)), _f(f) { Set(src, len); }
    virtual ~Animation() {}
    inline uint32_t Add(const FRAME& frame) { uint32_t r = _frames.Insert(frame); _calc = _frames.Back().time; return r; }
    inline uint32_t Add(double time, const T& value) { FRAME f = { time, value }; return Add(f); }
    inline void Set(const FRAME* src, uint32_t len) { _frames = ArraySlice<const FRAME, uint32_t>(src, len); _calc = _frames.Back().time; }
    inline const FRAME& Get(uint32_t index) const { return _frames[index]; }
    inline bool Remove(uint32_t index) { return _frames.Remove(index); }
    virtual uint32_t GetSize() const { return _frames.Length(); }
    virtual const void* GetArray() const { return _frames.begin(); }
    void SetFunc(FUNC f) { _f = f; }
    virtual void* GetFunc() const { return (void*)_f; }
    virtual AniBase* Clone() const { return new Animation(*this); }

    Animation& operator=(Animation&& mov)
    {
      AniBase::operator=(std::move(mov));
      _frames = std::move(_frames);
      _f = std::move(_f);
      return *this;
    }

    BSS_FORCEINLINE static char CompAniFrame(const FRAME& left, const FRAME& right) { return SGNCOMPARE(left.time, right.time); }

  protected:
    ArraySort<FRAME, CompAniFrame, uint32_t, ArrayType, Alloc> _frames;
    FUNC _f;
  };

  template<typename T, ARRAY_TYPE ArrayType = ARRAY_SAFE, typename Alloc = StaticAllocPolicy<AniFrame<T, double>>>
  class BSS_COMPILER_DLLEXPORT AnimationInterval : public Animation<T, double, ArrayType, Alloc>
  {
    typedef Animation<T, double, ArrayType, Alloc> BASE;
    using BASE::_calc;
    using BASE::_frames;
  public:
    explicit AnimationInterval(typename BASE::FUNC f = 0) : BASE(f) {}
    AnimationInterval(const AnimationInterval& copy) : BASE(copy) {}
    AnimationInterval(AnimationInterval&& mov) : BASE(std::move(mov)) {}
    AnimationInterval(const typename BASE::FRAME* src, uint32_t len, typename BASE::FUNC f = 0) : BASE(src, len, f) { _recalcLength(); }
    virtual ~AnimationInterval() {}
    inline uint32_t Add(const typename BASE::FRAME& frame) { uint32_t r = BASE::Add(frame); _checkIndex(r); return r; }
    inline uint32_t Add(double time, const T& value, const double& data) { typename BASE::FRAME f = { time, value, data }; return Add(f); }
    inline void Set(const typename BASE::FRAME* src, uint32_t len) { BASE::Set(src, len); _recalcLength(); }
    inline bool Remove(uint32_t index) { bool r = BASE::Remove(index); if(r) _recalcLength(); return r; }
    virtual AniBase* Clone() const { return new AnimationInterval(*this); }

    AnimationInterval& operator=(AnimationInterval&& mov) { BASE::operator=(std::move(mov)); return *this; }

  protected:
    void _recalcLength()
    {
      _calc = 0.0;
      for(uint32_t i = 0; i < _frames.Length(); ++i)
        _checkIndex(i);
    }
    void _checkIndex(uint32_t i)
    {
      double t = _frames[i].time + _frames[i].data;
      if(t > _calc)
        _calc = t;
    }
  };

  // Animation state object. References an animation that inherits AniBase
  struct BSS_COMPILER_DLLEXPORT AniState
  {
    AniState(const AniState& copy) : _ani(copy._ani), _cur(copy._cur), _time(copy._time) { if(_ani) _ani->Grab(); }
    AniState(AniState&& mov) : _ani(mov._ani), _cur(mov._cur), _time(mov._time) { mov._ani = 0; }
    explicit AniState(AniBase* p) : _ani(p), _cur(0), _time(0.0) { if(_ani) _ani->Grab(); }
    virtual ~AniState() { if(_ani) _ani->Drop(); }
    virtual bool Interpolate(double delta) = 0;
    virtual void Reset() { _cur = 0; _time = 0.0; }
    inline AniBase* GetAniBase() const { return _ani; }
    inline double GetTime() const { return _time; }
    inline void SetTime(double time)
    {
      if(_time > time) // If we are rewinding, we have to reset _cur to 0 to preserve a valid state.
        Reset();
      _time = time; // Otherwise it is legal for us to skip ahead an arbitrary length of time.
    }

    AniState& operator=(const AniState& copy)
    {
      if(_ani) _ani->Drop();
      _ani = copy._ani;
      _cur = copy._cur;
      _time = copy._time;
      if(_ani) _ani->Grab();
      return *this;
    }
    AniState& operator=(AniState&& mov)
    {
      if(_ani) _ani->Drop();
      _ani = mov._ani;
      _cur = mov._cur;
      _time = mov._time;
      mov._ani = 0;
      return *this;
    }

  protected:
    AniBase* _ani;
    uint32_t _cur;
    double _time;
  };

  template<typename T, typename D = void, typename REF = T, typename AUX = void>
  struct BSS_COMPILER_DLLEXPORT AniStateDiscrete : public AniState
  {
    using AniState::_ani;
    using AniState::_cur;
    using AniState::_time;
    AniStateDiscrete(const AniStateDiscrete& copy) : AniState(copy), _set(copy._set) {}
    AniStateDiscrete(AniStateDiscrete&& mov) : AniState(std::move(mov)), _set(std::move(mov._set)) {}
    AniStateDiscrete(AniBase* p, Delegate<AUX, REF> d) : AniState(p), _set(d) { assert(_ani->SizeOf() == sizeof(T)); }
    template<ARRAY_TYPE ArrayType, typename Alloc>
    AniStateDiscrete(Animation<T, D, ArrayType, Alloc>* p, Delegate<AUX, REF> d) : AniState(p), _set(d) {}
    AniStateDiscrete() : _set(0, 0) {}

    AniStateDiscrete& operator=(const AniStateDiscrete& copy) { AniState::operator=(copy); _set = copy._set; return *this; }
    AniStateDiscrete& operator=(AniStateDiscrete&& mov) { AniState::operator=(std::move(mov)); _set = std::move(mov._set); return *this; }

    virtual bool Interpolate(double delta)
    {
      _time += delta;
      auto svar = _ani->GetSize();
      auto v = (typename Animation<T, D>::FRAME*)_ani->GetArray();
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
    Delegate<AUX, REF> _set;
  };

  template<typename T, typename D = void, typename REF = T, typename AUX = void>
  struct BSS_COMPILER_DLLEXPORT AniStateSmooth : public AniStateDiscrete<T, D, REF, AUX>
  {
    typedef AniStateDiscrete<T, D, REF, AUX> BASE;
    using BASE::_ani;
    using BASE::_cur;
    using BASE::_time;
    AniStateSmooth(const AniStateSmooth& copy) : BASE(copy), _init(copy._init) {}
    AniStateSmooth(AniStateSmooth&& mov) : BASE(std::move(mov)), _init(std::move(mov._init)) {}
    AniStateSmooth(AniBase* p, Delegate<AUX, REF> d, T init = T()) : BASE(p, d), _init(init) { assert(_ani->GetFunc() != 0); }
    template<ARRAY_TYPE ArrayType, typename Alloc>
    AniStateSmooth(Animation<T, D, ArrayType, Alloc>* p, Delegate<AUX, REF> d, T init = T()) : BASE(p, d), _init(init) {}
    AniStateSmooth() {}

    AniStateSmooth& operator=(const AniStateSmooth& copy) { BASE::operator=(copy); _init = copy._init; return *this; }
    AniStateSmooth& operator=(AniStateSmooth&& mov) { BASE::operator=(std::move(mov)); _init = std::move(mov._init); return *this; }

    virtual bool Interpolate(double delta)
    {
      _time += delta;
      auto svar = _ani->GetSize();
      auto v = (typename Animation<T, D>::FRAME*)_ani->GetArray();
      auto f = (typename Animation<T, D>::FUNC)_ani->GetFunc();
      double loop = _ani->GetLoop();
      double length = _ani->GetLength();
      if(_time >= length && loop >= 0.0)
      {
        _cur = 0;
        _time = fmod(_time - length, length - loop) + loop;
      }

      while(_cur < svar && v[_cur].time <= _time) ++_cur;
      if(_cur >= svar)
      { //Resolve the animation, but only if there was more than 1 keyframe, otherwise we'll break it.
        if(svar > 1)
          BASE::_set(f(v, svar, svar - 1, 1.0, _init));
      }
      else
      {
        double hold = !_cur ? 0.0 : v[_cur - 1].time;
        BASE::_set(f(v, svar, _cur, (_time - hold) / (v[_cur].time - hold), _init));
      }
      return _time < length;
    }

    static inline T NoInterpolate(const typename Animation<T, D>::FRAME* v, uint32_t s, uint32_t cur, double t, const T& init) { uint32_t i = cur - (t != 1.0); return (i < 0) ? init : v[i].value; }
    static inline T NoInterpolateRel(const typename Animation<T, D>::FRAME* v, uint32_t s, uint32_t cur, double t, const T& init) { assert(cur > 0); return init + v[cur - (t != 1.0)].value; }
    static inline T LerpInterpolate(const typename Animation<T, D>::FRAME* v, uint32_t s, uint32_t cur, double t, const T& init) { return lerp<T>(!cur ? init : v[cur - 1].value, v[cur].value, t); }
    static inline T LerpInterpolateRel(const typename Animation<T, D>::FRAME* v, uint32_t s, uint32_t cur, double t, const T& init) { assert(cur > 0); return init + lerp<T>(v[cur - 1].value, v[cur].value, t); }

  protected:
    T _init; // This is referenced by an index of -1
  };

  struct AniCubicEasing
  {
    static inline double TimeBezier(double s, double p1, double p2) { double inv = 1.0 - s; return 3.0*inv*inv*s*p1 + 3.0*inv*s*s*p2 + s*s*s; }
    static inline double TimeBezierD(double s, double p1, double p2) { double inv = 1.0 - s; return 3.0*inv*inv*(p1)+6.0*inv*s*(p2 - p1) + inv*s*s*p2 + 3.0*s*s*(1.0 - p2); }

    template<class T, class D>
    static inline double GetProgress(const typename Animation<T, D>::FRAME* v, uint32_t l, uint32_t cur, double t)
    {
      AniCubicEasing& data = v[cur].data;
      auto f = [&](double s) -> double { return TimeBezier(s, data.t1, data.t2) - t; };
      auto fd = [&](double s) -> double { return TimeBezierD(s, data.t1, data.t2); };
      double p = NewtonRaphsonBisection<double>(t, 0.0, 1.0, f, fd, FLT_EPS);
      return TimeBezier(p, data.s1, data.s2);
    }
    double t1;
    double s1;
    double t2;
    double s2;
  };

  template<class T>
  struct AniLinearData : AniCubicEasing
  {
    static inline T LinearInterpolate(const typename Animation<T, AniLinearData<T>>::FRAME* v, uint32_t l, uint32_t cur, double t, const T& init)
    {
      double s = GetProgress<T, AniLinearData<T>>(v, l, cur, t);
      return lerp<T>(!cur ? init : v[cur - 1].value, v[cur].value, s);
    }
    static inline T LinearInterpolateRel(const typename Animation<T, AniLinearData<T>>::FRAME* v, uint32_t l, uint32_t cur, double t, const T& init)
    {
      assert(cur > 0);
      double s = GetProgress<T, AniLinearData<T>>(v, l, cur, t);
      return init + lerp<T>(v[cur - 1].value, v[cur].value, s);
    }
  };

  template<class T, double(*LENGTH)(const T&, const T&, const T&)>
  struct AniQuadData : AniCubicEasing
  {
    template<class U>
    static inline U Bezier(double s, const U& p0, const U& p1, const U& p2) { double inv = 1.0 - s; return inv*inv*p0 + 2.0*inv*s*p1 + s*s*p2; }
    template<class U>
    static inline U BezierD(double s, const U& p0, const U& p1, const U& p2) { double inv = 1.0 - s; return 2.0*inv*(p1 - p0) + 2.0*s*(p2 - p1); }

    // Almost all points are really just arrays of floats or doubles. This provides a general length function to handle all those cases in any number of dimensions
    template<class U, int I>
    static inline double Length(double s, U(&p0)[I], U(&p1)[I], U(&p2)[I])
    {
      U r;
      U a = (U)0;
      for(int i = 0; i < I; ++i)
      {
        r = BezierD<U>(s, p0[i], p1[i], p2[i]);
        a += r * r;
      }
      return FastSqrt<U>(r);
    }
    static inline T QuadraticInterpolateBase(const typename Animation<T, AniQuadData<T, LENGTH>>::FRAME* v, uint32_t l, uint32_t cur, double t, const T& init)
    {
      double s = GetProgress<T, AniQuadData<T, LENGTH>>(v, l, cur, t);
      const T& p0 = (!cur ? init : v[cur - 1].value);
      const T& p1 = v[cur].cp; // we store the control point in the end point, not the start point, which allows us to properly use the init value.
      const T& p2 = v[cur].value;
      auto f = [&](double s) -> double { return GaussianQuadrature<double, 3>(0.0, s, &LENGTH, p0, p1, p2) - s; };
      auto fd = [&](double s) -> double { return LENGTH(s, p0, p1, p2); };
      return Bezier<T>(NewtonRaphsonBisection<double>(s, 0.0, 1.0, f, fd, FLT_EPS), p0, p1, p2);
    }
    static inline T QuadraticInterpolate(const typename Animation<T, AniQuadData<T, LENGTH>>::FRAME* v, uint32_t l, uint32_t cur, double t, const T& init)
    {
      return QuadraticInterpolateBase(v, l, cur, t, init);
    }
    static inline T QuadraticInterpolateRel(const typename Animation<T, AniQuadData<T, LENGTH>>::FRAME* v, uint32_t l, uint32_t cur, double t, const T& init)
    {
      assert(cur > 0);
      return init + QuadraticInterpolateBase(v, l, cur, t, init);
    }
    T cp;
  };

  template<class T, double(*LENGTH)(const T&, const T&, const T&, const T&)>
  struct AniCubicData : AniCubicEasing
  {
    static inline T Bezier(double s, const T& p0, const T& p1, const T& p2, const T& p3) { double inv = 1.0 - s; return inv*inv*inv*p0 + 3.0*inv*inv*s*p1 + 3.0*inv*s*s*p2 + s*s*s*p3; }
    template<class U>
    static inline U BezierD(double s, const U& p0, const U& p1, const U& p2, const U& p3) { double inv = 1.0 - s; return 3.0*inv*inv*(p1 - p0) + 6.0*inv*s*(p2 - p1) + 3.0*inv*s*s*(p3 - p2); }

    // Almost all points are really just arrays of floats or doubles. This provides a general length function to handle all those cases in any number of dimensions
    template<class U, int I>
    static inline double Length(double s, U(&p0)[I], U(&p1)[I], U(&p2)[I], U(&p3)[I])
    {
      U r;
      U a = (U)0;
      for(int i = 0; i < I; ++i)
      {
        r = BezierD<U>(s, p0[i], p1[i], p2[i], p3[i]);
        a += r * r;
      }
      return FastSqrt<U>(r);
    }
    static inline T CubicInterpolateBase(const typename Animation<T, AniCubicData<T, LENGTH>>::FRAME* v, uint32_t l, uint32_t cur, double t, const T& init)
    {
      double s = GetProgress<T, AniCubicData<T, LENGTH>>(v, l, cur, t);
      const T& p0 = (!cur ? init : v[cur - 1].value);
      const T& p1 = v[cur].p1; // we store the control point in the end point, not the start point, which allows us to properly use the init value.
      const T& p2 = v[cur].p2;
      const T& p3 = v[cur].value;
      double p0_cache = LENGTH(0.0, p0, p1, p2, p3);
      auto f = [&](double s) -> double { return GaussianQuadrature<double, 5>(0.0, s, &LENGTH, p0, p1, p2, p3) - s; };
      auto fd = [&](double s) -> double { return LENGTH(s, p0, p1, p2, p3); };
      return Bezier(NewtonRaphsonBisection<double>(s, 0.0, 1.0, f, fd, FLT_EPS), p0, p1, p2, p3);
    }
    static inline double CubicInterpolate(const typename Animation<T, AniCubicData<T, LENGTH>>::FRAME* v, uint32_t l, uint32_t cur, double t, const T& init)
    {
      return CubicInterpolateBase(v, l, cur, t, init);
    }
    static inline double CubicInterpolateRel(const typename Animation<T, AniCubicData<T, LENGTH>>::FRAME* v, uint32_t l, uint32_t cur, double t, const T& init)
    {
      assert(cur > 0);
      return init + CubicInterpolateBase(v, l, cur, t, init);
    }

    T p1;
    T p2;
  };

  template<typename T, typename AUX, typename REF = T, typename QUEUEALLOC = StaticAllocPolicy<std::pair<double, AUX>>>
  struct BSS_COMPILER_DLLEXPORT AniStateInterval : public AniStateDiscrete<T, double, REF, AUX>
  {
    typedef AniStateDiscrete<T, double, REF, AUX> BASE;
    using BASE::_ani;
    using BASE::_cur;
    using BASE::_time;
    AniStateInterval(AniBase* p, Delegate<AUX, REF> d, Delegate<void, AUX> rm) : BASE(p, d), _remove(rm) { assert(_ani->SizeOf() == sizeof(T)); }
    template<ARRAY_TYPE ArrayType, typename Alloc>
    AniStateInterval(AnimationInterval<T, ArrayType, Alloc>* p, Delegate<AUX, REF> d, Delegate<void, AUX> rm) : BASE(p, d), _remove(rm) {}
    inline ~AniStateInterval() { _clearQueue(); }
    virtual void Reset() { _clearQueue(); BASE::Reset(); }
    virtual bool Interpolate(double delta)
    {
      _time += delta;
      auto svar = _ani->GetSize();
      auto v = (typename Animation<T, double>::FRAME*)_ani->GetArray();
      double loop = _ani->GetLoop();
      double length = _ani->GetLength();

      while(_cur < svar && v[_cur].time <= _time)
        _addToQueue(v[_cur++]); // We call all the discrete values because many discrete values are interdependent on each other.
      if(_time >= length && loop >= 0.0) // We do the loop check down here because we need to finish calling all the discrete values on the end of the animation before looping
      {
        _cur = 0; // We can't call Reset() here because _time contains information we need.
        _clearQueue(); // Ensure the queue is cleared before looping or Bad Things will happen.
        _time = fmod(_time - length, length - loop);// + loop; // instead of adding loop here we just pass loop into Interpolate, which adds it.
        return Interpolate(loop); // because we used fmod, it is impossible for this to result in another loop.
      }
      while(!_queue.Empty() && _queue.Peek().first <= _time)
        _remove(_queue.Pop().second);
      return _time < length;
    }
  protected:
    inline void _addToQueue(const typename Animation<T, double>::FRAME& v) { _queue.Push(v.data, BASE::_set(v.value)); }

    void _clearQueue()
    {
      while(!_queue.Empty()) // Correctly remove everything currently on the queue
        _remove(_queue.Pop().second);
    }
    Delegate<void, AUX> _remove; //Delegate for removal
    PriorityQueue<double, AUX, CompT<double>, uint32_t, ARRAY_SIMPLE, QUEUEALLOC> _queue;
  };
}

#endif