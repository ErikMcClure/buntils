// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __ANI_ATTRIBUTE__BSS__
#define __ANI_ATTRIBUTE__BSS__

#include "AniTypeID.h"
#include "cArraySimple.h"

namespace bss_util {
  // Pair that stores the time and data of a given animation keyframe
  template<unsigned char TypeID>
  struct BSS_COMPILER_DLLEXPORT KeyFrame
  {
    KeyFrame(double time, ANI_TID(DATACONST) value) : time(time), value(value) {}
    KeyFrame() : time(0.0) {}
    double time;
    ANI_TID(DATA) value;
  };

  // Base AniAttribute definition
  struct BSS_COMPILER_DLLEXPORT AniAttribute
  {
    typedef unsigned short IDTYPE;

		AniAttribute(unsigned char _typeID) : typeID(_typeID) {}
		virtual ~AniAttribute() {}
		inline virtual bool Interpolate(double timepassed)=0;
		inline virtual void Start()=0;
    inline virtual void Finish()=0; //Ensures the source is set to the final value
		inline virtual double Length()=0;
		inline virtual AniAttribute* BSS_FASTCALL Clone() const { return 0; }
    inline virtual void BSS_FASTCALL CopyAnimation(AniAttribute* ptr)=0;
    inline virtual bool SetRelative(bool relative) { return false; }

		unsigned char typeID;
  };
  
  // This is a static allocator that redirects all dynamic array allocations to the static functions in cAbstractAnim
  template<typename T>
  struct AniStaticAlloc : StaticAllocPolicy<T,void>
  {
    typedef typename StaticAllocPolicy<T,void>::pointer pointer;
    inline static pointer allocate(std::size_t cnt, typename std::allocator<void>::const_pointer = 0) { return reinterpret_cast<pointer>(cAbstractAnim::AnimAlloc(cnt)); }
    inline static void deallocate(pointer p, std::size_t = 0) { cAbstractAnim::AnimFree(p); }
    inline static pointer reallocate(pointer p, std::size_t cnt) { return reinterpret_cast<pointer>(cAbstractAnim::AnimAlloc(cnt,p)); }
  };

  template<unsigned char TypeID, typename T> struct ANI_ATTR__SAFE__ { typedef cArrayWrap<cArraySafe<KeyFrame<TypeID>,AniAttribute::IDTYPE,AniStaticAlloc<KeyFrame<TypeID>>>> TVT_ARRAY; };
  template<unsigned char TypeID> struct ANI_ATTR__SAFE__<TypeID,void> { typedef cArrayWrap<cArraySimple<KeyFrame<TypeID>,AniAttribute::IDTYPE,AniStaticAlloc<KeyFrame<TypeID>>>> TVT_ARRAY; };

  // Abstract base implementation of an AniAttribute
  template<unsigned char TypeID>
  class BSS_COMPILER_DLLEXPORT AniAttributeT : public AniAttribute
  {
  public:
    typedef typename AniAttribute::IDTYPE IDTYPE;
    typedef typename ANI_ATTR__SAFE__<TypeID, typename ANI_IDTYPE<TypeID>::SAFE>::TVT_ARRAY TVT_ARRAY_T;
    
