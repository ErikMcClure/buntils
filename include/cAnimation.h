// Copyright ©2015 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_ANIMATION_H__BSS__
#define __C_ANIMATION_H__BSS__

#include "AniAttribute.h"
#include "cMap.h"
#include "cDynArray.h"

namespace bss_util {
  template<typename Alloc, int TypeID>
  struct cAnimation_GenAttribute {
    BSS_FORCEINLINE static AniAttribute* f(unsigned char typeID)
    {
      if(TypeID==typeID)
        return new(Alloc::template rebind<AniAttributeT<TypeID, Alloc>>::other::allocate(1)) AniAttributeT<TypeID, Alloc>();
      else
        return cAnimation_GenAttribute<Alloc, std::is_void<typename AniAttributeT<TypeID+1, Alloc>::IDTYPE>::value?-1:TypeID+1>::f(typeID);
    }
  };

  template<typename Alloc>
  struct cAnimation_GenAttribute<Alloc, -1> {
    BSS_FORCEINLINE static AniAttribute* f(unsigned char typeID) { return 0; }
  };

  // A class representing a single animation comprised of various attributes (position, rotation, etc.)
  template<typename Alloc = StaticAllocPolicy<char>>
  class BSS_COMPILER_DLLEXPORT cAnimation
  {
  protected:
    enum ANIBOOLS : unsigned char { ANI_PLAYING=1, ANI_PAUSED=2, ANI_ATTACHED=4 };
    typedef std::pair<unsigned char, AniAttribute*> MAPPAIR; // VS2010 can't handle this being inside the rebind for some reason
    typedef typename Alloc::template rebind<MAPPAIR>::other MAPALLOC;

  public:
    inline cAnimation(const cAnimation& copy) { operator=(copy); }
    inline cAnimation(cAnimation&& mov) { operator=(std::move(mov)); }
    inline cAnimation() : _aniwarp(1.0), _anilength(0), _timepassed(0), _anicalc(0), _looppoint(-1.0) { }
    inline ~cAnimation() { Clear(); }
    // Allows you to skip forward by setting _timepassed to the specified value.
    inline virtual void Start(double timepassed=0.0)
    {
      if(!(_anibool&ANI_ATTACHED)) return; // Can't start if you aren't attached to something
      _timepassed=timepassed;
      _anibool+=ANI_PLAYING;

      unsigned char svar=_attributes.Length();
      double length;
      _anicalc = 0.0;

      for(unsigned char i = 0; i < svar; ++i) {
        _attributes[i]->Start();
        if((length = _attributes[i]->Length()) > _anicalc)
          _anicalc = length;
      }
    }
    // Stop animation
    inline void Stop() { _anibool-=ANI_PLAYING; _timepassed=0.0; }
    // Temporarily pauses or unpauses the animation
    inline void Pause(bool pause) { _anibool[ANI_PAUSED]=pause; }
    // Sets the loop point for the animation. If its negative, looping is disable. This change affects the animation even if its already started.
    inline void SetLoopPoint(double looppoint=-1.0) { _looppoint=looppoint; }
    // Starts the animation and sets the looppoint at the same time
    inline void Loop(double timepassed=0.0, double looppoint=0.0) { SetLoopPoint(looppoint); Start(timepassed); }
    // Interpolates the animation by moving it forward *delta* milliseconds
    bool Interpolate(double delta)
    {
      if((_anibool&(ANI_PLAYING|ANI_ATTACHED))==0) return false; // If it isn't playing or it's not attached to anything, bail out.
      if((_anibool&ANI_PAUSED)!=0) return true;
      _timepassed += delta*_aniwarp;
      double length = _anilength==0.0?_anicalc:_anilength;

      if(IsLooping() && length>0.0 && _timepassed > length)
        Start(fmod(_timepassed-length, length-_looppoint)+_looppoint);

      bool notfinished=_timepassed<_anilength;

      unsigned char svar=_attributes.Length();
      for(unsigned char i = 0; i < svar; ++i)
        if(_attributes[i]->Interpolate(_timepassed))
          notfinished=true;

      if(!notfinished) //in this case we're done, so reset
      {
        if(IsLooping()) Start(fmod(_timepassed-length, length-_looppoint)+_looppoint);
        else Stop();
      }

      return (_anibool&ANI_PLAYING)!=0;
    }
    // Gets the abstract class of any TypeID at runtime, creating the attribute if necessary.
    inline AniAttribute* GetTypeID(unsigned char TypeID)
    {
      unsigned char index=_attributes.Get(TypeID);
      if(index<_attributes.Length()) return _attributes[index];
      AniAttribute* retval=cAnimation_GenAttribute<Alloc, 0>::f(TypeID);
      if(retval!=0) _attributes.Insert(retval->typeID, retval);
      return retval;
    }
    // Gets the actual type of a given attribute, but TypeID must be known at compile time. Also creates the attribute if it doesn't exist.
    template<unsigned char TypeID>
    inline AniAttributeT<TypeID, Alloc>* GetAttribute() { return static_cast<AniAttributeT<TypeID, Alloc>*>(GetTypeID(TypeID)); }
    // Attaches the animation to an arbitrary object by calling f(attribute) on all attributes. An object doesn't need to accept all possible attributes of an animation.
    template<typename F> // F must be something resolving to void(AniAttribute* p) that calls p->Attach() if appropriate.
    inline void Attach(F f)
    {
      Detach(); // Forces all attributes to assume they have no legal attachment (so when you don't call ->Attach() it works properly)
      unsigned char svar=_attributes.Length();
      for(unsigned char i = 0; i < svar; ++i)
        f(_attributes[i]);

      _anibool+=ANI_ATTACHED;
    }
    // Detaches the animation by detaching all of its attributes.
    inline void Detach()
    {
      unsigned char svar=_attributes.Length();
      for(unsigned char i = 0; i < svar; ++i)
        _attributes[i]->Detach();
      _anibool-=ANI_ATTACHED;
    }
    // Returns true if the TypeID currently exists or false otherwise.
    inline bool HasTypeID(unsigned char TypeID) const { return _attributes.Get(TypeID)<_attributes.Length(); }
    // Sets the animation length. If the animation length is set to 0, the total amount of time the attributes take is used as the animation length instead.
    inline void SetAnimationLength(double anilength = 0.0) {
      _anilength=anilength;
      _anicalc = 0.0;
      double length;
      for(unsigned char i = 0; i < _attributes.Length(); ++i) {
        if((length = _attributes[i]->Length()) > _anicalc)
          _anicalc = length;
      }
    }
    // Gets the animation length.
    inline double GetAnimationLength() const { return _anilength==0.0?_anicalc:_anilength; }
    // Sets the time warp of the animation, which is simply a constant multiplied against the delta. 0.5 halves the animation speed, 2 doubles it.
    inline void SetTimeWarp(double aniwarp) { _aniwarp=aniwarp; }
    // Gets the time warp.
    inline double GetTimeWarp() const { return _aniwarp; }
    // Gets the amount of time passed in the animation. This is reset to zero every time the animation loops.
    inline double GetTimePassed() const { return _timepassed; }
    inline bool IsAttached() const { return (_anibool&ANI_ATTACHED)!=0; }
    inline bool IsPlaying() const { return (_anibool&ANI_PLAYING)!=0; }
    inline bool IsLooping() const { return _looppoint>=0.0; }
    inline bool IsPaused() const { return (_anibool&ANI_PAUSED)!=0; }

