// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __ANI_ATTRIBUTE__BSS__
#define __ANI_ATTRIBUTE__BSS__

#include "AniTypeID.h"
#include "cPriorityQueue.h"
#include "bss_algo.h"

namespace bss_util {
  // Pair that stores the time and data of a given animation keyframe
  template<unsigned char TypeID>
  struct BSS_COMPILER_DLLEXPORT KeyFrame
  {
    inline KeyFrame(double time, ANI_TID(DATACONST) value) : time(time), value(value) {}
    inline KeyFrame() : time(0.0) {}
    double time;
    ANI_TID(DATA) value;
  };

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
    virtual void BSS_FASTCALL CopyAnimation(AniAttribute* ptr)=0;
    virtual void BSS_FASTCALL AddAnimation(AniAttribute* ptr)=0;

		unsigned char typeID;
  };
  
  // This is a static allocator that redirects all dynamic array allocations to the static functions in cAbstractAnim
  template<typename T>
  struct AniStaticAlloc : AllocPolicySize<T>
  {
    typedef typename AllocPolicySize<T>::pointer pointer;
    inline static pointer allocate(std::size_t cnt, typename std::allocator<void>::const_pointer = 0) { return reinterpret_cast<pointer>(cAbstractAnim::AnimAlloc(cnt*sizeof(T))); }
    inline static void deallocate(pointer p, std::size_t = 0) { cAbstractAnim::AnimFree(p); }
    inline static pointer reallocate(pointer p, std::size_t cnt) { return reinterpret_cast<pointer>(cAbstractAnim::AnimAlloc(cnt*sizeof(T),p)); }
  };

  template<unsigned char TypeID, typename T> struct ANI_ATTR__SAFE__ { typedef cArraySafe<KeyFrame<TypeID>,AniAttribute::IDTYPE,AniStaticAlloc<KeyFrame<TypeID>>> INNER_ARRAY; };
  template<unsigned char TypeID> struct ANI_ATTR__SAFE__<TypeID,void> { typedef cArraySimple<KeyFrame<TypeID>,AniAttribute::IDTYPE,AniStaticAlloc<KeyFrame<TypeID>>> INNER_ARRAY; };

  // Abstract base implementation of an AniAttribute
  template<unsigned char TypeID>
  class BSS_COMPILER_DLLEXPORT AniAttributeT : public AniAttribute
  {
  public:
    typedef typename AniAttribute::IDTYPE IDTYPE;
    typedef cArrayWrap<typename ANI_ATTR__SAFE__<TypeID,ANI_TID(SAFE)>::INNER_ARRAY> TVT_ARRAY_T;
    typedef ANI_TID(VALUE) (BSS_FASTCALL *FUNC)(const TVT_ARRAY_T&,IDTYPE, double);
    
    inline AniAttributeT(const AniAttributeT& copy) : AniAttribute(copy), _timevalues(copy._timevalues), _curpair(copy._curpair),
      _initzero(copy._initzero)
		{
      assert(_timevalues.Size()>0);
		}
    inline AniAttributeT() : AniAttribute(TypeID), _curpair(1), _timevalues(0), _initzero(false) { Clear(); }
    inline virtual double Length() { return _timevalues.Back().time; }
		inline virtual AniAttribute* BSS_FASTCALL Clone() const { return 0; }
    inline virtual void BSS_FASTCALL CopyAnimation(AniAttribute* ptr) { operator=(*static_cast<AniAttributeT*>(ptr)); }
    inline virtual void BSS_FASTCALL AddAnimation(AniAttribute* ptr) { operator+=(*static_cast<AniAttributeT*>(ptr)); }
    inline virtual bool SetInterpolation(FUNC f) { return false; }
    inline virtual bool SetRelative(bool rel) { return false; } // If set to non-zero, this will be relative.
    inline IDTYPE GetNumFrames() const { return _timevalues.Size(); }
    inline const KeyFrame<TypeID>& GetKeyFrame(IDTYPE index) const { return _timevalues[index]; }
    inline void Clear() { _timevalues.SetSize(1); _timevalues[0].time=0; _initzero=false; }
		virtual double SetKeyFrames(const KeyFrame<TypeID>* frames, IDTYPE num)
    {
      if(!num || !frames)
        Clear();
      else if(_initzero = (frames[0].time==0.0)) // If the array includes 0, we can just copy it over directly
        _timevalues.SetArray(frames,num);
      else { // If the array doesn't include 0, we have to introduce our own fake 0 value.
        _timevalues.SetArray(frames,num,1);
        _timevalues[0].time=0;
      }
      return Length();
    }
		virtual IDTYPE AddKeyFrame(const KeyFrame<TypeID>& frame) //time is given in milliseconds
		{
      if(frame.time==0.0)
      {
        _initzero=true;
        _timevalues[0].value=frame.value;
        return 0;
      }
      IDTYPE i;
      IDTYPE svar=_timevalues.Size(); //doesn't change
      for(i=svar; --i>0;) // We go through this backwards because it's more efficient when we're adding things in order
        if(frame.time>_timevalues[i].time)
          break;
      ++i;

      if(frame.time==_timevalues[i].time)
        _timevalues[i].value=frame.value;
      else
        _timevalues.Insert(frame,i);
      return i;
		}
    virtual bool RemoveKeyFrame(IDTYPE ID)
    {
      if(ID>=_timevalues.Size()) return false;
      if(_timevalues.Size()<=1) _initzero=false;
      else _timevalues.Remove(ID);
      return true;
    }
		AniAttributeT& operator=(const AniAttributeT& right)
		{
      _initzero=right._initzero;
      _timevalues=right._timevalues;
      _curpair=right._curpair;
      return *this;
		}
		AniAttributeT& operator+=(const AniAttributeT& right)
    {
      for(unsigned int i = 0; i < right._timevalues.Size(); ++i)
        AddKeyFrame(right._timevalues[i]); // We can't directly append the array because it might need to be interlaced with ours.
      return *this;
    }

  protected:
    TVT_ARRAY_T _timevalues;
    IDTYPE _curpair;
    bool _initzero;
  };
  
  // Fully generic attribute accepting any value that can be called as a function (seperated out to prevent it from calling floats as functions)
  template<unsigned char TypeID>
  class BSS_COMPILER_DLLEXPORT AniAttributeGeneric : public AniAttributeT<TypeID>
  {
  public:
    typedef typename AniAttributeT<TypeID>::IDTYPE IDTYPE;
    using AniAttributeT<TypeID>::_timevalues;
    using AniAttributeT<TypeID>::_curpair;

    inline AniAttributeGeneric(const AniAttributeGeneric& copy) : AniAttributeT<TypeID>(copy) {}
    inline AniAttributeGeneric() {}
    virtual bool Interpolate(double timepassed)
    {
      IDTYPE svar=_timevalues.Size();
      while(_curpair<svar && _timevalues[_curpair].time <= timepassed)
        _timevalues[_curpair++].value(); //You have to call ALL events even if you missed some because you don't know which ones do what
      return _curpair<svar;
    }
    inline virtual void Start() { _curpair=1; if(AniAttributeT<TypeID>::_initzero) _timevalues[0].value(); }
		inline virtual AniAttribute* BSS_FASTCALL Clone() const { return new AniAttributeGeneric(*this); }
		inline AniAttributeGeneric& operator=(const AniAttributeGeneric& right) { AniAttributeT<TypeID>::operator=(right); return *this; }
  };

  // Discrete attribute definition
  template<unsigned char TypeID>
  class BSS_COMPILER_DLLEXPORT AniAttributeDiscrete : public AniAttributeT<TypeID>
  {
  public:
    typedef typename AniAttributeT<TypeID>::IDTYPE IDTYPE;
    using AniAttributeT<TypeID>::_timevalues;
    using AniAttributeT<TypeID>::_curpair;
    
    inline AniAttributeDiscrete(const AniAttributeDiscrete& copy) : AniAttributeT<TypeID>(copy), _del(copy._del) {}
    inline AniAttributeDiscrete(ANI_TID(DELEGATE) del) : AniAttributeT<TypeID>(), _del(del) {}
    virtual bool Interpolate(double timepassed)
    {
      IDTYPE svar=_timevalues.Size();
      while(_curpair<svar && _timevalues[_curpair].time <= timepassed)
        _del(_timevalues[_curpair++].value); // We call all the discrete values because many discrete values are interdependent on each other.
      return _curpair<svar;
   //   IDTYPE svar=_timevalues.Size();
   //   while(_curpair<svar && _timevalues[_curpair].time < timepassed) ++_curpair;
   //   if(_curpair>=svar) { if(svar>1) _del(_timevalues.Back().value); return false; } //Resolve the animation
   //   _del(_timevalues[_curpair].value);
			//return true;
    }
    inline virtual void Start() { _curpair=1; if(AniAttributeT<TypeID>::_initzero) _del(_timevalues[0].value); }
		inline virtual AniAttribute* BSS_FASTCALL Clone() const { return new AniAttributeDiscrete(*this); }
    inline virtual void BSS_FASTCALL CopyAnimation(AniAttribute* ptr) { operator=(*static_cast<AniAttributeDiscrete*>(ptr)); }
    inline AniAttributeDiscrete& operator=(const AniAttributeDiscrete& right) { AniAttributeT<TypeID>::operator=(right); return *this; }
    
  protected:
    ANI_TID(DELEGATE) _del;
  };

  // Continuous attribute definition supporting relative animations. pval is required only if relative animations are used. pval cannot be
  // NULL if you haven't supplied a value in the 0.0 time segment.
  template<unsigned char TypeID>
  class BSS_COMPILER_DLLEXPORT AniAttributeSmooth : public AniAttributeDiscrete<TypeID>
  {
  public:
    typedef typename AniAttributeT<TypeID>::TVT_ARRAY_T TVT_ARRAY_T;
    typedef typename AniAttributeT<TypeID>::IDTYPE IDTYPE;
    typedef typename AniAttributeT<TypeID>::FUNC FUNC;
    using AniAttributeT<TypeID>::_timevalues;
    using AniAttributeT<TypeID>::_curpair;

    inline AniAttributeSmooth(const AniAttributeSmooth& copy) : AniAttributeDiscrete<TypeID>(copy), _pval(0), _func(copy._func), _rel(false) {}
    inline AniAttributeSmooth(ANI_TID(DELEGATE) del, FUNC func=&NoInterpolate, const ANI_TID(VALUE)* pval=0, bool rel=false) :
      AniAttributeDiscrete<TypeID>(del), _func(func), _pval(pval), _rel(rel&&(_pval!=0)) {}
    virtual bool Interpolate(double timepassed)
    {
      IDTYPE svar=_timevalues.Size();
      while(_curpair<svar && _timevalues[_curpair].time<=timepassed) ++_curpair;
      if(_curpair>=svar) 
      { //Resolve the animation, but only if there was more than 1 keyframe, otherwise we'll break it.
        if(svar>1) _setval(_func(_timevalues,svar-1,1.0));
        return false; 
      } 
      double hold = _timevalues[_curpair-1].time;
      _setval(_func(_timevalues,_curpair,(timepassed-hold)/(_timevalues[_curpair].time-hold)));
			return true;
    }
    virtual void Start()
    { 
      _curpair=1; 
      if(_pval) _initval=*_pval; 
      assert(AniAttributeT<TypeID>::_initzero || _pval!=0); // You can have a _timevalues size of just 1, but only if you have interpolation disabled
      if(!AniAttributeT<TypeID>::_initzero) 
        _timevalues[0].value=*_pval;
      _setval(_func(_timevalues,_curpair,0.0));
    }
		inline virtual AniAttribute* BSS_FASTCALL Clone() const { return new AniAttributeDiscrete<TypeID>(*this); }
    inline virtual void BSS_FASTCALL CopyAnimation(AniAttribute* ptr) { operator=(*static_cast<AniAttributeSmooth*>(ptr)); }
    inline virtual bool SetInterpolation(FUNC func) { if(!func) return false; _func=func; return true; }
    inline virtual bool SetRelative(bool rel) { if(!_pval) return _rel=false; _rel=rel; return true; } // If set to non-zero, this will be relative.
    inline AniAttributeSmooth& operator=(const AniAttributeSmooth& right)
    { 
      AniAttributeDiscrete<TypeID>::operator=(right);
      _initval=right._initval;
      _rel=right._rel;
      _func=right._func;
      return *this;
    }

    static inline ANI_TID(VALUE) BSS_FASTCALL NoInterpolate(const TVT_ARRAY_T& a, IDTYPE i, double t) { return a[i-(t!=1.0)].value; }
    static inline ANI_TID(VALUE) BSS_FASTCALL LerpInterpolate(const TVT_ARRAY_T& a, IDTYPE i, double t) { return lerp<ANI_TID(VALUE)>(a[i-1].value,a[i].value,t); }
    static inline ANI_TID(VALUE) BSS_FASTCALL CubicInterpolate(const TVT_ARRAY_T& a, IDTYPE i, double t) { return CubicBSpline<ANI_TID(VALUE)>(t,a[i-1-(i!=1)].value,a[i-1].value,a[i].value,a[i+((i+1)!=a.Size())].value); }

  protected:
    BSS_FORCEINLINE void _setval(ANI_TID(VALUECONST) val) const { AniAttributeDiscrete<TypeID>::_del(_rel?val+_initval:val); }

    ANI_TID(VALUE) _initval;
    const ANI_TID(VALUE)* _pval;
    bool _rel;
    FUNC _func;
  };

  // Discrete animation with an interval. After the interval has passed, the object is removed using a second delegate function.
  template<unsigned char TypeID>
  struct BSS_COMPILER_DLLEXPORT AniAttribute_Interval : AniAttributeDiscrete<TypeID>
  {
  public:
    typedef typename AniAttributeT<TypeID>::IDTYPE IDTYPE;
    using AniAttributeT<TypeID>::_timevalues;
    using AniAttributeT<TypeID>::_curpair;
    
    inline AniAttribute_Interval(const AniAttribute_Interval& copy) : AniAttributeDiscrete<TypeID>(copy), _rmdel(copy._rmdel), _toduration(copy.toduration) {}
    inline AniAttribute_Interval(ANI_TID(DELEGATE) del, delegate<void,ANI_TID(AUXTYPE)> rmdel, double (*toduration)(ANI_TID(VALUECONST))) : 
      AniAttributeDiscrete<TypeID>(del), _rmdel(rmdel), _toduration(toduration) {}
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
		virtual double SetKeyFrames(const KeyFrame<TypeID>* frames, IDTYPE num) { AniAttributeDiscrete<TypeID>::SetKeyFrames(frames,num); _recalclength(); return _length; }
		virtual IDTYPE AddKeyFrame(const KeyFrame<TypeID>& frame) //time is given in milliseconds
		{
      AniAttributeDiscrete<TypeID>::AddKeyFrame(frame);
      double t = frame.time+_toduration(frame.value);
      if(t>_length) _length=t;
		}
    virtual bool RemoveKeyFrame(IDTYPE ID)
    {
      if(ID<_timevalues.Size() && _length==_timevalues[ID].time+_toduration(_timevalues[ID].value)) _recalclength();
      return AniAttributeDiscrete<TypeID>::RemoveKeyFrame(ID);
    }
    inline virtual void Start() { _queue.Clear(); _curpair=1; if(AniAttributeT<TypeID>::_initzero) _addtoqueue(_timevalues[0].value); }
		inline virtual AniAttribute* BSS_FASTCALL Clone() const { return new AniAttribute_Interval(*this); }
    inline virtual void BSS_FASTCALL CopyAnimation(AniAttribute* ptr) { operator=(*static_cast<AniAttribute_Interval*>(ptr)); }
    inline AniAttribute_Interval& operator=(const AniAttribute_Interval& right) { AniAttributeDiscrete<TypeID>::operator=(right); _rmdel=right._rmdel; return *this; }

  protected:
    inline void _recalclength()
    {
      _length=0;
      double t;
      for(IDTYPE i = 0; i < _timevalues.Size(); ++i)
        if((t=_timevalues[i].time+_toduration(_timevalues[i].value))>_length)
          _length=t;
    }
    inline void _addtoqueue(ANI_TID(VALUECONST) v) { _queue.Push(_toduration(v),_del(v)); }

    double (*_toduration)(ANI_TID(VALUECONST));
    delegate<void,ANI_TID(AUXTYPE)> _rmdel; //delegate for removal
    cPriorityQueue<double,ANI_TID(AUXTYPE)> _queue;
    double _length;
  };
    
  // Generic attribute definition
  template<unsigned char TypeID>
  struct BSS_COMPILER_DLLEXPORT AttrDef : cDef<AniAttribute>
  { 
    inline virtual AniAttribute* BSS_FASTCALL Spawn() const { return new AniAttributeGeneric<TypeID>(); } 
    inline virtual AttrDef* BSS_FASTCALL Clone() const { return new AttrDef(*this); } 
  };

  // Discrete attribute definition
  template<unsigned char TypeID>
  struct BSS_COMPILER_DLLEXPORT AttrDefDiscrete : cDef<AniAttribute>
  { 
    inline AttrDefDiscrete(ANI_TID(DELEGATE) del) : _del(del) {}
    inline virtual AniAttribute* BSS_FASTCALL Spawn() const { return new AniAttributeDiscrete<TypeID>(_del); } 
    inline virtual AttrDefDiscrete* BSS_FASTCALL Clone() const { return new AttrDefDiscrete(*this); } 
    ANI_TID(DELEGATE) _del;
  };

  // Smooth attribute definition
  template<unsigned char TypeID, typename AniAttributeT<TypeID>::FUNC Func>
  struct BSS_COMPILER_DLLEXPORT AttrDefSmooth : AttrDefDiscrete<TypeID>
  { 
    inline AttrDefSmooth(ANI_TID(DELEGATE) del, const ANI_TID(VALUE)* src=0) : AttrDefDiscrete<TypeID>(del), _src(src) {}
    inline virtual AniAttribute* BSS_FASTCALL Spawn() const { return new AniAttributeSmooth<TypeID>(AttrDefDiscrete<TypeID>::_del,Func,_src); }
    inline virtual AttrDefSmooth* BSS_FASTCALL Clone() const { return new AttrDefSmooth(*this); } 
    const ANI_TID(VALUE)* _src;
  };

}

#endif
