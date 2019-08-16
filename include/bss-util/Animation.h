// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __ANIMATION_H__BSS__
#define __ANIMATION_H__BSS__

#include "ArraySort.h"
#include "RefCounter.h"
#include "Delegate.h"
#include "PriorityQueue.h"

namespace bss {
  // Represents a single animation keyframe
  template<typename T, typename D>
  struct AniFrame
  {
    typedef T VALUE;
    typedef D DATA;

    double time;
    T value;
    D data; // Additional data for animation specific operations, like interpolation

    template<typename Engine>
    void Serialize(Serializer<Engine>& e, const char*)
    {
      e.template EvaluateType<AniFrame>(
        GenPair("time", time),
        GenPair("value", value),
        GenPair("data", data)
        );
    }
  };

  template<typename T>
  struct AniFrame<T, void>
  {
    typedef T VALUE;
    typedef void DATA;

    double time;
    T value;

    template<typename Engine>
    void Serialize(Serializer<Engine>& e, const char*)
    {
      e.template EvaluateType<AniFrame>(
        GenPair("time", time),
        GenPair("value", value)
        );
    }
  };

  class BSS_COMPILER_DLLEXPORT AniStateBase
  {
  public:
    AniStateBase(const AniStateBase& copy) : _time(copy._time) {}
    AniStateBase() : _time(0.0) { }
    virtual ~AniStateBase() { }
    virtual bool Interpolate(double delta) = 0;
    virtual void Reset() { _time = 0.0; }
    inline double GetTime() const { return _time; }
    inline void SetTime(double time)
    {
      if(_time > time) // If we are rewinding, we have to reset _cur to 0 to preserve a valid state.
        Reset();
      _time = time; // Otherwise it is legal for us to skip ahead an arbitrary length of time.
    }

    inline AniStateBase& operator=(const AniStateBase& copy)
    {
      _time = copy._time;
      return *this;
    }

  protected:
    double _time;
  };

  template<typename T, typename A, typename... Args>
  class BSS_COMPILER_DLLEXPORT AniState : public AniStateBase
  {
  public:
    typedef decltype(std::tuple_cat(std::declval<typename std::conditional<std::is_same<void, typename Args::VALUE>::value, std::tuple<>, std::tuple<typename Args::VALUE>>::type>()...)) VALUES;

    AniState(T* dest, const A* ani) : _dest(dest), _ani(ani) {}
    virtual ~AniState() { Reset(); } // Call Reset() up here, otherwise the virtual call isn't properly evaluated
    virtual void Reset() override
    {
      _reset(std::index_sequence_for<Args...>{});
      AniStateBase::Reset();
    }
    virtual bool Interpolate(double delta) override
    {
      double length = _ani->GetLength();
      double loop = _ani->GetLoop();
      if(_time >= length && loop < 0.0) // If animation is over and it doesn't loop, bail out immediately
        return false;
      _time += delta;
      _interpolate(std::index_sequence_for<Args...>{});

      if(_time >= length && loop >= 0.0) // We do the loop check down here because we need to finish calling all the discrete values on the end of the animation before looping
      {
        assert(length > 0.0); // If length is zero everything explodes
        _reset(std::index_sequence_for<Args...>{}); // We only reset the individual states because _time contains information we need
        _time = fmod(_time - length, length - loop);// + loop; // instead of adding loop here we just pass loop into Interpolate, which adds it.
        return Interpolate(loop); // because we used fmod, it is impossible for this to result in another loop.
      }

      return _time < length;
    }
    inline std::tuple<Args...>& States() { return _states; }
    inline const std::tuple<Args...>& States() const { return _states; }
    inline VALUES GetValues() const { VALUES v; _getvalues<0, Args...>(v); return v; }
    inline void SetValues(const VALUES& values) { _setvalues<0, Args...>(values); }

    typedef T Ty;
    typedef A ANI;