    cAnimation& operator +=(const cAnimation& add)
    {
      unsigned char svar=_attributes.Length();
      unsigned char rvar=add._attributes.Length();
      unsigned char j=0;
      for(unsigned char i = 0; i < rvar;)
      {
        if(j>=svar) { //We've run off the end of our existing IDs so simply append the rest
          AniAttribute* p = add._attributes[i++];
          GetTypeID(p->typeID)->CopyAnimation(p);
        } else
          switch(SGNCOMPARE(_attributes[j]->typeID, add._attributes[i]->typeID))
        {
          case -1:
            ++j;
            break;
          case 0: // They are the same, so add them together
            _attributes[j]->AddAnimation(add._attributes[i]);
            break;
          case 1: // If this happens, we skipped over one of the IDs in the right, so just append it.
            AniAttribute* p = add._attributes[i++];
            GetTypeID(p->typeID)->CopyAnimation(p);
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
    cAnimation& operator=(const cAnimation& right)
    {
      Clear();
      _aniwarp=right._aniwarp;
      _anibool=right._anibool;
      _timepassed=right._timepassed;
      _anicalc=right._anicalc;
      _anilength=right._anilength;
      _looppoint=right._looppoint;
      _attributes=right._attributes;

      unsigned char svar=_attributes.Length();
      for(unsigned char i = 0; i < svar; ++i)
        _attributes[i]=_attributes[i]->Clone();

      return *this;
    }

    cAnimation& operator=(cAnimation&& right)
    {
      Clear();
      _aniwarp=right._aniwarp;
      _anibool=right._anibool;
      _timepassed=right._timepassed;
      _anicalc=right._anicalc;
      _anilength=right._anilength;
      _looppoint=right._looppoint;
      _attributes=std::move(right._attributes);
      return *this;
    }

    inline void Serialize(std::ostream& s)
    {
      bss_Serialize(_anilength, s);
      bss_Serialize(_aniwarp, s);
      bss_Serialize(_looppoint, s);
      bss_Serialize<unsigned char>(_attributes.Length(), s);

      for(int i = 0; i < _attributes.Length(); ++i) {
        bss_Serialize(_attributes[i]->typeID, s);
        _attributes[i]->Serialize(s);
      }
    }
    inline void Deserialize(std::istream& s)
    {
      bss_Deserialize(_anilength, s);
      bss_Deserialize(_aniwarp, s);
      bss_Deserialize(_looppoint, s);
      unsigned char l;
      bss_Deserialize<unsigned char>(l, s);

      unsigned char id;
      for(int i = 0; i < l; ++i) {
        bss_Deserialize(id, s);
        GetTypeID(id)->Deserialize(s);
      }
    }
    inline void Clear()
    {
      for(unsigned char i = 0; i < _attributes.Length(); ++i)
      {
        _attributes[i]->~AniAttribute();
        Alloc::deallocate((char*)_attributes[i]);
      }
      _attributes.Clear();
    }

  protected:
    bss_util::cMap<unsigned char, AniAttribute*, bss_util::CompT<unsigned char>, unsigned char, CARRAY_SIMPLE, MAPALLOC> _attributes;
    double _timepassed; //in milliseconds
    double _anicalc; //Calculated effective length
    double _anilength; //if zero, we automatically stop when all animations are exhausted
    double _aniwarp; //Time warp factor
    cBitField<unsigned char> _anibool;
    double _looppoint;
  };
}

#endif
