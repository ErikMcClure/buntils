// Copyright ©2*013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __ANI_ATTRIBUTE__BSS__
#define __ANI_ATTRIBUTE__BSS__

#include "delegate.h"
#include "cPriorityQueue.h"
#include "cBitField.h"
#include "cBitStream.h"

namespace bss_util {
  // Generic attribute definition
  struct BSS_COMPILER_DLLEXPORT AttrDef { };

  // Base AniAttribute definition
  struct BSS_COMPILER_DLLEXPORT AniAttribute
  {
    typedef unsigned short IDTYPE;

    inline AniAttribute(unsigned char _typeID) : typeID(_typeID) {}
    virtual ~AniAttribute() {}
    virtual bool Interpolate(double timepassed)=0;
    virtual void Start()=0;
    virtual double Length()=0;
    virtual AniAttribute* BSS_FASTCALL Clone() const { return 0; }
    virtual void BSS_FASTCALL Attach(AttrDef* def)=0;
    virtual void BSS_FASTCALL Detach()=0;
    virtual void BSS_FASTCALL CopyAnimation(AniAttribute* ptr)=0;
    virtual void BSS_FASTCALL AddAnimation(AniAttribute* ptr)=0;
    virtual void BSS_FASTCALL Serialize(std::ostream& s)=0;
    virtual void BSS_FASTCALL Deserialize(std::istream& s)=0;

    const unsigned char typeID;
    //static cHash<unsigned char, size_t(*)(AniAttribute*)> _attrhash;
  };

  template<unsigned char TypeID, typename Alloc=StaticAllocPolicy<char>>
  struct BSS_COMPILER_DLLEXPORT AniAttributeT { typedef void IDTYPE; };

  // Default KeyFrame used by most attributes
  template<unsigned char TypeID>
  struct KeyFrame
  {
    inline KeyFrame(double time, const typename AniAttributeT<TypeID>::Data& value) : time(time), value(value) {}
    inline KeyFrame() : time(0.0) {}
    double time;
    typename AniAttributeT<TypeID>::Data value;
  };

  // Abstract base implementation of an AniAttribute
  template<typename Alloc, typename DATA, unsigned char TypeID, ARRAY_TYPE ARRAY = CARRAY_SIMPLE>
  struct BSS_COMPILER_DLLEXPORT AniAttributeBase : public AniAttribute
  {
    typedef DATA Data;
    typedef KeyFrame<TypeID> KF;
    typedef typename AniAttribute::IDTYPE IDTYPE;
    typedef cArray<KF, IDTYPE, ARRAY, typename Alloc::template rebind<KF>::other> TVT_ARRAY_T;
    enum ATTR_FLAGS : unsigned char { ATTR_INITZERO=1, ATTR_REL=2, ATTR_ATTACHED=4 };