  protected:
    template<size_t ...S> BSS_FORCEINLINE void _interpolate(std::index_sequence<S...>) { (std::get<S>(_states).template Interpolate<A, T>(_ani, _dest, _time), ...); }
    template<size_t ...S> BSS_FORCEINLINE void _reset(std::index_sequence<S...>) { (std::get<S>(_states).template Reset<T>(_dest), ...); }
    template<int I, typename X, typename... Xs>
    BSS_FORCEINLINE void _getvalues(VALUES& v) const
    {
      if constexpr(sizeof...(Xs) > 0 && std::is_same<typename X::VALUE, void>::value)
        _getvalues<I, Xs...>(v);
      else if constexpr(!std::is_same<typename X::VALUE, void>::value)
      {
        std::get<I>(v) = std::get<X>(_states).GetInit();
        if constexpr(sizeof...(Xs) > 0)
          _getvalues<I + 1, Xs...>(v);
      }
    }
    template<int I, typename X, typename... Xs>
    BSS_FORCEINLINE void _setvalues(const VALUES& v)
    {
      if constexpr(sizeof...(Xs) > 0 && std::is_same<typename X::VALUE, void>::value)
        _setvalues<I, Xs...>(v);
      else if constexpr(!std::is_same<typename X::VALUE, void>::value)
      {
        std::get<X>(_states).SetInit(std::get<I>(v));
        if constexpr(sizeof...(Xs) > 0)
          _setvalues<I + 1, Xs...>(v);
      }
    }

#pragma warning(push)
#pragma warning(disable:4251)
    std::tuple<Args...> _states;
#pragma warning(pop)
    const A* _ani;
    T* _dest;
  };

  template<typename T, typename D, typename REF, void(T::*FN)(REF)>
  struct BSS_COMPILER_DLLEXPORT AniStateDiscrete
  {
    typedef typename D::FRAME FRAME;
    typedef void VALUE;

    AniStateDiscrete() : _cur(0) {}
    template<typename U>
    void Reset(U* dest) { _cur = 0; }
    template<typename A, typename U>
    void Interpolate(const A* ani, U* dest, double t)
    {
      const D& data = ani->template Get<D>();
      Slice<const FRAME> frames = data.Frames();

      while(_cur < frames.length && frames[_cur].time <= t)
        (static_cast<T*>(dest)->*FN)(frames[_cur++].value); // We call all the discrete values because many discrete values are interdependent on each other.
    }

  protected:
    size_t _cur;
  };

  template<typename T, typename D, typename REF, void(T::*FN)(REF)>
  struct BSS_COMPILER_DLLEXPORT AniStateSmooth
  {
    typedef typename D::FRAME FRAME;
    typedef typename D::INTERPOLATE INTERPOLATE;
    typedef decltype(FRAME::value) VALUE;

    AniStateSmooth() : _cur(0) {}
    template<typename U>
    void Reset(U* dest) { _cur = 0; }
    void SetInit(const VALUE& v) { _init = v; }
    const VALUE& GetInit() const { return _init; }
    template<typename A, typename U>
    void Interpolate(const A* ani, U* dest, double t)
    {
      const D& data = ani->template Get<D>();
      Slice<const FRAME> frames = data.Frames();
      INTERPOLATE f = data.GetInterpolation();

      while(_cur < frames.length && frames[_cur].time <= t)
        ++_cur;

      if(_cur >= frames.length)
      { //Resolve the animation, but only if there was more than 1 keyframe, otherwise we'll break it.
        if(frames.length > 1)
        {
          assert(f != 0); // We only check if f is NULL if there are actually frames to process
          (static_cast<T*>(dest)->*FN)(f(frames.start, frames.length, frames.length - 1, 1.0, _init));
        }
      }
      else
      {
        assert(f != 0);
        double hold = !_cur ? 0.0 : frames[_cur - 1].time;
        (static_cast<T*>(dest)->*FN)(f(frames.start, frames.length, _cur, (t - hold) / (frames[_cur].time - hold), _init));
      }
    }

  protected:
    VALUE _init;
    size_t _cur;
  };

  template<typename T, typename D, typename REF, typename AUX, AUX(T::*FN)(REF), void(T::*REMOVE)(AUX), typename QUEUEALLOC = StandardAllocator<std::pair<double, AUX>>>
  struct BSS_COMPILER_DLLEXPORT AniStateInterval
  {
    typedef typename D::FRAME FRAME;
    typedef void VALUE;

