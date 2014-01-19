// Copyright ©2*013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __ANI_ATTRIBUTE__BSS__
#define __ANI_ATTRIBUTE__BSS__

#include "delegate.h"
#include "cDef.h"
#include "cPriorityQueue.h"
#include "bss_algo.h"
#include "cBitField.h"
#include "cBitStream.h"

#define ANI_TID(tdef) typename bss_util::ANI_IDTYPE<TypeID>::TYPES::tdef

namespace bss_util {
  template<unsigned char T> //if you need your own type, just insert another explicit specialization in your code
  struct ANI_IDTYPE { typedef struct { typedef void TYPE; } TYPES; }; 

  template<typename E, typename V, typename D = V, typename S = void, typename A = void, typename DEL = delegate<A, V>>
  struct ANI_IDTYPE_TYPES {
    typedef E TYPE;
    typedef V VALUE;
    typedef V const& VALUECONST;
    typedef D DATA;
    typedef DATA const& DATACONST;
    typedef S SAFE;
    typedef A AUX;
    typedef DEL DELEGATE;
  };

  // Pair that stores the time and data of a given animation keyframe
  template<unsigned char TypeID>
  struct BSS_COMPILER_DLLEXPORT KeyFrame
  {
    inline KeyFrame(double time, ANI_TID(DATACONST) value) : time(time), value(value) {}
    inline KeyFrame() : time(0.0) {}
    double time;
    ANI_TID(DATA) value;
  };

  template<unsigned char TypeID>
  inline static void KeyFrame_Serialize(const KeyFrame<TypeID>& k, std::ostream& s)
  {
    bss_Serialize(k.time,s);
    bss_Serialize(k.value,s);
  }

  template<unsigned char TypeID>
  inline static void KeyFrame_Deserialize(KeyFrame<TypeID>& k, std::istream& s)
  {
    bss_Deserialize(k.time,s);
    bss_Deserialize(k.value,s);
  }
  
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