    inline AniAttributeBase(const AniAttributeBase& copy) : AniAttribute(copy), _timevalues(copy._timevalues), _curpair(copy._curpair),
      _flags(copy._flags)
    {
      assert(_timevalues.Size()>0);
    }
    inline explicit AniAttributeBase() : AniAttribute(TypeID), _curpair(1), _timevalues(0), _flags(0) { Clear(); }
    inline virtual double Length() { return _timevalues.Back().time; }
    inline virtual AniAttribute* BSS_FASTCALL Clone() const { return 0; }
    inline virtual void BSS_FASTCALL CopyAnimation(AniAttribute* ptr) { operator=(*static_cast<AniAttributeBase*>(ptr)); }
    inline virtual void BSS_FASTCALL AddAnimation(AniAttribute* ptr) { operator+=(*static_cast<AniAttributeBase*>(ptr)); }
    inline IDTYPE GetNumFrames() const { return _timevalues.Size(); }
    inline const KF& GetKeyFrame(IDTYPE index) const { return _timevalues[index]; }
    inline void Clear() { _timevalues.SetSize(1); _timevalues[0].time=0; _flags-=ATTR_INITZERO; }
    virtual double SetKeyFrames(const KF* frames, IDTYPE num)
    {
      if(!num || !frames)
        Clear();
      else if(_flags[ATTR_INITZERO] = (frames[0].time==0.0)) // If the array includes 0, we can just copy it over directly
        _timevalues.SetArray(frames, num);
      else { // If the array doesn't include 0, we have to introduce our own fake 0 value.
        _timevalues.SetArray(frames, num, 1);
        _timevalues[0].time=0;
      }
      return Length();
    }
    virtual IDTYPE AddKeyFrame(const KF& frame) //time is given in milliseconds
    {
      if(frame.time==0.0)
      {
        _flags+=ATTR_INITZERO;
        _timevalues[0]=frame;
        return 0;
      }
      IDTYPE i;
      IDTYPE svar=_timevalues.Size(); //doesn't change
      for(i=svar; (i--)>0;) // We go through this backwards because it's more efficient when we're adding things in order
        if(frame.time>_timevalues[i].time)
          break;
      ++i;

      if(frame.time==_timevalues[i].time)
        _timevalues[i]=frame;
      else
        _timevalues.Insert(frame, i);
      return i;
    }
    virtual bool RemoveKeyFrame(IDTYPE ID)
    {
      if(ID>=_timevalues.Size()) return false;
      if(_timevalues.Size()<=1) _flags-=ATTR_INITZERO;
      else _timevalues.Remove(ID);
      return true;
    }
    AniAttributeBase& operator=(const AniAttributeBase& right)
    {
      _flags[ATTR_INITZERO]=right._flags[ATTR_INITZERO];
      _timevalues=right._timevalues;
      _curpair=right._curpair;
      return *this;
    }
    AniAttributeBase& operator+=(const AniAttributeBase& right)
    {
      for(unsigned int i = 0; i < right._timevalues.Size(); ++i)
        AddKeyFrame(right._timevalues[i]); // We can't directly append the array because it might need to be interlaced with ours.
      return *this;
    }
    virtual void BSS_FASTCALL Serialize(std::ostream& s)
    {
      bss_Serialize<AniAttribute::IDTYPE>(_timevalues.Size(), s);
      bss_Serialize<unsigned char>(_flags&(~ATTR_ATTACHED), s);
      for(unsigned int i = 0; i < _timevalues.Size(); ++i)
      {
        bss_Serialize(_timevalues[i].time, s);
        bss_Serialize(_timevalues[i].value, s);
      }
    }
    virtual void BSS_FASTCALL Deserialize(std::istream& s)
    {
      AniAttribute::IDTYPE len;
      bss_Deserialize<AniAttribute::IDTYPE>(len, s);
      unsigned char hold;
      bss_Deserialize<unsigned char>(hold, s);
      _flags=((_flags&ATTR_ATTACHED)|hold);

      _timevalues.SetSize(len);
      for(unsigned int i = 0; i < len; ++i)
      {
        bss_Deserialize(_timevalues[i].time, s);
        bss_Deserialize(_timevalues[i].value, s);
      }
    }
    virtual void BSS_FASTCALL Attach(AttrDef* def) { _flags+=ATTR_ATTACHED; }
    virtual void BSS_FASTCALL Detach() { _flags-=ATTR_ATTACHED; }

  protected:
    BSS_FORCEINLINE bool _initzero() const { return _flags&ATTR_INITZERO; }
    BSS_FORCEINLINE bool _attached() { if(_flags&ATTR_ATTACHED) return true; _curpair=_timevalues.Size(); return false; }

    TVT_ARRAY_T _timevalues;
    IDTYPE _curpair;
    cBitField<unsigned char> _flags;
  };

  // Fully generic attribute accepting any value that can be called as a function (seperated out to prevent it from calling floats as functions)
  template<typename Alloc, typename T, unsigned char TypeID>
  struct BSS_COMPILER_DLLEXPORT AniAttributeGeneric : public AniAttributeBase<Alloc, T, TypeID, CARRAY_SAFE>
  {
    typedef AniAttributeBase<Alloc, T, TypeID, CARRAY_SAFE> BASE;
    typedef AttrDef ATTRDEF;
    using BASE::_timevalues;
    using BASE::_curpair;

    inline AniAttributeGeneric(const AniAttributeGeneric& copy) : BASE(copy) {}
    inline explicit AniAttributeGeneric() : BASE() {}
    virtual bool Interpolate(double timepassed)
    {
      auto svar=_timevalues.Size();
      while(_curpair<svar && _timevalues[_curpair].time <= timepassed)
        _timevalues[_curpair++].value(); //You have to call ALL events even if you missed some because you don't know which ones do what
      return _curpair<svar;
    }
    inline virtual void Start() { if(!BASE::_attached()) return; _curpair=1; if(BASE::_initzero()) _timevalues[0].value(); }
    inline virtual AniAttribute* BSS_FASTCALL Clone() const { return new(Alloc::template rebind<AniAttributeGeneric>::other::allocate(1)) AniAttributeGeneric(*this); }
    inline AniAttributeGeneric& operator=(const AniAttributeGeneric& right) { BASE::operator=(right); return *this; }
  };