    AniAttributeT(const AniAttributeT& copy) : AniAttribute(copy), _timevalues(copy._timevalues), _curpair(copy._curpair),
      _initzero(copy._initzero)
		{
      assert(_timevalues.Size()>0);
		}
    AniAttributeT() : AniAttribute(TypeID), _curpair(1), _timevalues(1), _initzero(false)
    {
      _timevalues[0].time=0; //Must be set to zero in case Length() is called with no entries.
    }
    inline virtual double Length() { return _timevalues.Back().time; }
		inline virtual AniAttribute* BSS_FASTCALL Clone() const { return 0; }
    inline virtual void BSS_FASTCALL CopyAnimation(AniAttribute* ptr) { operator=(*static_cast<AniAttributeT*>(ptr)); }
    inline virtual bool SetInterpolation(ANI_TID(VALUE) (BSS_FASTCALL *func)(const TVT_ARRAY_T&,IDTYPE, double)) { return false; }
    inline IDTYPE GetNumFrames() const { return _timevalues.Size(); }
    inline const KeyFrame<TypeID>& GetKeyFrame(IDTYPE index) const { return _timevalues[index]; }
		inline IDTYPE AddKeyFrame(const KeyFrame<TypeID>& frame) //time is given in milliseconds
		{
      double time = frame.time;
      if(time==0.0)
      {
        _initzero=true;
        _timevalues[0].value=frame.value;
        return 0;
      }
      IDTYPE i;
      IDTYPE svar=_timevalues.Size(); //doesn't change
      for(i=0; i<svar; ++i)
        if(time<=_timevalues[i].time)
          break;

      if(time==_timevalues[i].time)
        _timevalues[i].value=frame.value;
      else
        _timevalues.Insert(frame,i);
      return i;
		}
    inline bool RemoveKeyFrame(IDTYPE ID)
    {
      if(ID>=_timevalues.Size() || _timevalues.Size()<=1) return false;
      _timevalues.Remove(ID);
      return true;
    }
		inline AniAttributeT& operator=(const AniAttributeT& right)
		{
      _initzero=right._initzero;
      _timevalues=right._timevalues;
      _curpair=right._curpair;
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
    using AniAttributeT<TypeID>::_initzero;

    AniAttributeGeneric(const AniAttributeGeneric& copy) : AniAttributeT<TypeID>(copy) {}
    AniAttributeGeneric() {}
    inline virtual bool Interpolate(double timepassed)
    {
      IDTYPE svar=_timevalues.Size();
      while(_curpair<svar && _timevalues[_curpair].time < timepassed);
        //if(timehold>0.0) //Time 0.0 was already applied, but _curpair starts at 1.
          _timevalues[_curpair++].value(); //You have to call ALL events even if you missed some because you don't know which ones do what
      
      if(_curpair>=svar) { Finish(); return false; } //Resolve the animation
			return true;
    }
    inline virtual void Start() { _curpair=1; if(_initzero) _timevalues[0].value(); }
    inline virtual void Finish()
    { 
      IDTYPE svar=_timevalues.Size();
      while(_curpair<svar)
        _timevalues[_curpair++].value();
    }
		inline virtual AniAttribute* BSS_FASTCALL Clone() const { return new AniAttributeGeneric(*this); }
		inline AniAttributeGeneric& operator=(const AniAttributeGeneric& right)
		{
      AniAttributeT<TypeID>::operator=(right);
      return *this;
		}
  };

  // Discrete attribute definition
  template<unsigned char TypeID>
  class BSS_COMPILER_DLLEXPORT AniAttributeDiscrete : public AniAttributeT<TypeID>
  {
  public:
    typedef typename AniAttributeT<TypeID>::TVT_ARRAY_T TVT_ARRAY_T;
    typedef typename AniAttributeT<TypeID>::IDTYPE IDTYPE;
    typedef ANI_TID(DELEGATE) DELEGATE;
    using AniAttributeT<TypeID>::_timevalues;
    using AniAttributeT<TypeID>::_curpair;
    using AniAttributeT<TypeID>::_initzero;
    
    AniAttributeDiscrete(const AniAttributeDiscrete& copy) : AniAttributeT<TypeID>(copy), _del(copy._del) {}
    AniAttributeDiscrete(DELEGATE del) : AniAttributeT<TypeID>(), _del(del) {}
    inline virtual bool Interpolate(double timepassed)
    {
      IDTYPE svar=_timevalues.Size();
      while(_curpair<svar && _timevalues[_curpair].time < timepassed) ++_curpair;
      if(_curpair>=svar) { Finish(); return false; } //Resolve the animation
      _del(_timevalues[_curpair].value);
			return true;
    }
    inline virtual void Start() { _curpair=1; if(_initzero) _del(_timevalues[0].value); }
    inline virtual void Finish() { if(_timevalues.Size()>1 || _initzero) _del(_timevalues.Back().value); }
		inline virtual AniAttribute* BSS_FASTCALL Clone() const { return new AniAttributeDiscrete(*this); }
    inline virtual void BSS_FASTCALL CopyAnimation(AniAttribute* ptr) { operator=(*static_cast<AniAttributeDiscrete*>(ptr)); }
    inline AniAttributeDiscrete& operator=(const AniAttributeDiscrete& right) { AniAttributeT<TypeID>::operator=(right); return *this; }
    