    AniStateInterval() : _cur(0) {}
    template<typename U>
    void Reset(U* dest) { _clearQueue(static_cast<T*>(dest)); _cur = 0; }
    template<typename A, typename U>
    void Interpolate(const A* ani, U* dest, double t)
    {
      const D& data = ani->template Get<D>();
      Slice<const FRAME> frames = data.Frames();

      while(_cur < frames.length && frames[_cur].time <= t)
        _addToQueue(static_cast<T*>(dest), frames[_cur++]); // We call all the discrete values because many discrete values are interdependent on each other.

      while(!_queue.Empty() && _queue.Peek().first <= t)
        (static_cast<T*>(dest)->*REMOVE)(_queue.Pop().second);
    }

  protected:
    inline void _addToQueue(T* dest, const FRAME& v) { _queue.Push(v.data, (dest->*FN)(v.value)); }

    void _clearQueue(T* dest)
    {
      while(!_queue.Empty()) // Correctly remove everything currently on the queue
        (dest->*REMOVE)(_queue.Pop().second);
    }

    PriorityQueue<double, AUX, CompT<double>, size_t, ARRAY_SIMPLE, QUEUEALLOC> _queue;
    size_t _cur;
  };

  template<typename T, typename D = void, ARRAY_TYPE ArrayType = ARRAY_SIMPLE, int DATAID = 0, typename Alloc = StandardAllocator<AniFrame<T, D>>>
  struct BSS_COMPILER_DLLEXPORT AniData
  {
    typedef AniStateDiscrete<AniStateBase, AniData, T, nullptr> STATE;
    typedef AniFrame<T, D> FRAME;

    AniData() {}
    AniData(AniData&& mov) : _frames(std::move(mov._frames)) {}
    AniData(const AniData& copy) : _frames(copy._frames) {}
    AniData(const FRAME* src, size_t len) { Set(src, len); }
    BSS_FORCEINLINE double Begin() const { return !_frames.Length() ? 0.0 : _frames.Front().time; }
    BSS_FORCEINLINE double End() const { return !_frames.Length() ? 0.0 : _frames.Back().time; }
    BSS_FORCEINLINE Slice<const FRAME> Frames() const { return _frames.GetSlice(); }
    BSS_FORCEINLINE size_t Add(const FRAME& frame) { return _frames.Insert(frame); }
    BSS_FORCEINLINE size_t Add(double time, const T& value)
    {
      FRAME f = { time, value };
      return Add(f);
    }
    inline void Set(const FRAME* src, size_t len)
    {
      _frames = Slice<const FRAME>(src, len);
#ifdef BSS_DEBUG
      for(size_t i = 1; i < _frames.Length(); ++i)
        assert(_frames[i - 1].time <= _frames[i].time);
#endif
    }
    inline bool Remove(size_t index) { return _frames.Remove(index); }

    inline AniData& operator=(AniData&& mov)
    {
      _frames = std::move(mov._frames);
      return *this;
    }
    inline AniData& operator=(const AniData& copy)
    {
      _frames = copy._frames;
      return *this;
    }

    BSS_FORCEINLINE static char CompAniFrame(const FRAME& left, const FRAME& right) { return SGNCOMPARE(left.time, right.time); }

    template<typename Engine>
    void Serialize(Serializer<Engine>& e, const char*)
    {
      e.template EvaluateType<AniData>(GenPair("frame", _frames));
    }

  protected:
    ArraySort<FRAME, CompAniFrame, size_t, ArrayType, Alloc> _frames;
  };

  template<typename T, typename D = void, ARRAY_TYPE ArrayType = ARRAY_SIMPLE, int DATAID = 0, typename Alloc = StandardAllocator<AniFrame<T, D>>>
  struct BSS_COMPILER_DLLEXPORT AniDataSmooth : AniData<T, D, ArrayType, DATAID, Alloc>
  {
    typedef AniStateSmooth<AniStateBase, AniDataSmooth, T, nullptr> STATE;
    typedef AniData<T, D, ArrayType, DATAID, Alloc> BASE;
    typedef typename BASE::FRAME FRAME;
    typedef T(*INTERPOLATE)(const FRAME*, size_t, size_t, double, const T&);