		unsigned char typeID;
  };

  template<unsigned char TypeID, typename Alloc, typename T> struct ANI_ATTR__SAFE__ { typedef cArraySafe<KeyFrame<TypeID>, AniAttribute::IDTYPE, Alloc> INNER_ARRAY; };
  template<unsigned char TypeID, typename Alloc> struct ANI_ATTR__SAFE__<TypeID, Alloc, void> { typedef cArraySimple<KeyFrame<TypeID>, AniAttribute::IDTYPE, Alloc> INNER_ARRAY; };

  // Abstract base implementation of an AniAttribute
  template<unsigned char TypeID, typename Alloc=StaticAllocPolicy<char>>
  class BSS_COMPILER_DLLEXPORT AniAttributeT : public AniAttribute
  {
  public:
    template<typename U> struct rebind { typedef AniAttributeT<TypeID,U> other; };
    typedef typename AniAttribute::IDTYPE IDTYPE;
    typedef cArrayWrap<typename ANI_ATTR__SAFE__<TypeID, typename Alloc::template rebind<KeyFrame<TypeID>>::other, ANI_TID(SAFE)>::INNER_ARRAY> TVT_ARRAY_T;
    typedef ANI_TID(VALUE) (BSS_FASTCALL *FUNC)(const TVT_ARRAY_T&,IDTYPE, double);
    enum ATTR_FLAGS : unsigned char { ATTR_INITZERO=1, ATTR_REL=2, ATTR_ATTACHED=4 };
    
    inline AniAttributeT(const AniAttributeT& copy) : AniAttribute(copy), _timevalues(copy._timevalues), _curpair(copy._curpair),
      _flags(copy._flags)
		{
      assert(_timevalues.Size()>0);
		}
    inline AniAttributeT() : AniAttribute(TypeID), _curpair(1), _timevalues(0), _flags(0) { Clear(); }
    inline virtual double Length() { return _timevalues.Back().time; }
		inline virtual AniAttribute* BSS_FASTCALL Clone() const { return 0; }
    inline virtual void BSS_FASTCALL CopyAnimation(AniAttribute* ptr) { operator=(*static_cast<AniAttributeT*>(ptr)); }
    inline virtual void BSS_FASTCALL AddAnimation(AniAttribute* ptr) { operator+=(*static_cast<AniAttributeT*>(ptr)); }
    inline virtual bool SetInterpolation(FUNC f) { return false; }
    inline virtual bool SetRelative(bool rel) { return false; } // If set to non-zero, this will be relative.
    inline IDTYPE GetNumFrames() const { return _timevalues.Size(); }
    inline const KeyFrame<TypeID>& GetKeyFrame(IDTYPE index) const { return _timevalues[index]; }
    inline void Clear() { _timevalues.SetSize(1); _timevalues[0].time=0; _flags-=ATTR_INITZERO; }
		virtual double SetKeyFrames(const KeyFrame<TypeID>* frames, IDTYPE num)
    {
      if(!num || !frames)
        Clear();
      else if(_flags[ATTR_INITZERO] = (frames[0].time==0.0)) // If the array includes 0, we can just copy it over directly
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
        _flags+=ATTR_INITZERO;
        _timevalues[0].value=frame.value;
        return 0;
      }
      IDTYPE i;
      IDTYPE svar=_timevalues.Size(); //doesn't change
      for(i=svar; (i--)>0;) // We go through this backwards because it's more efficient when we're adding things in order
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
      if(_timevalues.Size()<=1) _flags-=ATTR_INITZERO;
      else _timevalues.Remove(ID);
      return true;
    }
		AniAttributeT& operator=(const AniAttributeT& right)
		{
      _flags[ATTR_INITZERO]=right._flags[ATTR_INITZERO];
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
    virtual void BSS_FASTCALL Serialize(std::ostream& s)
    {
      bss_Serialize<AniAttribute::IDTYPE>(_timevalues.Size(),s);
      bss_Serialize<unsigned char>(_flags&(~ATTR_ATTACHED),s);
      for(unsigned int i = 0; i < _timevalues.Size(); ++i)
        KeyFrame_Serialize<TypeID>(_timevalues[i],s);
    }
    virtual void BSS_FASTCALL Deserialize(std::istream& s)
    {
      AniAttribute::IDTYPE len;
      bss_Deserialize<AniAttribute::IDTYPE>(len,s);
      unsigned char hold;
      bss_Deserialize<unsigned char>(hold,s);
      _flags=((_flags&ATTR_ATTACHED)|hold);

      _timevalues.SetSize(len);
      for(unsigned int i = 0; i < len; ++i)
        KeyFrame_Deserialize<TypeID>(_timevalues[i],s);
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
  template<unsigned char TypeID, typename Alloc = StaticAllocPolicy<char>>
  class BSS_COMPILER_DLLEXPORT AniAttributeGeneric : public AniAttributeT<TypeID, Alloc>
  {
  public:
    template<typename U> struct rebind { typedef AniAttributeGeneric<TypeID, U> other; };
    using AniAttributeT<TypeID>::_timevalues;
    using AniAttributeT<TypeID>::_curpair;

    inline AniAttributeGeneric(const AniAttributeGeneric& copy) : AniAttributeT<TypeID>(copy) {}
    inline AniAttributeGeneric() {}
    virtual bool Interpolate(double timepassed)
    {
      auto svar=_timevalues.Size();
      while(_curpair<svar && _timevalues[_curpair].time <= timepassed)
        _timevalues[_curpair++].value(); //You have to call ALL events even if you missed some because you don't know which ones do what
      return _curpair<svar;
    }
    inline virtual void Start() { if(!_attached()) return; _curpair=1; if(AniAttributeT<TypeID>::_initzero()) _timevalues[0].value(); }
    inline virtual AniAttribute* BSS_FASTCALL Clone() const { return new(Alloc::allocate(sizeof(AniAttributeGeneric))) AniAttributeGeneric(*this); }
		inline AniAttributeGeneric& operator=(const AniAttributeGeneric& right) { AniAttributeT<TypeID>::operator=(right); return *this; }
  };

  // Discrete attribute definition
  template<unsigned char TypeID>
  struct BSS_COMPILER_DLLEXPORT AttrDefDiscrete : AttrDef { explicit AttrDefDiscrete(ANI_TID(DELEGATE) d) : del(d) {} ANI_TID(DELEGATE) del; };

  // Discrete attribute definition
  template<unsigned char TypeID, typename Alloc = StaticAllocPolicy<char>>
  class BSS_COMPILER_DLLEXPORT AniAttributeDiscrete : public AniAttributeT<TypeID, Alloc>
  {
  public:
    template<typename U> struct rebind { typedef AniAttributeDiscrete<TypeID, U> other; };
    using AniAttributeT<TypeID>::_timevalues;
    using AniAttributeT<TypeID>::_curpair;
    
    inline AniAttributeDiscrete(const AniAttributeDiscrete& copy) : AniAttributeT<TypeID>(copy), _del(copy._del) {}
    inline explicit AniAttributeDiscrete(ANI_TID(DELEGATE) del) : AniAttributeT<TypeID>(), _del(del) {}
    inline AniAttributeDiscrete() : AniAttributeT<TypeID>(), _del(0,0) {}
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
    inline virtual void Start() { if(!_attached()) return; _curpair=1; if(AniAttributeT<TypeID>::_initzero()) _del(_timevalues[0].value); }
    inline virtual AniAttribute* BSS_FASTCALL Clone() const { return new(Alloc::allocate(sizeof(AniAttributeDiscrete))) AniAttributeDiscrete(*this); }
    virtual void BSS_FASTCALL Attach(AttrDef* def) { _del=static_cast<AttrDefDiscrete<TypeID>*>(def)->del; _flags+=ATTR_ATTACHED; }
    
  protected:
    ANI_TID(DELEGATE) _del;
  };
  
  // Smooth attribute definition
  template<unsigned char TypeID>
  struct BSS_COMPILER_DLLEXPORT AttrDefSmooth : AttrDefDiscrete<TypeID> { 
    AttrDefSmooth(const ANI_TID(VALUE)* s, const ANI_TID(DELEGATE)& d) : AttrDefDiscrete<TypeID>(d), src(s) {} 
    const ANI_TID(VALUE)* src; 
  };

  // Continuous attribute definition supporting relative animations. pval is required only if relative animations are used. pval cannot be
  // NULL if you haven't supplied a value in the 0.0 time segment.
  template<unsigned char TypeID, typename Alloc = StaticAllocPolicy<char>>
  class BSS_COMPILER_DLLEXPORT AniAttributeSmooth : public AniAttributeDiscrete<TypeID, Alloc>
  {
  public:
    template<typename U> struct rebind { typedef AniAttributeSmooth<TypeID, U> other; };
    typedef typename AniAttributeT<TypeID>::TVT_ARRAY_T TVT_ARRAY_T;
    typedef typename AniAttributeT<TypeID>::IDTYPE IDTYPE;
    typedef typename AniAttributeT<TypeID>::FUNC FUNC;
    using AniAttributeT<TypeID>::_timevalues;
    using AniAttributeT<TypeID>::_curpair;

    inline AniAttributeSmooth(const AniAttributeSmooth& copy) : AniAttributeDiscrete<TypeID>(copy), _pval(0), _func(copy._func), _initval(false) {}
    inline AniAttributeSmooth(ANI_TID(DELEGATE) del, FUNC func=&NoInterpolate, const ANI_TID(VALUE)* pval=0, bool rel=false) :
    AniAttributeDiscrete<TypeID>(del), _func(func), _pval(pval), _initval(false) { _flags[ATTR_REL]=rel&&(_pval!=0); }
    inline AniAttributeSmooth() : AniAttributeDiscrete<TypeID>(), _func(&NoInterpolate), _pval(0), _initval(false) {}
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
      if(!_attached()) return;
      _curpair=1; 
      if(_pval) _initval=*_pval; 
      assert(AniAttributeT<TypeID>::_initzero() || _pval!=0); // You can have a _timevalues size of just 1, but only if you have interpolation disabled
      if(!AniAttributeT<TypeID>::_initzero()) 
        _timevalues[0].value=*_pval;
      _setval(_func(_timevalues,_curpair,0.0));
    }
    inline virtual AniAttribute* BSS_FASTCALL Clone() const { return new(Alloc::allocate(sizeof(AniAttributeSmooth))) AniAttributeSmooth(*this); }
    inline virtual void BSS_FASTCALL CopyAnimation(AniAttribute* ptr) { operator=(*static_cast<AniAttributeSmooth*>(ptr)); }
    inline virtual bool SetInterpolation(FUNC func) { if(!func) return false; _func=func; return true; }
    inline virtual bool SetRelative(bool rel) { if(!_pval) return false; _flags[ATTR_REL]=rel; return true; } // If set to non-zero, this will be relative.
    virtual void BSS_FASTCALL Attach(AttrDef* def) { _pval=static_cast<AttrDefSmooth<TypeID>*>(def)->src; AniAttributeDiscrete<TypeID>::Attach(def); }
    inline AniAttributeSmooth& operator=(const AniAttributeSmooth& right)
    { 
      AniAttributeDiscrete<TypeID>::operator=(right);
      _func=right._func;
      return *this;
    }

    static inline ANI_TID(VALUE) BSS_FASTCALL NoInterpolate(const TVT_ARRAY_T& a, IDTYPE i, double t) { return a[i-(t!=1.0)].value; }
    static inline ANI_TID(VALUE) BSS_FASTCALL LerpInterpolate(const TVT_ARRAY_T& a, IDTYPE i, double t) { return lerp<ANI_TID(VALUE)>(a[i-1].value,a[i].value,t); }
    static inline ANI_TID(VALUE) BSS_FASTCALL CubicInterpolate(const TVT_ARRAY_T& a, IDTYPE i, double t) { return CubicBSpline<ANI_TID(VALUE)>(t,a[i-1-(i!=1)].value,a[i-1].value,a[i].value,a[i+((i+1)!=a.Size())].value); }

  protected:
    BSS_FORCEINLINE void _setval(ANI_TID(VALUECONST) val) const { AniAttributeDiscrete<TypeID>::_del((_flags&ATTR_REL)?val+_initval:val); }

    ANI_TID(VALUE) _initval;
    const ANI_TID(VALUE)* _pval;
    FUNC _func;
  };
  
  // Interval attribute definition
  template<unsigned char TypeID>
  struct BSS_COMPILER_DLLEXPORT AttrDefInterval : AttrDefDiscrete<TypeID> { 
    AttrDefInterval(delegate<void, ANI_TID(AUX)> rm, ANI_TID(DELEGATE) d) : AttrDefDiscrete<TypeID>(d), rmdel(rm) {}
    delegate<void, ANI_TID(AUX)> rmdel;
  };

  // Discrete animation with an interval. After the interval has passed, the object is removed using a second delegate function.
  template<unsigned char TypeID, typename Alloc = StaticAllocPolicy<char>>
  struct BSS_COMPILER_DLLEXPORT AniAttributeInterval : AniAttributeDiscrete<TypeID, Alloc>
  {
  public:
    template<typename U> struct rebind { typedef AniAttributeInterval<TypeID, U> other; };
    typedef typename AniAttributeT<TypeID>::IDTYPE IDTYPE;
    typedef typename Alloc::template rebind<std::pair<double, ANI_TID(AUX)>>::other QUEUEALLOC;
    using AniAttributeT<TypeID>::_timevalues;
    using AniAttributeT<TypeID>::_curpair;
    
    inline AniAttributeInterval(const AniAttributeInterval& copy) : AniAttributeDiscrete<TypeID>(copy), _rmdel(copy._rmdel) {}
    inline AniAttributeInterval(ANI_TID(DELEGATE) del, delegate<void,ANI_TID(AUX)> rmdel) :
      AniAttributeDiscrete<TypeID>(del), _rmdel(rmdel) {}
    inline AniAttributeInterval() : AniAttributeDiscrete<TypeID>(), _rmdel(0, 0), _length(0) {}
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
		virtual double SetKeyFrames(const KeyFrame<TypeID>* frames, IDTYPE num) { AniAttributeDiscrete<TypeID>::SetKeyFrames(frames,num); _recalclength(); return _length; }
		virtual IDTYPE AddKeyFrame(const KeyFrame<TypeID>& frame) //time is given in milliseconds
		{
      IDTYPE r = AniAttributeDiscrete<TypeID>::AddKeyFrame(frame);
      double t = frame.time+bss_util::ANI_IDTYPE<TypeID>::toduration(frame.value);
      if(t>_length) _length=t;
      return r;
		}
    virtual bool RemoveKeyFrame(IDTYPE ID)
    {
      if(ID<_timevalues.Size() && _length==_timevalues[ID].time+bss_util::ANI_IDTYPE<TypeID>::toduration(_timevalues[ID].value)) _recalclength();
      return AniAttributeDiscrete<TypeID>::RemoveKeyFrame(ID);
    }
    inline virtual void Start() 
    {
      while(!_queue.Empty()) // Correctly remove everything currently on the queue
        _rmdel(_queue.Pop().second);
      if(!_attached()) return;
      _curpair=1;
      if(AniAttributeT<TypeID>::_initzero())
        _addtoqueue(_timevalues[0].value); 
    }
    inline virtual AniAttribute* BSS_FASTCALL Clone() const { return new(Alloc::allocate(sizeof(AniAttributeInterval))) AniAttributeInterval(*this); }
    inline virtual void BSS_FASTCALL CopyAnimation(AniAttribute* ptr) { operator=(*static_cast<AniAttributeInterval*>(ptr)); }
    virtual void BSS_FASTCALL Attach(AttrDef* def) { _rmdel = static_cast<AttrDefInterval<TypeID>*>(def)->rmdel; AniAttributeDiscrete<TypeID>::Attach(def); }
    inline AniAttributeInterval& operator=(const AniAttributeInterval& right) { AniAttributeDiscrete<TypeID>::operator=(right); _length=right._length; return *this; }

  protected:
    inline void _recalclength()
    {
      _length=0;
      double t;
      for(IDTYPE i = 0; i < _timevalues.Size(); ++i)
      if((t = _timevalues[i].time+bss_util::ANI_IDTYPE<TypeID>::toduration(_timevalues[i].value))>_length)
        _length=t;
    }
    inline void _addtoqueue(ANI_TID(DATACONST) v) { _queue.Push(bss_util::ANI_IDTYPE<TypeID>::toduration(v), _del(v)); }

    delegate<void,ANI_TID(AUX)> _rmdel; //delegate for removal
    cPriorityQueue<double, ANI_TID(AUX), CompT<double>, unsigned int, cArraySimple<std::pair<double, ANI_TID(AUX)>, unsigned int, QUEUEALLOC>> _queue;
    double _length;
  };
}

#endif
