// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_ANIMATION_H__BSS__
#define __C_ANIMATION_H__BSS__

#include "AniAttribute.h"
#include "cMap.h"
#include "cDynArray.h"
#include "cBitField.h"

namespace bss_util {
  struct DEF_ANIMATION;

  // A class representing a single animation comprised of various attributes (position, rotation, etc.) 
  class BSS_COMPILER_DLLEXPORT cAnimation
	{
  protected:
    enum ANIBOOLS : unsigned char { ANI_PLAYING=1,ANI_LOOPING=2,ANI_PAUSED=4 };

	public:
		inline cAnimation(const cAnimation& copy) : _parent(copy._parent) { operator=(copy); }
		inline cAnimation(const cAnimation& copy, cAbstractAnim* ptr) : _parent(ptr) { operator=(copy); }
    inline cAnimation(cAbstractAnim* ptr) : _parent(ptr), _aniwarp(1.0), _anilength(0), _timepassed(0), _anicalc(0), _looppoint(0) { }
		inline cAnimation(const DEF_ANIMATION& def, cAbstractAnim* ptr);
		inline ~cAnimation()
    {
      for(unsigned char i = 0; i < _attributes.Length(); ++i)
        _delattr(_attributes[i]);
      _attributes.Clear();
    }
    // Allows you to skip forward by setting _timepassed to the specified value.
		inline virtual void Start(double timepassed=0.0)
    {
	    _timepassed=timepassed;
      _anibool+=ANI_PLAYING;
  
      unsigned char svar=_attributes.Length();
      for(unsigned char i = 0; i < svar; ++i)
        _attributes[i]->Start();
    }
    // Stop animation
    inline void Stop() { _anibool-=(ANI_LOOPING|ANI_PLAYING); _timepassed=0.0; }
    // Temporarily pauses or unpauses the animation
    inline void Pause(bool pause) { _anibool[ANI_PAUSED]=pause; }
    // Starts looping the animation (if it hasn't started yet, it is started.) Looping ends when Stop() is called
    inline void Loop(double timepassed=0.0, double looppoint=0.0) { _looppoint=looppoint; _anibool+=ANI_LOOPING; if((_anibool&ANI_PLAYING)==0) Start(timepassed); }
    // Interpolates the animation by moving it forward *delta* milliseconds
    inline bool Interpolate(double delta)
    {
      if((_anibool&ANI_PAUSED)!=0) return true;
      if((_anibool&ANI_PLAYING)==0) return false;
	    _timepassed += delta*_aniwarp;
		  double length = _anilength==0.0?_anicalc:_anilength;

		  if(((_anibool&ANI_LOOPING)!=0) && length>0.0 && _timepassed > length)
        Start(fmod(_timepassed-length,length-_looppoint)+_looppoint);

	    bool notfinished=_timepassed<_anilength;

      unsigned char svar=_attributes.Length();
      for(unsigned char i = 0; i < svar; ++i)
        if(_attributes[i]->Interpolate(_timepassed))
          notfinished=true;

	    if(!notfinished) //in this case we're done, so reset
      {
		    if((_anibool&ANI_LOOPING)!=0) Start(fmod(_timepassed-length,length-_looppoint)+_looppoint);
		    else Stop();
      }

      return (_anibool&ANI_PLAYING)!=0;
    }
    inline AniAttribute* GetTypeID(unsigned char TypeID)
    {
      unsigned char index=_attributes.Get(TypeID);
      if(index<_attributes.Length()) return _attributes[index];
      AniAttribute* retval=_parent->TypeIDRegFunc(TypeID);
      if(retval!=0) _attributes.Insert(retval->typeID,retval);
      return retval;
    }
		template<unsigned char TypeID>
		inline bool SetInterpolation(typename AniAttributeSmooth<TypeID>::FUNC interpolation)
		{
			AniAttribute* hold = GetTypeID(TypeID);
			if(hold) return static_cast<AniAttributeT<TypeID>*>(hold)->SetInterpolation(interpolation);
      return false;
		}
		template<unsigned char TypeID>
		inline bool SetRelative(bool rel)
		{
			AniAttribute* hold = GetTypeID(TypeID);
			if(hold) return static_cast<AniAttributeT<TypeID>*>(hold)->SetRelative(rel);
      return false;
		}
    template<unsigned char TypeID>
		inline AniAttribute::IDTYPE AddKeyFrame(const KeyFrame<TypeID>& frame)
		{
			AniAttributeT<TypeID>* hold = static_cast<AniAttributeT<TypeID>*>(GetTypeID(TypeID));
      if(hold)
			{
        AniAttribute::IDTYPE retval = hold->AddKeyFrame(frame);
				if(hold->Length() > _anicalc)
					_anicalc = hold->Length();
				return retval;
			}
      return (AniAttribute::IDTYPE)-1;
		}
		template<unsigned char TypeID>
    inline AniAttribute::IDTYPE AddKeyFrame(double time, ANI_TID(DATACONST) value) { return AddKeyFrame<TypeID>(KeyFrame<TypeID>(time,value)); };
		template<unsigned char TypeID>
    inline bool RemoveKeyFrame(AniAttribute::IDTYPE ID)
		{ //This is inline because the adding function is inline, and we must preserve the DLL we're adding/deleting things from
      bool retval=false;
      unsigned char index = _attributes.Get(TypeID);
      if(index<_attributes.Length()) retval=static_cast<AniAttributeT<TypeID>*>(_attributes[index])->RemoveKeyFrame(ID);
      else return false;

			_anicalc=0.0;
      unsigned char svar=_attributes.Length();
      for(unsigned char i = 0; i < svar; ++i)
        if(_anicalc>_attributes[i]->Length())
					_anicalc=_attributes[i]->Length();

			return retval;
		}
    inline bool HasTypeID(unsigned char TypeID) const { return _attributes.Get(TypeID)<_attributes.Length(); }
    inline void SetAnimationLength(double anilength) { _anilength=anilength; }
		inline double GetAnimationLength() const { return _anilength==0.0?_anicalc:_anilength; }
		inline void SetTimeWarp(double aniwarp) { _aniwarp=aniwarp; }
		inline double GetTimeWarp() const { return _aniwarp; }
    inline double GetTimePassed() const { return _timepassed; }
    inline bool IsPlaying() const { return (_anibool&ANI_PLAYING)!=0; }
    inline bool IsLooping() const { return (_anibool&ANI_LOOPING)!=0; }
    inline bool IsPaused() const { return (_anibool&ANI_PAUSED)!=0; }
    