    AniDataSmooth() : _interpolate(0) {}
    explicit AniDataSmooth(INTERPOLATE f) : _interpolate(f) {}
    AniDataSmooth(AniDataSmooth&& mov) : BASE(std::move(mov)), _interpolate(mov._interpolate) { mov._interpolate = 0; }
    AniDataSmooth(const AniDataSmooth& copy) : BASE(copy), _interpolate(copy._interpolate) {}
    AniDataSmooth(const FRAME* src, size_t len, INTERPOLATE f = 0) : BASE(src, len), _interpolate(f) { }
    inline void SetInterpolation(INTERPOLATE f) { _interpolate = f; }
    inline INTERPOLATE GetInterpolation() const { return _interpolate; }

    inline AniDataSmooth& operator=(AniDataSmooth&& mov)
    {
      BASE::operator=(mov);
      _interpolate = mov._interpolate;
      mov._interpolate = 0;
      return *this;
    }
    inline AniDataSmooth& operator=(const AniDataSmooth& copy)
    {
      BASE::operator=(copy);
      _interpolate = copy._interpolate;
      return *this;
    }

    static inline T NoInterpolate(const FRAME* v, size_t s, size_t cur, double t, const T& init) { ptrdiff_t i = ptrdiff_t(cur) - (t != 1.0); return (i < 0) ? init : v[i].value; }
    static inline T NoInterpolateRel(const FRAME* v, size_t s, size_t cur, double t, const T& init) { assert(cur > 0); return init + v[cur - (t != 1.0)].value; }
    static inline T LerpInterpolate(const FRAME* v, size_t s, size_t cur, double t, const T& init) { return lerp<T, float>(!cur ? init : v[cur - 1].value, v[cur].value, (float)t); }
    static inline T LerpInterpolateRel(const FRAME* v, size_t s, size_t cur, double t, const T& init) { assert(cur > 0); return init + lerp<T, float>(v[cur - 1].value, v[cur].value, (float)t); }

  protected:
    INTERPOLATE _interpolate;
  };

  template<typename T, ARRAY_TYPE ArrayType = ARRAY_SIMPLE, int DATAID = 0, typename Alloc = StandardAllocator<AniFrame<T, double>>>
  struct BSS_COMPILER_DLLEXPORT AniDataInterval : AniData<T, double, ArrayType, DATAID, Alloc>
  {
    typedef AniStateInterval<AniStateBase, AniDataInterval, T, char, nullptr, nullptr, StandardAllocator<std::pair<double, char>>> STATE;
    typedef AniData<T, double, ArrayType, DATAID, Alloc> BASE;
    typedef typename BASE::FRAME FRAME;
    using BASE::_frames;
    AniDataInterval() : _end(0.0) {}
    AniDataInterval(AniDataInterval&& mov) : BASE(std::move(mov)), _end(mov._end) { mov._end = 0.0f; }
    AniDataInterval(const AniDataInterval& copy) : BASE(copy), _end(copy._end) {}
    AniDataInterval(const FRAME* src, size_t len) : BASE(src, len) {}
    BSS_FORCEINLINE double End() const { return _end; }
    BSS_FORCEINLINE Slice<const FRAME> Frames() const { return _frames.GetSlice(); }
    BSS_FORCEINLINE size_t Add(const FRAME& frame)
    {
      size_t r = BASE::Add(frame);
      _checkIndex(r);
      return r;
    }
    BSS_FORCEINLINE size_t Add(double time, const T& value)
    {
      typename BASE::FRAME f = { time, value, 0.0 };
      return Add(f);
    }
    BSS_FORCEINLINE size_t Add(double time, const T& value, double interval)
    {
      typename BASE::FRAME f = { time, value, interval };
      return Add(f);
    }
    inline void Set(const typename BASE::FRAME* src, size_t len)
    {
      BASE::Set(src, len);
      _recalcLength();
    }
    inline bool Remove(size_t index)
    {
      if(!BASE::Remove(index))
        return false;
      _recalcLength();
      return true;
    }