  protected:
    DELEGATE _del;
  };

  // Continuous attribute definition supporting relative animations
  template<unsigned char TypeID>
  class BSS_COMPILER_DLLEXPORT AniAttributeSmooth : public AniAttributeDiscrete<TypeID>
  {
  public:
    typedef typename AniAttributeDiscrete<TypeID>::TVT_ARRAY_T TVT_ARRAY_T;
    typedef typename AniAttributeDiscrete<TypeID>::IDTYPE IDTYPE;
    typedef typename AniAttributeDiscrete<TypeID>::DELEGATE DELEGATE;
    typedef ANI_TID(VALUE) (BSS_FASTCALL *FUNC)(const TVT_ARRAY_T&,IDTYPE, double);
    using AniAttributeDiscrete<TypeID>::_timevalues;
    using AniAttributeDiscrete<TypeID>::_curpair;
    using AniAttributeDiscrete<TypeID>::_initzero;

    AniAttributeSmooth(const AniAttributeSmooth& copy) : AniAttributeDiscrete<TypeID>(copy), _rel(false) {}
    AniAttributeSmooth(const ANI_TID(VALUE)* pval, FUNC func, DELEGATE del) : AniAttributeDiscrete<TypeID>(del), _func(func), _pval(pval), _rel(false) {}
    inline virtual bool Interpolate(double timepassed)
    {
      IDTYPE svar=_timevalues.Size();
      while(_curpair<svar && _timevalues[_curpair].time<timepassed) ++_curpair;
      if(_curpair>=svar) { Finish(); return false; } //Resolve the animation
      _setval(_func(_timevalues,_curpair,timepassed));
			return true;
    }
    inline virtual void Start() { _curpair=1; _initval=*_pval; if(_initzero) _setval(_func(_timevalues,_curpair,0.0)); }
    inline virtual void Finish() { IDTYPE svar=_timevalues.Size(); if(svar>1 || _initzero) _setval(_func(_timevalues,svar,AniAttributeDiscrete<TypeID>::Length())); }
		inline virtual AniAttribute* BSS_FASTCALL Clone() const { return new AniAttributeDiscrete<TypeID>(*this); }
    inline virtual void BSS_FASTCALL CopyAnimation(AniAttribute* ptr) { operator=(*static_cast<AniAttributeSmooth*>(ptr)); }
    inline virtual bool SetRelative(bool relative) { _rel=relative; return true; }
    inline virtual bool SetInterpolation(FUNC func) { _func=func; return true; }
    inline AniAttributeSmooth& operator=(const AniAttributeSmooth& right) { 
      AniAttributeDiscrete<TypeID>::operator=(right);
      _initval=right._initval;
      _func=right._func;
      _rel=right._rel;
      return *this;
    }

  protected:
    BSS_FORCEINLINE void _setval(ANI_TID(VALUECONST) val) const {  AniAttributeDiscrete<TypeID>::_del(_rel?val+_initval:val); } 

    ANI_TID(VALUE) _initval;
    const ANI_TID(VALUE)* _pval;
    FUNC _func;
    bool _rel;
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
  template<unsigned char TypeID, typename AniAttributeSmooth<TypeID>::FUNC Func>
  struct BSS_COMPILER_DLLEXPORT AttrDefSmooth : AttrDefDiscrete<TypeID>
  { 
    inline AttrDefSmooth(const ANI_TID(VALUE)* src, ANI_TID(DELEGATE) del) : AttrDefDiscrete<TypeID>(del), _src(src) {}
    inline virtual AniAttribute* BSS_FASTCALL Spawn() const { return new AniAttributeSmooth<TypeID>(_src,Func,AttrDefDiscrete<TypeID>::_del); }
    inline virtual AttrDefSmooth* BSS_FASTCALL Clone() const { return new AttrDefSmooth(*this); } 
    const ANI_TID(VALUE)* _src;
  };

}

#endif
