// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __MAP_H__BUN__
#define __MAP_H__BUN__

#include "ArraySort.h"
#include <tuple>

namespace bun {

  // A map class implemented as an associative sorted array
  template<class Key, class Data, Comparison<Key, Key> Comp = std::compare_three_way, typename CType = size_t,
           typename Alloc = StandardAllocator<std::tuple<Key, Data>>>
  class BUN_COMPILER_DLLEXPORT Map :
    protected ArraySort<std::tuple<Key, Data>, tuple_three_way<std::tuple<Key, Data>, std::tuple<Key, Data>, 0, Comp>,
                        CType, Alloc>
  {
  protected:
    using pair_t   = std::tuple<Key, Data>;
    using BASE     = ArraySort<pair_t, tuple_three_way<pair_t, pair_t, 0, Comp>, CType, Alloc>;
    using constref = const Data&;
    using CKEYREF  = const Key&;
    using BASE::_array;
    using CT = typename BASE::CT;
    using Ty = typename BASE::Ty;

    [[nodiscard]] constexpr BUN_FORCEINLINE const Comp& _getcomp() const noexcept { return BASE::_getbase()._getf(); }

    template<typename K, typename D, Comparison<K, K> C> struct _map_three_way : protected C
    {
      _map_three_way(const C& c) : C(c) {}
      _map_three_way(C&& c) : C(std::move(c)) {}
      _map_three_way(const _map_three_way&) = default;
      _map_three_way(_map_three_way&&)      = default;

      [[nodiscard]] constexpr BUN_FORCEINLINE auto operator()(const std::tuple<K, D>& l, const K& r) const
      {
        return _getf()(std::get<0>(l), r);
      }

      [[nodiscard]] constexpr BUN_FORCEINLINE const C& _getf() const noexcept { return *this; }

      using is_transparent = int;
    };

  public:
    inline Map(CT init, const Alloc& alloc, const Comp& c) : BASE(init, alloc, c) {}
    inline explicit Map(CT init, const Alloc& alloc)
      requires std::is_default_constructible_v<Comp>
      : Map(init, alloc, Comp())
    {}
    inline explicit Map(CT init, const Comp& c)
      requires std::is_default_constructible_v<Alloc>
      : Map(init, Alloc(), c)
    {}
    inline explicit Map(CT init)
      requires std::is_default_constructible_v<Alloc> && std::is_default_constructible_v<Comp>
      : Map(init, Alloc(), Comp())
    {}
    inline explicit Map()
      requires std::is_default_constructible_v<Alloc> && std::is_default_constructible_v<Comp>
      : Map(1, Alloc(), Comp())
    {}
    inline Map(const Map& copy) = default;
    inline Map(Map&& mov)       = default;
    //~Map() {}
    BUN_FORCEINLINE void Clear() { BASE::Clear(); }
    BUN_FORCEINLINE void Discard(CT num) { BASE::Discard(num); }
    BUN_FORCEINLINE CT Insert(CKEYREF key, constref data) { return BASE::Insert(pair_t(key, data)); }
    BUN_FORCEINLINE CT Insert(CKEYREF key, Data&& data) { return BASE::Insert(pair_t(key, std::move(data))); }
    inline CT Get(CKEYREF key) const
    {
      CT retval = GetNear(key, true);
      return (retval != (CT)(~0) && (_getcomp()(std::get<0>(_array[retval]), key) == 0)) ? retval : (CT)(~0);
    }
    inline constref GetData(CKEYREF key) const { return DataIndex(GetNear(key, true)); } // this has no checking
    inline constref DataIndex(CT index) const { return std::get<1>(_array[index]); }
    inline CKEYREF KeyIndex(CT index) const { return std::get<0>(_array[index]); }
    inline CT Remove(CKEYREF key)
    {
      CT retval = Get(key);
      BASE::Remove(retval);
      return retval;
    }
    BUN_FORCEINLINE CT RemoveIndex(CT index) { return BASE::Remove(index); }
    BUN_FORCEINLINE CT Replace(CT index, CKEYREF key, constref data) { return BASE::ReplaceData(index, pair_t(key, data)); }
    BUN_FORCEINLINE CT Replace(CT index, CKEYREF key, Data&& data)
    {
      return BASE::ReplaceData(index, pair_t(key, std::move(data)));
    }
    inline CT ReplaceKey(CT index, CKEYREF key)
    {
      if(index < 0 || index >= size())
        return (CT)(~0);
      return BASE::ReplaceData(index, pair_t(key, std::get<1>(_array[index])));
    }
    inline CT Set(CKEYREF key, constref data)
    {
      CT retval = GetNear(key, true);
      if(retval != (CT)(~0))
      {
        auto& [k, v] = _array[retval];
        if(_getcomp()(k, key) == 0)
          v = data;
      }
      return retval;
    }
    CT GetNear(CKEYREF key, bool before) const
    {
      auto mapper = _map_three_way<Key, Data, Comp>(_getcomp());
      CT retval   = before ? BinarySearchNear<true, const decltype(BASE::_array)&, Key>(_array, key, std::move(mapper)) - 1:
                             BinarySearchNear<false, const decltype(BASE::_array)&, Key>(_array, key, std::move(mapper));

      return (retval < size()) ?
               retval :
               (CT)(~0); // This is only needed for before=false in case it returns a value outside the range.
    }
    inline const pair_t* begin() const { return _array.begin(); }
    inline const pair_t* end() const { return _array.end(); }
    inline pair_t* begin() { return _array.begin(); }
    inline pair_t* end() { return _array.end(); }
    inline const pair_t& Front() const { return _array.Front(); }
    inline pair_t& Front() { return _array.Front(); }
    inline const pair_t& Back() const { return _array.Back(); }
    inline pair_t& Back() { return _array.Back(); }
    inline CT size() const noexcept { return _array.size(); }
    inline pair_t* data() noexcept { return _array.data(); }
    inline const pair_t* data() const noexcept { return _array.data(); }

    inline Map& operator=(const Map& right) = default;
    inline Map& operator=(Map&& right)      = default;
    inline const pair_t& operator[](CT index) const { return _array[index]; }
    inline pair_t& operator[](CT index) { return _array[index]; }
    inline constref operator()(CT index) const { return std::get<1>(_array[index]); }
    inline Data& operator()(CT index) { return std::get<1>(_array[index]); }

    using BASE::SerializerArray;
    template<typename Engine> void Serialize(Serializer<Engine>& s, const char* id)
    {
      BASE::template Serialize<Engine>(s, id);
    }
  };
}

#endif