    inline AniDataInterval& operator=(AniDataInterval&& mov)
    {
      BASE::operator=(mov);
      _end = mov._end;
      mov._end = 0.0;
      return *this;
    }
    inline AniDataInterval& operator=(const AniDataInterval& copy)
    {
      BASE::operator=(copy);
      _end = copy._end;
      return *this;
    }

  protected:
    void _recalcLength()
    {
      _end = 0.0;
      for(size_t i = 0; i < _frames.Length(); ++i)
        _checkIndex(i);
    }
    void _checkIndex(size_t i)
    {
      double t = _frames[i].time + _frames[i].data;
      if(t > _end)
        _end = t;
    }

    double _end;
  };

  class BSS_COMPILER_DLLEXPORT AnimationBase
  {
  public:
    AnimationBase() : _length(-1.0), _calc(0.0), _loop(-1.0) {}
    AnimationBase(double length, double loop) : _length(length), _calc(0.0), _loop(loop) {}
    AnimationBase(const AnimationBase& ani) :_length(ani._length), _calc(ani._calc), _loop(ani._loop) {}
    AnimationBase(AnimationBase&& ani) : _length(ani._length), _calc(ani._calc), _loop(ani._loop) { ani._length = 0; ani._calc = 0; ani._loop = 0; }
    virtual ~AnimationBase() {}
    inline virtual void SetLength(double length = -1.0) { _length = length; }
    inline double GetLength() const { return _length < 0.0 ? _calc : _length; }
    inline virtual void SetLoop(double loop = 0.0) { _loop = loop; }
    inline double GetLoop() const { return _loop; }

    inline AnimationBase& operator=(AnimationBase&& mov)
    {
      _length = mov._length;
      _calc = mov._calc;
      _loop = mov._loop;
      mov._length = 0;
      mov._calc = 0;
      mov._loop = 0;
      return *this;
    }
    inline AnimationBase& operator=(const AnimationBase& copy)
    {
      _length = copy._length;
      _calc = copy._calc;
      _loop = copy._loop;
      return *this;
    }

  protected:
    double _length;
    double _calc;
    double _loop;
  };

#pragma warning(push)
#pragma warning(disable:4251)
  template<typename... Args>
  class BSS_COMPILER_DLLEXPORT Animation : public AnimationBase, public std::tuple<Args...>
  {
#pragma warning(pop)
  public:
    Animation() {}
    Animation(double length, double loop) : AnimationBase(length, loop) {}
    Animation(const Animation& ani) : AnimationBase(ani), std::tuple<Args...>(ani) {}
    Animation(Animation&& ani) : AnimationBase(std::move(ani)), std::tuple<Args...>(std::move(ani)) {}
    inline virtual void SetLength(double length = -1.0) override
    {
      _length = length;
      _calc = _calclength(std::index_sequence_for<Args...>{});
    }
    template<class T>
    inline const T& Get() const { static_assert(std::disjunction<std::is_same<T, Args>...>::value, "T is not in this Animation type!"); return std::get<T>(*this); }
    template<class T>
    inline T& Get() { static_assert(std::disjunction<std::is_same<T, Args>...>::value, "T is not in this Animation type!"); return std::get<T>(*this); }
    template<class T>
    BSS_FORCEINLINE size_t Add(const typename T::FRAME& frame)
    {
      static_assert(std::disjunction<std::is_same<T, Args>...>::value, "T is not in this Animation type!");
      size_t r = std::get<T>(*this).Add(frame);
      _checklength<T>();
      return r;
    }
    template<class T>
    BSS_FORCEINLINE size_t Add(double time, const typename T::FRAME::VALUE& value)
    {
      static_assert(std::disjunction<std::is_same<T, Args>...>::value, "T is not in this Animation type!");
      size_t r = std::get<T>(*this).Add(time, value);
      _checklength<T>();
      return r;
    }
    template<class T>
    inline bool Remove(size_t index)
    {
      static_assert(std::disjunction<std::is_same<T, Args>...>::value, "T is not in this Animation type!");
      if(!std::get<T>(*this).Remove(index))
        return false;
      _calc = _calclength(std::index_sequence_for<Args...>{});
      return true;
    }
    template<class T>
    inline void Set(const typename T::FRAME* src, size_t len)
    {
      static_assert(std::disjunction<std::is_same<T, Args>...>::value, "T is not in this Animation type!");
      std::get<T>().Set(src, len);
      _calc = _calclength(std::index_sequence_for<Args...>{});
    }