    inline cAnimation& operator +=(const cAnimation& add)
    {
      unsigned char svar=_attributes.Length(); 
      unsigned char rvar=add._attributes.Length();
      unsigned char j=0;
      for(unsigned char i = 0; i < rvar;)
      {
        if(j>=svar) //We've run off the end of our existing IDs so simply append the rest
          _appendattribute(add._attributes[i++]);
        else
          switch(SGNCOMPARE(_attributes[j]->typeID,add._attributes[i]->typeID))
          {
          case -1:
            ++j;
            break;
          case 0: // They are the same, so add them together
            _attributes[j]->AddAnimation(add._attributes[i]);
            break;
          case 1: // If this happens, we skipped over one of the IDs in the right, so just append it.
            _appendattribute(add._attributes[i++]);
          }
      }
      return *this;
    }
    inline const cAnimation operator +(const cAnimation& add) const
    {
      cAnimation retval(*this);
      retval+=add;
      return retval;
    }
		inline cAnimation& operator=(const cAnimation& right)
    {
      for(unsigned char i = 0; i < _attributes.Length(); ++i)
        _delattr(_attributes[i]);
      _attributes.Clear();

	    _aniwarp=right._aniwarp;
      _anibool=right._anibool;
	    _timepassed=right._timepassed;
	    _anicalc=right._anicalc;
	    _anilength=right._anilength;
      _looppoint=right._looppoint;

      unsigned char svar=right._attributes.Length(); 
      for(unsigned char i = 0; i < svar; ++i)
        _appendattribute(right._attributes[i]);

	    return *this;
    }

    static inline void _delattr(AniAttribute* p) { p->~AniAttribute(); cAbstractAnim::AnimFree(p); }

	protected:
    bss_util::cMap<unsigned char,AniAttribute*,bss_util::CompT<unsigned char>,unsigned char,cArraySimple<std::pair<unsigned char,AniAttribute*>,unsigned char,AniStaticAlloc<std::pair<unsigned char,AniAttribute*>>>> _attributes;
		double _timepassed; //in milliseconds
		double _anicalc; //Calculated effective length
		double _anilength; //if zero, we automatically stop when all animations are exhausted
		double _aniwarp; //Time warp factor
    cAbstractAnim* _parent;
    cBitField<unsigned char> _anibool;
    double _looppoint;