  // Discrete attribute definition
  template<typename DEL>
  struct BSS_COMPILER_DLLEXPORT AttrDefDiscrete : AttrDef { explicit AttrDefDiscrete(DEL d) : del(d) {} DEL del; };

  // Discrete attribute definition
  template<typename Alloc, typename T, unsigned char TypeID, typename DEL = delegate<void, T>, ARRAY_TYPE ARRAY = CARRAY_SIMPLE>
  struct BSS_COMPILER_DLLEXPORT AniAttributeDiscrete : public AniAttributeBase<Alloc, T, TypeID, ARRAY>
  {
    typedef DEL DELEGATE;
    typedef AttrDefDiscrete<DELEGATE> ATTRDEF;
    typedef AniAttributeBase<Alloc, T, TypeID, ARRAY> BASE;
    using BASE::_timevalues;
    using BASE::_curpair;

    inline AniAttributeDiscrete(const AniAttributeDiscrete& copy) : BASE(copy), _del(copy._del) {}
    inline explicit AniAttributeDiscrete() : BASE(), _del(0, 0) {}
    virtual bool Interpolate(double timepassed)
    {
      auto svar=_timevalues.Size();
      while(_curpair<svar && _timevalues[_curpair].time <= timepassed)
        _del(_timevalues[_curpair++].value); // We call all the discrete values because many discrete values are interdependent on each other.
      return _curpair<svar;
      //   IDTYPE svar=_timevalues.Size();
      //   while(_curpair<svar && _timevalues[_curpair].time < timepassed) ++_curpair;
      //   if(_curpair>=svar) { if(svar>1) _del(_timevalues.Back().value); return false; } //Resolve the animation
      //   _del(_timevalues[_curpair].value);
      //return true;
    }
    inline virtual void Start() { if(!BASE::_attached()) return; _curpair=1; if(BASE::_initzero()) _del(_timevalues[0].value); }
    inline virtual AniAttribute* BSS_FASTCALL Clone() const { return new(Alloc::template rebind<AniAttributeDiscrete>::other::allocate(1)) AniAttributeDiscrete(*this); }
    virtual void BSS_FASTCALL Attach(AttrDef* def) { _del=static_cast<ATTRDEF*>(def)->del; BASE::Attach(def); }

  protected:
    DELEGATE _del;
  };

  // Smooth attribute definition
  template<typename DELEGATE, typename GETDELEGATE>
  struct BSS_COMPILER_DLLEXPORT AttrDefSmooth : AttrDefDiscrete<DELEGATE> {
    AttrDefSmooth(GETDELEGATE g, DELEGATE d) : AttrDefDiscrete<DELEGATE>(d), get(g) {}
    GETDELEGATE get;
  };

  // Continuous attribute definition supporting relative animations. get is required only if relative animations are used. get cannot be
  // NULL if you haven't supplied a value in the 0.0 time segment.
  template<typename Alloc, typename VALUE, unsigned char TypeID, typename DATA=VALUE, typename DEL = delegate<void, VALUE>, typename GETD = delegate<const VALUE&>, ARRAY_TYPE ARRAY = CARRAY_SIMPLE>
  struct BSS_COMPILER_DLLEXPORT AniAttributeSmooth : public AniAttributeDiscrete<Alloc, DATA, TypeID, DEL, ARRAY>
  {
  protected:
    typedef AniAttributeDiscrete<Alloc, DATA, TypeID, DEL, ARRAY> BASE;
    typedef AniAttributeBase<Alloc, DATA, TypeID, ARRAY> ROOT;
    using BASE::_flags;
    using BASE::ATTR_REL;

  public:
    typedef GETD GETDELEGATE;
    typedef typename ROOT::TVT_ARRAY_T TVT_ARRAY_T;
    typedef typename ROOT::IDTYPE IDTYPE;
    typedef AttrDefSmooth<DEL, GETDELEGATE> ATTRDEF;
    typedef VALUE(BSS_FASTCALL *FUNC)(const TVT_ARRAY_T&, IDTYPE, double);
    using ROOT::_timevalues;
    using ROOT::_curpair;

