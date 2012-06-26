// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_ALIAS_TABLE_H__BSS__
#define __C_ALIAS_TABLE_H__BSS__

#include "bss_defines.h"
#include "bss_deprecated.h"

namespace bss_util {
  /* Implementation of the Alias method, based off Keith Schwarz's code, found here: http://www.keithschwarz.com/darts-dice-coins/ */
  template<typename UINT=unsigned int, typename F=double>
  class BSS_COMPILER_DLLEXPORT cAliasTable
  {
  public:
    inline cAliasTable(const cAliasTable& copy) : _alias(new UINT[copy._count]), _prob(new F[copy._count]), _count(copy._count)
    {
      memcpy(_alias,copy._alias,sizeof(UINT)*copy._count);
      memcpy(_prob,copy._prob,sizeof(UINT)*copy._count);
    }
    inline cAliasTable(cAliasTable&& mov) : _alias(mov._alias), _prob(mov._prob), _count(mov._count) { mov._alias=0; mov._prob=0; mov._count=0; }
    inline cAliasTable(const F* problist, UINT count) : _alias(0), _prob(0), _count(0) { _gentable(problist, count); }
    template<UINT I>
    inline explicit cAliasTable(const F (&problist)[I]) : _alias(0), _prob(0), _count(0) { _gentable(problist, I);  }
    inline ~cAliasTable()
    { 
      if(_alias!=0) delete [] _alias;
      if(_prob!=0) delete [] _prob;
    }
    inline BSS_FORCEINLINE UINT Get() const
    {
      UINT c = (UINT)(rand()%_count);
      return (((F)rand()/(F)RAND_MAX) < _prob[c])?c:_alias[c];
    }

  protected:
    inline void _gentable(const F* problist, UINT count)
    {
      if(!problist || !count)
        return;
      _count=count; //this is done here so a failure results in _count=0
      _prob=new F[_count];
      _alias=new UINT[_count];
      F average = ((F)1.0)/_count;

      _probcopy = new F[_count]; //Temporary copy of probabilities (seperate from our stored ones)
      memcpy(_probcopy,problist,sizeof(F)*count);

      UINT* small=new UINT[_count]; //Small and large stacks as simple arrays
      UINT n_small=0;
      UINT* large=new UINT[_count];
      UINT n_large=0;

      for (int i = 0; i < count; ++i)
      {
        if(_probcopy[i] >= average)
          large[n_large++]=i;
        else
          small[n_small++]=i;
      }

      int less;
      int more;
      while (n_small!=0 && n_large!=0) //In a perfect world, small always empties before large, but our world isn't, so we check both
      { 
        less=small[--n_small];
        more=large[--n_large];

        _prob[less] = _probcopy[less] * _count; //scale probabilities so 1/n is given weight 1.0
        alias[less] = more;
        _probcopy[more]=(_probcopy[more] + _probcopy[less]) - average; //Set new probability    

        if(_probcopy[more] >= average) //Move new probability to correct list
          large[n_large++]=more;
        else
          small[n_small++]=more;
      }

      //Set everything to 1.0 (both lists are set due to numerical uncertainty)
      while (n_small!=0)
        _prob[--n_small] = 1.0;
      while (n_large!=0)
        _prob[--n_large] = 1.0;
    }

    UINT* _alias;
    F* _prob;
    UINT _count;
  };
}

#endif