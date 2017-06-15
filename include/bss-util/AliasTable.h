// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __ALIAS_TABLE_H__BSS__
#define __ALIAS_TABLE_H__BSS__

#include "algo.h"

namespace bss {
  // Implementation of the Alias method, based off Keith Schwarz's code, found here: http://www.keithschwarz.com/darts-dice-coins/
  template<typename UINT = size_t, typename F = double, typename ENGINE = XorshiftEngine<uint64_t>>
  class AliasTable
  {
  public:
    AliasTable(const AliasTable& copy) : _alias(new UINT[copy._count]), _prob(new F[copy._count]), _count(copy._count),
      _dist(copy._dist), _fdist(copy._fdist), _engine(copy._engine)
    {
      memcpy(_alias, copy._alias, sizeof(UINT)*copy._count);
      memcpy(_prob, copy._prob, sizeof(UINT)*copy._count);
    }
    AliasTable(AliasTable&& mov) : _alias(mov._alias), _prob(mov._prob), _count(mov._count), _dist(std::move(mov._dist)),
      _fdist(std::move(mov._fdist)), _engine(std::move(mov._engine))
    {
      mov._alias = 0;
      mov._prob = 0;
      mov._count = 0;
    }
    // Constructs a new table from a list of probabilities of type F (defaults to double)
    AliasTable(const F* problist, UINT count, ENGINE& e = bss_getdefaultengine()) : _alias(0), _prob(0), _count(0),
      _fdist(0, (F)1), _engine(e) 
    { 
      _getTable(problist, count);
    }
    template<UINT I>
    explicit AliasTable(const F(&problist)[I], ENGINE& e = bss_getdefaultengine()) : _alias(0), _prob(0), _count(0),
      _fdist(0, (F)1), _engine(e)
    { 
      _getTable(problist, I); 
    }
    ~AliasTable()
    {
      if(_alias != 0)
        delete[] _alias;
      if(_prob != 0)
        delete[] _prob;
    }
    // Gets a new random integer using ENGINE (defaults to xorshift) over a uniform integer distribution
    BSS_FORCEINLINE UINT Get()
    {
      UINT c = _dist(_engine);
      UINT r = (_fdist(_engine) <= _prob[c]) ? c : _alias[c]; // must be <= to ensure it's ALWAYS true if _prob[c]==1.0
      assert(r < _count);
      return r;
    }
    BSS_FORCEINLINE UINT operator()(void) { return Get(); }
    inline F* GetProbabilities() const { return _prob; }
    inline UINT* GetAliases() const { return _alias; }
    inline UINT GetCount() const { return _count; }

  protected:
    void _getTable(const F* problist, UINT count)
    {
      if(!problist || !count)
        return;
      _count = count; //this is done here so a failure results in _count=0
      _prob = new F[_count];
      _alias = new UINT[_count];
      F average = ((F)1.0) / _count;

      std::unique_ptr<F[]> _probcopy(new F[_count]); //Temporary copy of probabilities (seperate from our stored ones)
      memcpy(_probcopy.get(), problist, sizeof(F)*count);

#ifdef BSS_DEBUG
      bssFillN(_alias, _count, -1);
#endif
      std::unique_ptr<UINT[]> small(new UINT[_count]); //Small and large stacks as simple arrays
      UINT n_small = 0;
      std::unique_ptr<UINT[]> large(new UINT[_count]);
      UINT n_large = 0;

      for(UINT i = 0; i < count; ++i)
      {
        if(_probcopy[i] >= average)
          large[n_large++] = i;
        else
          small[n_small++] = i;
      }

      UINT l;
      UINT g;
      while(n_small != 0 && n_large != 0) //In a perfect world, small always empties before large, but our world isn't, so we check both
      {
        l = small[--n_small];
        g = large[--n_large];

        _prob[l] = _probcopy[l] * _count; //scale probabilities so 1/n is given weight 1.0
        _alias[l] = g;
        _probcopy.get()[g] = (_probcopy[g] + _probcopy[l]) - average; //Set new probability    

        if(_probcopy[g] >= average) //Move new probability to correct list
          large[n_large++] = g;
        else
          small[n_small++] = g;
      }

      //Set everything to 1.0 (both lists are set due to numerical uncertainty)
      while(n_large != 0)
        _prob[large[--n_large]] = 1.0;
      while(n_small != 0)
        _prob[small[--n_small]] = 1.0;

#ifdef BSS_DEBUG
      for(UINT i = 0; i < _count; ++i)
        assert(_alias[i] < _count || (_prob[i] == 1.0));
      for(UINT i = 0; i < _count; ++i)
        assert(_prob[i] <= 1.0);
#endif

      _dist = std::uniform_int_distribution<UINT>(0, _count - 1);
    }

    UINT* _alias;
    F* _prob;
    UINT _count;
    ENGINE& _engine;
    std::uniform_int_distribution<UINT> _dist;
    std::uniform_real_distribution<F> _fdist;
  };
}

#endif