    inline AniAttributeSmooth(const AniAttributeSmooth& copy) : BASE(copy), _get(0,0), _func(copy._func) {}
    inline explicit AniAttributeSmooth() : BASE(), _func(&NoInterpolate), _get(0, 0) {}
    virtual bool Interpolate(double timepassed)
    {
      IDTYPE svar=_timevalues.Size();
      while(_curpair<svar && _timevalues[_curpair].time<=timepassed) ++_curpair;
      if(_curpair>=svar)
      { //Resolve the animation, but only if there was more than 1 keyframe, otherwise we'll break it.
        if(svar>1) _setval(_func(_timevalues, svar-1, 1.0));
        return false;
      }
      double hold = _timevalues[_curpair-1].time;
      _setval(_func(_timevalues, _curpair, (timepassed-hold)/(_timevalues[_curpair].time-hold)));
      return true;
    }
    virtual void Start()
    {
      if(!BASE::_attached()) return;
      _curpair=1;
      if(!_get.IsEmpty()) _initval=_get();
      assert(BASE::_initzero() || !_get.IsEmpty()); // You can have a _timevalues size of just 1, but only if you have interpolation disabled
      if(!BASE::_initzero())
        _timevalues[0].value=_get();
      _setval(_func(_timevalues, _curpair, 0.0));
    }
    inline virtual AniAttribute* BSS_FASTCALL Clone() const { return new(Alloc::template rebind<AniAttributeSmooth>::other::allocate(1)) AniAttributeSmooth(*this); }
    inline virtual void BSS_FASTCALL CopyAnimation(AniAttribute* ptr) { operator=(*static_cast<AniAttributeSmooth*>(ptr)); }
    inline virtual bool SetInterpolation(FUNC func) { if(!func) return false; _func=func; return true; }
    inline virtual bool SetRelative(bool rel) { if(_get.IsEmpty()) return false; _flags[ATTR_REL]=rel; return true; } // If set to non-zero, this will be relative.
    virtual void BSS_FASTCALL Attach(AttrDef* def) { _get=static_cast<ATTRDEF*>(def)->get; BASE::Attach(def); }
    inline AniAttributeSmooth& operator=(const AniAttributeSmooth& right)
    {
      BASE::operator=(right);
      _func=right._func;
      return *this;
    }

    static inline VALUE BSS_FASTCALL NoInterpolate(const TVT_ARRAY_T& a, IDTYPE i, double t) { return a[i-(t!=1.0)].value; }
    static inline VALUE BSS_FASTCALL LerpInterpolate(const TVT_ARRAY_T& a, IDTYPE i, double t) { return lerp<VALUE>(a[i-1].value, a[i].value, t); }
    static inline VALUE BSS_FASTCALL CubicInterpolate(const TVT_ARRAY_T& a, IDTYPE i, double t) { return CubicBSpline<VALUE>(t, a[i-1-(i!=1)].value, a[i-1].value, a[i].value, a[i+((i+1)!=a.Size())].value); }
    typedef VALUE (BSS_FASTCALL *TIME_FNTYPE)(const TVT_ARRAY_T& a, IDTYPE i, double t); // VC++ 2010 can't handle this being in the template itself
    template<TIME_FNTYPE FN, double(*TIME)(DATA&)>
    static inline VALUE BSS_FASTCALL TimeInterpolate(const TVT_ARRAY_T& a, IDTYPE i, double t) { return (*FN)(a, i, UniformQuadraticBSpline<double, double>(t, (*TIME)(a[i-1-(i>2)]), (*TIME)(a[i-1]), (*TIME)(a[i]))); }

  protected:
    BSS_FORCEINLINE void _setval(const VALUE& val) const { BASE::_del((_flags&ATTR_REL)?val+_initval:val); }

    VALUE _initval;
    GETDELEGATE _get;
    FUNC _func;
  };

  // Interval attribute definition
  template<typename DELEGATE, typename RMDELEGATE>
  struct BSS_COMPILER_DLLEXPORT AttrDefInterval : AttrDefDiscrete<DELEGATE> {
    AttrDefInterval(RMDELEGATE rm, DELEGATE d) : AttrDefDiscrete<DELEGATE>(d), rmdel(rm) {}
    RMDELEGATE rmdel;
  };

