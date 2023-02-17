// Copyright ©2018 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __ALIAS_TABLE_H__BUN__
#define __ALIAS_TABLE_H__BUN__

#include "algo.h"

namespace bun {
  // Implementation of the Alias method, based off Keith Schwarz's code, found here: http://www.keithschwarz.com/darts-dice-coins/
  template<typename UINT = size_t, typename F = double>
  class AliasTable
  {
  public:
    AliasTable(const AliasTable& copy) : _alias(new UINT[copy._count]), _prob(new F[copy._count]), _count(copy._count),
      _dist(copy._dist), _fdist(copy._fdist)
    {
      memcpy(_alias, copy._alias, sizeof(UINT)*copy._count);
      memcpy(_prob, copy._prob, sizeof(UINT)*copy._count);
    }
    AliasTable(AliasTable&& mov) : _prob(std::move(mov._prob)), _alias(std::move(mov._alias)),
      _dist(std::move(mov._dist)), _fdist(std::move(mov._fdist)), _count(mov._count)
    {}
    AliasTable() : _alias(0), _prob(0), _count(0), _fdist(0, (F)1) {}
    // Constructs a new table from a list of probabilities of type F (defaults to double)
    AliasTable(const F* problist, UINT count) : _alias(0), _prob(0), _count(0), _fdist(0, (F)1)
    {
      if(!problist || !count)
        return;
      _count = count; //this is done here so a failure results in _count=0
      _prob.reset(new F[_count]);
      _alias.reset(new UINT[_count]);
      F average = ((F)1.0) / _count;

      std::unique_ptr<F[]> _probcopy(new F[_count]); //Temporary copy of probabilities (seperate from our stored ones)
      memcpy(_probcopy.get(), problist, sizeof(F)*count);

#ifdef BUN_DEBUG
      bun_FillN<UINT>(_alias.get(), _count, -1);
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

#ifdef BUN_DEBUG
      for(UINT i = 0; i < _count; ++i)
        assert(_alias[i] < _count || (_prob[i] == 1.0));
      for(UINT i = 0; i < _count; ++i)
        assert(_prob[i] <= 1.0);
#endif

      _dist = std::uniform_int_distribution<UINT>(0, _count - 1);
    }
    template<UINT I>
    explicit AliasTable(const F(&problist)[I]) : AliasTable(problist, I) {}
    template<UINT I>
    explicit AliasTable(const std::array<F, I>& problist) : AliasTable(problist.data(), I) {}
    ~AliasTable() {}
    // Gets a new random integer using ENGINE (defaults to xorshift) over a uniform integer distribution
    template<typename ENGINE>
    BUN_FORCEINLINE UINT Get(ENGINE& e) const
    {
      UINT c = _dist(e);
      UINT r = (_fdist(e) <= _prob[c]) ? c : _alias[c]; // must be <= to ensure it's ALWAYS true if _prob[c]==1.0
      assert(r < _count);
      return r;
    }
    BUN_FORCEINLINE UINT Get() const { return Get(bun_getdefaultengine()); }
    template<typename ENGINE>
    BUN_FORCEINLINE UINT operator()(ENGINE& e) const { return Get<ENGINE>(e); }
    BUN_FORCEINLINE UINT operator()(void) const { return Get(); }
    inline F* GetProbabilities() const { return _prob; }
    inline UINT* GetAliases() const { return _alias; }
    inline UINT GetCount() const { return _count; }

    inline AliasTable& operator=(const AliasTable& copy)
    {
      _count = copy._count;
      _dist = copy._dist;
      _fdist = copy._fdist;
      _alias.reset(new UINT[copy._count]);
      _prob.reset(new F[copy._count]);
      memcpy(_alias.get(), copy._alias.get(), sizeof(UINT)*copy._count);
      memcpy(_prob.get(), copy._prob.get(), sizeof(UINT)*copy._count);
      return *this;
    }
    inline AliasTable& operator=(AliasTable&& mov)
    {
      _count = mov._count;
      _dist = std::move(mov._dist);
      _fdist = std::move(mov._fdist);
      _alias = std::move(mov._alias);
      _prob = std::move(mov._prob);
      return *this;
    }

  protected:
    std::unique_ptr<UINT[]> _alias;
    std::unique_ptr<F[]> _prob;
    UINT _count;
    mutable std::uniform_int_distribution<UINT> _dist;
    mutable std::uniform_real_distribution<F> _fdist;
  };
}

#endif