    inline Animation& operator=(Animation&& mov)
    {
      std::tuple<Args...>::operator=(std::move(mov));
      AnimationBase::operator=(std::move(mov));
      return *this;
    }
    inline Animation& operator=(const Animation& copy)
    {
      std::tuple<Args...>::operator=(copy);
      AnimationBase::operator=(copy);
      return *this;
    }

    template<typename Engine>
    void Serialize(Serializer<Engine>& e, const char*)
    {
      e.template EvaluateType<Animation>(
        GenPair("length", _length),
        GenPair("calc", _calc),
        GenPair("loop", _loop),
        GenPair("data", (std::tuple<Args...>&)*this)
        );
    }

    static const int STATESIZE = sizeof(AniState<AniStateBase, Animation<Args...>, typename Args::STATE...>);
    static const int STATEALIGN = alignof(AniState<AniStateBase, Animation<Args...>, typename Args::STATE...>);

  protected:
    template<size_t ...S> BSS_FORCEINLINE double _calclength(std::index_sequence<S...>) const { return std::max({ (std::get<S>(*this).End())... }); }
    template<class T> BSS_FORCEINLINE void _checklength() { _calc = std::max<double>(_calc, std::get<T>(*this).End()); }
  };

  struct AniCubicEasing
  {
    AniCubicEasing() : t1(0), s1(0), t2(1.0), s2(1.0) {}
    AniCubicEasing(double T1, double S1, double T2, double S2) : t1(T1), s1(S1), t2(T2), s2(S2) {}
    static inline double TimeBezier(double s, double p1, double p2) { double inv = 1.0 - s; return 3.0*inv*inv*s*p1 + 3.0*inv*s*s*p2 + s*s*s; }
    static inline double TimeBezierD(double s, double p1, double p2) { double inv = 1.0 - s; return 3.0*inv*inv*(p1)+6.0*inv*s*(p2 - p1) + inv*s*s*p2 + 3.0*s*s*(1.0 - p2); }

    template<class T, class D>
    static inline double GetProgress(const AniFrame<T, D>* v, size_t l, size_t cur, double t)
    {
      const AniCubicEasing& data = v[cur].data;
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
    AniLinearData() {}
    AniLinearData(const AniLinearData&) = default;
    AniLinearData(const AniCubicEasing& copy) : AniCubicEasing(copy) {}
    AniLinearData(double T1, double S1, double T2, double S2) : AniCubicEasing(T1, S1, T2, S2) {}
    static inline T LinearInterpolate(const AniFrame<T, AniLinearData<T>>* v, size_t l, size_t cur, double t, const T& init)
    {
      double s = GetProgress<T, AniLinearData<T>>(v, l, cur, t);
      return lerp<T, float>(!cur ? init : v[cur - 1].value, v[cur].value, (float)s);
    }
    static inline T LinearInterpolateRel(const AniFrame<T, AniLinearData<T>>* v, size_t l, size_t cur, double t, const T& init)
    {
      assert(cur > 0);
      double s = GetProgress<T, AniLinearData<T>>(v, l, cur, t);
      return init + lerp<T, float>(v[cur - 1].value, v[cur].value, (float)s);
    }

    template<typename Engine>
    void Serialize(Serializer<Engine>& e, const char*)
    {
      e.template EvaluateType<AniLinearData>(
        GenPair("t1", t1),
        GenPair("s1", s1),
        GenPair("t2", t2),
        GenPair("s2", s2)
        );
    }
  };