    BSS_FORCEINLINE void _appendattribute(AniAttribute* attr)
    {
      AniAttribute* cln = _parent->TypeIDRegFunc(attr->typeID);
      if(!cln) return; // This will happen if we copied from a different object, or in the copy constructor
      cln->CopyAnimation(attr);
      _attributes.Insert(cln->typeID,cln);
    }
	};
  
  struct DEF_ANIATTRIBUTE
  {
    virtual ~DEF_ANIATTRIBUTE() {}
		inline virtual DEF_ANIATTRIBUTE* BSS_FASTCALL Clone() const=0;
		inline virtual void BSS_FASTCALL Load(cAnimation* ani) const=0;
  };
  template<unsigned char TypeID>
  struct DEF_ANIATTRIBUTE_T : DEF_ANIATTRIBUTE, bss_util::cDynArray<bss_util::cArraySimple<KeyFrame<TypeID>>>
  {
    DEF_ANIATTRIBUTE_T() : rel(false),func(0) {}
    typedef bss_util::cDynArray<bss_util::cArraySimple<KeyFrame<TypeID>>> BASE;
    inline virtual DEF_ANIATTRIBUTE_T* BSS_FASTCALL Clone() const { return new DEF_ANIATTRIBUTE_T(*this); }
    inline virtual void BSS_FASTCALL Load(cAnimation* ani) const 
    { 
      ani->SetInterpolation<TypeID>(func);
      ani->SetRelative<TypeID>(rel);
      for(unsigned int i = 0; i < BASE::_length; ++i) 
        ani->AddKeyFrame<TypeID>(BASE::_array[i]); 
    }
    bool rel;
    typename AniAttributeSmooth<TypeID>::FUNC func;
  };

	struct DEF_ANIMATION : cDef<cAnimation>
	{
    typedef unsigned char ST_;
		inline DEF_ANIMATION() : anilength(0.0), aniwarp(1.0) {}
    inline DEF_ANIMATION(DEF_ANIMATION&& mov) : anilength(mov.anilength), aniwarp(mov.aniwarp), _frames(std::move(mov._frames)) { }
    inline DEF_ANIMATION(const DEF_ANIMATION& copy) : anilength(copy.anilength), aniwarp(copy.aniwarp), _frames(copy._frames)
    {
      for(ST_ i = 0; i < _frames.Length(); ++i) 
        _frames[i]=_frames[i]->Clone();
    }
		inline ~DEF_ANIMATION() { for(ST_ i = 0; i < _frames.Length(); ++i) delete _frames[i]; }
		inline virtual cAnimation* BSS_FASTCALL Spawn() const { return 0; } // You can't spawn an animation because it can't attach to anything
		inline virtual DEF_ANIMATION* BSS_FASTCALL Clone() const { return new DEF_ANIMATION(*this); }
    template<ST_ TypeID>
    inline DEF_ANIATTRIBUTE_T<TypeID>* BSS_FASTCALL GetAttribute()
    { 
      ST_ ind = _frames.Get(TypeID);
      if(ind>=_frames.Length()) 
      {
        _frames.Insert(TypeID,new DEF_ANIATTRIBUTE_T<TypeID>());
        ind = _frames.Get(TypeID);
        assert(ind<_frames.Length());
      }
      return static_cast<DEF_ANIATTRIBUTE_T<TypeID>*>(_frames[ind]);
    }
    template<ST_ TypeID>
    inline unsigned int AddFrame(const KeyFrame<TypeID>& frame) { return GetAttribute<TypeID>()->Add(frame); }
    template<ST_ TypeID>
    inline bool RemoveFrame(unsigned int i) { auto p = GetAttribute<TypeID>(); if(i>=p->Length()) return false; p->Remove(i); return true; }
    inline const bss_util::cMap<ST_,DEF_ANIATTRIBUTE*,bss_util::CompT<ST_>,ST_>& GetFrames() const { return _frames; }

		double anilength; //if zero, we automatically stop when all animations are exhausted
		double aniwarp; //Time warp factor

	private:
    bss_util::cMap<ST_,DEF_ANIATTRIBUTE*,bss_util::CompT<ST_>,ST_> _frames;
	};
  
  inline cAnimation::cAnimation(const DEF_ANIMATION& def, cAbstractAnim* ptr) : _parent(ptr), _aniwarp(def.aniwarp), _looppoint(0), _anicalc(0),
      _anilength(def.anilength), _timepassed(0)
    {
      auto& p = def.GetFrames();
      for(unsigned int i = 0; i < p.Length(); ++i)
        p[i]->Load(this);
    }
}

#endif
