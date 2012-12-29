// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_ANIMATION_H__PS__
#define __C_ANIMATION_H__PS__

#include "AniAttribute.h"
#include "cMap.h"
#include "cDynArray.h"
#include "cBitField.h"
#include "bss_dlldef.h"

namespace bss_util {
  class cAnimated;
  class Attribute_Def_base;
  struct DEF_ANIMATION;

  // A class representing a single animation comprised of various attributes (position, rotation, etc.) 
	class BSS_DLLEXPORT cAnimation
	{
    enum ANIBOOLS : unsigned char { ANI_PLAYING=1,ANI_LOOPING=2,ANI_ACTIVE=4,ANI_PAUSED=8 };

	public:
		inline cAnimation(const cAnimation& copy) : _parent(copy._parent) { operator=(copy); }
		inline cAnimation(const cAnimation& copy, cAbstractAnim* ptr) : _parent(ptr) { operator=(copy); }
    inline cAnimation(cAbstractAnim* ptr) : _parent(ptr), _aniwarp(1.0), _anilength(0), _timepassed(0), _anicalc(0), _looppoint(0) { }
		cAnimation(const DEF_ANIMATION& def, cAbstractAnim* ptr);
		~cAnimation();
    // Allows you to skip forward by setting _timepassed to the specified value.
		virtual void Start(double timepassed=0.0); 
    // Stop animation
    inline void Stop() { _anibool-=(ANI_ACTIVE|ANI_PAUSED); _timepassed=0.0; }
    // Temporarily pauses or unpauses the animation
    inline void Pause(bool pause) { _anibool(ANI_PAUSED,pause); }
    // Starts looping the animation (if it hasn't started yet, it is started.) Looping ends when Stop() is called
    inline void Loop(double timepassed=0.0, double looppoint=0.0) { _looppoint=looppoint; if(!_anibool[ANI_PLAYING]) Start(timepassed); _anibool(ANI_LOOPING,true); }
    // Interpolates the animation by moving it forward *delta* milliseconds
    inline bool Interpolate(double delta)
    {
      if((_anibool&ANI_PAUSED)!=0) return true;
      if((_anibool&ANI_PLAYING)==0) { _anibool-=ANI_ACTIVE; return false; }
	    _timepassed += delta*_aniwarp;

	    if((_anibool&ANI_LOOPING)!=0)
	    {
		    double length = _anilength==0.0?_anicalc:_anilength;
		    if(length>0.0 && _timepassed > length)
          Start((floor(_timepassed/length)*length)+_looppoint);
	    }

	    bool notfinished=_timepassed<_anilength;

      unsigned char svar=_attributes.Length();
      for(unsigned char i = 0; i < svar; ++i)
        if(_attributes[i]->Interpolate(_timepassed))
          notfinished=true;

	    if(!notfinished) //in this case we're done, so reset
      {
		    if((_anibool&ANI_LOOPING)!=0) Start(_looppoint);
		    else { Stop(); _anibool-=ANI_ACTIVE; }
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
			if(hold) static_cast<AniAttributeT<TypeID>*>(hold)->SetInterpolation(interpolation);
      else return false;
			return true;
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
		inline unsigned short GetID() const { return _id; }
		template<unsigned char TypeID>
    inline bool SetRelative(bool rel) { AniAttribute* hold = GetTypeID(TypeID); if(!hold) return; hold->_rel=rel; }
    inline bool IsPlaying() const { return (_anibool&ANI_PLAYING)!=0; }
    inline bool IsLooping() const { return (_anibool&ANI_LOOPING)!=0; }
    inline bool IsPaused() const { return (_anibool&ANI_PAUSED)!=0; }

		cAnimation& operator=(const cAnimation& right);

	protected:
		void AddAttribute(Attribute_Def_base* def);
		void RemoveAttribute(Attribute_Def_base* def);

    bss_util::cMap<unsigned char,AniAttribute*,bss_util::CompT<unsigned char>,unsigned char> _attributes;
		double _timepassed; //in milliseconds
		double _anicalc; //Calculated effective length
		double _anilength; //if zero, we automatically stop when all animations are exhausted
		double _aniwarp; //Time warp factor
    cAbstractAnim* _parent;
    cBitField<unsigned char> _anibool;
		unsigned short _id;
    double _looppoint;
	};
  
  struct DEF_ANIMATION_ARRAY
  {
    virtual ~DEF_ANIMATION_ARRAY() {}
		inline virtual DEF_ANIMATION_ARRAY* BSS_FASTCALL Clone() const=0;
		inline virtual void BSS_FASTCALL Load(cAnimation* ani) const=0;
  };
  template<unsigned char TypeID>
  struct DEF_ANIMATION_ARRAY_T : DEF_ANIMATION_ARRAY, bss_util::cDynArray<bss_util::cArraySimple<KeyFrame<TypeID>>>
  {
    virtual ~DEF_ANIMATION_ARRAY_T() {}
    inline virtual DEF_ANIMATION_ARRAY_T* BSS_FASTCALL Clone() const { return new DEF_ANIMATION_ARRAY_T(*this); }
    inline virtual void BSS_FASTCALL Load(cAnimation* ani) const 
    { 
      for(unsigned int i = 0; i < _length; ++i) 
        ani->AddKeyFrame<TypeID>(_array[i]); 
    }
  };

	struct DEF_ANIMATION : cDef<cAnimation>
	{
		inline DEF_ANIMATION() : anilength(0.0), aniwarp(1.0) {}
    inline DEF_ANIMATION(DEF_ANIMATION&& mov) : anilength(mov.anilength), aniwarp(mov.aniwarp), _frames(mov._frames) { mov._frames.Clear(); }
    inline DEF_ANIMATION(const DEF_ANIMATION& copy) : anilength(copy.anilength), aniwarp(copy.aniwarp), _frames(copy._frames)
    {
      for(size_t i = 0; i < _frames.Length(); ++i) 
        _frames[i]=_frames[i]->Clone();
    }
		inline ~DEF_ANIMATION() { for(size_t i = 0; i < _frames.Length(); ++i) delete _frames[i]; }
		inline virtual cAnimation* BSS_FASTCALL Spawn() const { return 0; } // You can't spawn an animation because it can't attach to anything
		inline virtual DEF_ANIMATION* BSS_FASTCALL Clone() const { return new DEF_ANIMATION(*this); }
    template<unsigned char TypeID>
    inline unsigned int AddFrame(const KeyFrame<TypeID>& frame)
    { 
      unsigned char ind = _frames.Get(TypeID);
      if(ind>=_frames.Length()) 
      {
        _frames.Insert(TypeID,new DEF_ANIMATION_ARRAY_T<TypeID>());
        ind = _frames.Get(TypeID);
        assert(ind<_frames.Length());
      }
      return static_cast<DEF_ANIMATION_ARRAY_T<TypeID>*>(_frames[ind])->Add(frame);
    }
    template<unsigned char TypeID>
    inline bool RemoveFrame(unsigned int index)
    {
      unsigned char ind = _frames.Get(TypeID);
      if(ind>=_frames.Length()) return false;
      static_cast<DEF_ANIMATION_ARRAY_T<TypeID>*>(_frames[ind])->Remove(index);
      return true;
    }
    inline const bss_util::cMap<unsigned char,DEF_ANIMATION_ARRAY*,bss_util::CompT<unsigned char>,unsigned char>& GetFrames() const
    { 
      return _frames;
    }
    inline DEF_ANIMATION& operator=(const DEF_ANIMATION& copy) {
      for(size_t i = 0; i < _frames.Length(); ++i) 
        delete _frames[i]; 
      _frames=copy._frames;
      for(size_t i = 0; i < _frames.Length(); ++i) 
        _frames[i]=_frames[i]->Clone();
    }
    inline DEF_ANIMATION& operator=(DEF_ANIMATION&& mov) {
      for(size_t i = 0; i < _frames.Length(); ++i) 
        delete _frames[i]; 
      _frames=mov._frames;
      mov._frames.Clear();
    }

		double anilength; //if zero, we automatically stop when all animations are exhausted
		double aniwarp; //Time warp factor

	private:
    bss_util::cMap<unsigned char,DEF_ANIMATION_ARRAY*,bss_util::CompT<unsigned char>,unsigned char> _frames;
	};
}

#endif