  template<class T, double(*LENGTH)(const T&, const T&, const T&)>
  struct AniQuadData : AniCubicEasing
  {
    AniQuadData() {}
    AniQuadData(const AniQuadData&) = default;
    AniQuadData(const AniCubicEasing& copy) : AniCubicEasing(copy) {}
    AniQuadData(double T1, double S1, double T2, double S2) : AniCubicEasing(T1, S1, T2, S2) {}
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
    static inline T QuadraticInterpolateBase(const AniFrame<T, AniQuadData<T, LENGTH>>* v, size_t l, size_t cur, double t, const T& init)
    {
      double st = GetProgress<T, AniQuadData<T, LENGTH>>(v, l, cur, t);
      const T& p0 = (!cur ? init : v[cur - 1].value);
      const T& p1 = v[cur].cp; // we store the control point in the end point, not the start point, which allows us to properly use the init value.
      const T& p2 = v[cur].value;
      auto f = [&](double s) -> double { return GaussianQuadrature<double, 3>(0.0, s, &LENGTH, p0, p1, p2) - s; };
      auto fd = [&](double s) -> double { return LENGTH(s, p0, p1, p2); };
      return Bezier<T>(NewtonRaphsonBisection<double>(st, 0.0, 1.0, f, fd, FLT_EPS), p0, p1, p2);
    }
    static inline T QuadraticInterpolate(const AniFrame<T, AniQuadData<T, LENGTH>>* v, size_t l, size_t cur, double t, const T& init)
    {
      return QuadraticInterpolateBase(v, l, cur, t, init);
    }
    static inline T QuadraticInterpolateRel(const AniFrame<T, AniQuadData<T, LENGTH>>* v, size_t l, size_t cur, double t, const T& init)
    {
      assert(cur > 0);
      return init + QuadraticInterpolateBase(v, l, cur, t, init);
    }

    template<typename Engine>
    void Serialize(Serializer<Engine>& e, const char*)
    {
      e.template EvaluateType<AniQuadData>(
        GenPair("t1", t1),
        GenPair("s1", s1),
        GenPair("t2", t2),
        GenPair("s2", s2),
        GenPair("cp", cp)
        );
    }

    T cp;
  };

  template<class T, double(*LENGTH)(const T&, const T&, const T&, const T&)>
  struct AniCubicData : AniCubicEasing
  {
    AniCubicData() {}
    AniCubicData(const AniCubicData&) = default;
    AniCubicData(const AniCubicEasing& copy) : AniCubicEasing(copy) {}
    AniCubicData(double T1, double S1, double T2, double S2) : AniCubicEasing(T1, S1, T2, S2) {}

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
    static inline T CubicInterpolateBase(const AniFrame<T, AniCubicData<T, LENGTH>>* v, size_t l, size_t cur, double t, const T& init)
    {
      double st = GetProgress<T, AniCubicData<T, LENGTH>>(v, l, cur, t);
      const T& p0 = (!cur ? init : v[cur - 1].value);
      const T& p1 = v[cur].p1; // we store the control point in the end point, not the start point, which allows us to properly use the init value.
      const T& p2 = v[cur].p2;
      const T& p3 = v[cur].value;
      double p0_cache = LENGTH(0.0, p0, p1, p2, p3);
      auto f = [&](double s) -> double { return GaussianQuadrature<double, 5>(0.0, s, &LENGTH, p0, p1, p2, p3) - s; };
      auto fd = [&](double s) -> double { return LENGTH(s, p0, p1, p2, p3); };
      return Bezier(NewtonRaphsonBisection<double>(st, 0.0, 1.0, f, fd, FLT_EPS), p0, p1, p2, p3);
    }
    static inline double CubicInterpolate(const AniFrame<T, AniCubicData<T, LENGTH>>* v, size_t l, size_t cur, double t, const T& init)
    {
      return CubicInterpolateBase(v, l, cur, t, init);
    }
    static inline double CubicInterpolateRel(const AniFrame<T, AniCubicData<T, LENGTH>>* v, size_t l, size_t cur, double t, const T& init)
    {
      assert(cur > 0);
      return init + CubicInterpolateBase(v, l, cur, t, init);
    }

    template<typename Engine>
    void Serialize(Serializer<Engine>& e, const char*)
    {
      e.template EvaluateType<AniCubicData>(
        GenPair("t1", t1),
        GenPair("s1", s1),
        GenPair("p1", p1),
        GenPair("t2", t2),
        GenPair("s2", s2),
        GenPair("p2", p2)
        );
    }

    T p1;
    T p2;
  };
}

#endif