  // Discrete animation with an interval. After the interval has passed, the object is removed using a second delegate function.
  template<typename Alloc, typename T, typename AUX, unsigned char TypeID, double(*TODURATION)(const T&), typename RMDEL = delegate<void, AUX>, typename DEL = delegate<AUX, T>, ARRAY_TYPE ARRAY = CARRAY_SIMPLE>
  struct BSS_COMPILER_DLLEXPORT AniAttributeInterval : AniAttributeDiscrete<Alloc, T, TypeID, DEL, ARRAY>
  {
    typedef AttrDefInterval<DEL, RMDEL> ATTRDEF;
    typedef AniAttributeBase<Alloc, T, TypeID, ARRAY> ROOT;
    typedef AniAttributeDiscrete<Alloc, T, TypeID, DEL, ARRAY> BASE;
    typedef typename ROOT::IDTYPE IDTYPE;
    typedef std::pair<double, AUX> QUEUEPAIR; // VS2010 can't handle this being inside the rebind for some reason
    typedef typename Alloc::template rebind<QUEUEPAIR>::other QUEUEALLOC;
    typedef T DATA;
    using ROOT::_timevalues;
    using ROOT::_curpair;

    inline AniAttributeInterval(const AniAttributeInterval& copy) : BASE(copy), _rmdel(copy._rmdel) {}
    inline AniAttributeInterval() : BASE(), _rmdel(0, 0), _length(0) {}
    inline ~AniAttributeInterval() {
      while(!_queue.Empty()) // Correctly remove everything currently on the queue
        _rmdel(_queue.Pop().second);
    }
    virtual bool Interpolate(double timepassed)
    {
      IDTYPE svar=_timevalues.Size();
      while(_curpair<svar && _timevalues[_curpair].time <= timepassed)
        _addtoqueue(_timevalues[_curpair++].value); // We call all the discrete values because many discrete values are interdependent on each other.

      while(!_queue.Empty() && _queue.Peek().first <= timepassed)
        _rmdel(_queue.Pop().second);
      return _curpair<svar && _queue.Empty();
    }
    inline virtual double Length() { return _length; }
    virtual double SetKeyFrames(const KeyFrame<TypeID>* frames, IDTYPE num) { BASE::SetKeyFrames(frames, num); _recalclength(); return _length; }
    virtual IDTYPE AddKeyFrame(const KeyFrame<TypeID>& frame) //time is given in milliseconds
    {
      IDTYPE r = BASE::AddKeyFrame(frame);
      double t = frame.time+TODURATION(frame.value);
      if(t>_length) _length=t;
      return r;
    }
    virtual bool RemoveKeyFrame(IDTYPE ID)
    {
      if(ID<_timevalues.Size() && _length==_timevalues[ID].time+TODURATION(_timevalues[ID].value)) _recalclength();
      return BASE::RemoveKeyFrame(ID);
    }
    inline virtual void Start()
    {
      while(!_queue.Empty()) // Correctly remove everything currently on the queue
        _rmdel(_queue.Pop().second);
      if(!BASE::_attached()) return;
      _curpair=1;
      if(ROOT::_initzero())
        _addtoqueue(_timevalues[0].value);
    }
    inline virtual AniAttribute* BSS_FASTCALL Clone() const { return new(Alloc::template rebind<AniAttributeInterval>::other::allocate(1)) AniAttributeInterval(*this); }
    inline virtual void BSS_FASTCALL CopyAnimation(AniAttribute* ptr) { operator=(*static_cast<AniAttributeInterval*>(ptr)); }
    virtual void BSS_FASTCALL Attach(AttrDef* def) { _rmdel = static_cast<ATTRDEF*>(def)->rmdel; BASE::Attach(def); }
    inline AniAttributeInterval& operator=(const AniAttributeInterval& right) { BASE::operator=(right); _length=right._length; return *this; }

  protected:
    inline void _recalclength()
    {
      _length=0;
      double t;
      for(IDTYPE i = 0; i < _timevalues.Size(); ++i)
        if((t = _timevalues[i].time+TODURATION(_timevalues[i].value))>_length)
          _length=t;
    }
    inline void _addtoqueue(const T& v) { _queue.Push(TODURATION(v), BASE::_del(v)); }

    RMDEL _rmdel; //delegate for removal
    cPriorityQueue<double, AUX, CompT<double>, unsigned int, CARRAY_SIMPLE, QUEUEALLOC> _queue;
    double _length;
  };
}

#endif
