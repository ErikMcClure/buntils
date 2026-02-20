// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __TRIE_H__
#define __TRIE_H__

#include "algo.h"
#include "BinaryHeap.h"
#include "compare.h"
#include <stdarg.h>

namespace bun {
  namespace internal {
    // Trie node
    template<typename T = uint8_t> struct BUN_COMPILER_DLLEXPORT TRIE_NODE
    {
      char chr; // letter that this node has
      T child;  // Pointer to this node's child, if it has one
      T clen;   // Number of siblings this node has
      T word;   // This stores the original index of the word that this node corresponds to, but only if chr is nullptr
                // (indicating the end of a word)

      bool operator==(const TRIE_NODE& r) const { return chr == r.chr; }
      std::strong_ordering operator<=>(const TRIE_NODE& r) const { return chr <=> r.chr; }
      bool operator==(const char& r) const { return chr == r; }
      std::strong_ordering operator<=>(const char& r) const { return chr <=> r; }
      inline operator char() const { return chr; }
    };

    static_assert(std::three_way_comparable_with<bun::internal::TRIE_NODE<uint16_t>, char, std::partial_ordering>);
  }

  // A static trie optimized for looking up small collections of words.
  template<typename T = uint8_t, bool IGNORECASE = false>
    requires std::is_unsigned_v<T>
  class BUN_COMPILER_DLLEXPORT Trie : protected Array<internal::TRIE_NODE<T>, T>
  {
    using BASE = Array<internal::TRIE_NODE<T>, T>;
    using BASE::_array;
    using TNODE = internal::TRIE_NODE<T>;
    using PAIR  = std::pair<T, const char*>;

  public:
    inline Trie(Trie&& mov) : BASE(std::move(mov)), _length(mov._length) { mov._length = 0; }
    inline Trie(const Trie& copy) : BASE(copy), _length(copy._length) {}
    inline Trie(std::initializer_list<const char*> init) :
      BASE(static_cast<T>(init.size())), _length(static_cast<T>(init.size()))
    {
      _construct(init.size(), init.begin());
    }
    template<size_t SZ> inline Trie(const char* const (&initstr)[SZ]) : BASE(SZ), _length(SZ) { _construct(SZ, initstr); }
    inline ~Trie() {}
    T Get(const char* word) const
    {
      assert(word != 0);
      std::span<TNODE> cur = _array; // root is always 0
      T r                  = 0;
      char c;
      while((c = *(word++)))
      {
        if constexpr(IGNORECASE)
          c = tolower(c);

        if(cur[0].clen > 1) // This is faster than a switch statement
          r =
            static_cast<T>(BinarySearchExact<std::span<TNODE>, char, std::compare_three_way>(cur.subspan(0, cur[0].clen), c,
                                                                                             std::compare_three_way{}));
        else if(cur[0].clen == 1)
          r = static_cast<T>(-(cur[0].chr != c));
        else
          return std::numeric_limits<T>::max();

        if(r == std::numeric_limits<T>::max())
          return std::numeric_limits<T>::max();
        cur = _array.subspan(cur[r].child);
      }
      return cur[0].word;
    }
    T Get(const char* word, T len) const
    {
      assert(word != 0);
      std::span<TNODE> cur = _array; // root is always 0
      T r                  = 0;
      char c;
      while((len--) > 0)
      {
        c = *(word++);
        if constexpr(IGNORECASE)
          c = tolower(c);

        if(cur[0].clen > 1) // This is faster than a switch statement
          r = BinarySearchExact<std::span<TNODE>, char, std::compare_three_way>(cur.subspan(0, cur[0].clen), c,
                                                                                std::compare_three_way{});
        else if(cur[0].clen == 1)
          r = (T) - (cur[0].chr != c);
        else
          return std::numeric_limits<T>::max();

        if(r == std::numeric_limits<T>::max())
          return std::numeric_limits<T>::max();
        cur = _array.subspan(cur[r].child);
      }
      return cur[0].word;
    }
    inline const TNODE* data() const { return _array.data(); }
    inline T size() { return _length; }
    inline size_t Capacity() { return BASE::Capacity(); }
    inline T operator[](const char* word) const { return Get(word); }
    inline Trie& operator=(const Trie& copy)
    {
      BASE::operator=(copy);
      _length = copy._length;
      return *this;
    }
    inline Trie& operator=(Trie&& mov)
    {
      BASE::operator=(std::move(mov));
      _length     = mov._length;
      mov._length = 0;
      return *this;
    }

  protected:
    void _construct(size_t num, const char* const* initstr)
    {
      _fill(0, num);
      VARARRAY(PAIR, s, num);

      for(size_t i = 0; i < num; ++i)
      {
        s[i].first  = static_cast<T>(i);
        s[i].second = initstr[i];
      }

      using SORTING_FUNC =
        second_three_way<const char*, const char*,
                         typename std::conditional<IGNORECASE, string_three_way_insensitive, string_three_way>::type>;

      BinaryHeap<PAIR, SORTING_FUNC, T>::HeapSort(s, SORTING_FUNC{}); // sort into alphabetical order
      _init(static_cast<T>(num), s.data(), 0, 0);                     // Put into our recursive initializer
    }
    BUN_FORCEINLINE void _fill(size_t s, size_t e) // Zeros out a range of nodes
    {
      for(size_t i = s; i < e; ++i)
      {
        _array[i].word  = std::numeric_limits<T>::max();
        _array[i].child = std::numeric_limits<T>::max();
        _array[i].clen  = 0;
        _array[i].chr   = 0;
      }
    }
    BUN_FORCEINLINE void _checkSize(T r)
    {
      assert(r < (std::numeric_limits<T>::max() - 2));
      if(r >= Capacity())
      {
        size_t s = Capacity();
        BASE::_setCapacity(Capacity() << 1);
        _fill(s, Capacity());
      }
    }
    T _init(T len, PAIR const* str, T cnt, T level)
    {
      T r    = cnt - 1;
      char c = str[0].second[level];
      if(IGNORECASE)
        c = tolower(c);

      if(!c) // The only place we'll recieve the end of the word is in our starting position due to alphabetical order
      {
        _checkSize(r + 1);
        _array[r + 1].word = str[0].first;
        ++str;
        --len;
      }

      char l = 0;
      for(T i = 0; i < len; ++i) // first pass so we can assemble top level nodes here
      {
        c = str[i].second[level];
        if(IGNORECASE)
          c = tolower(c);
        if(l != c)
        {
          _checkSize(++r);
          _array[r].chr = (l = c);
        }
        assert(_array[r].clen < (std::numeric_limits<T>::max() - 2));
        ++_array[r].clen;
      }

      len = (++r) - cnt;
      ++level;
      T last = r;

      for(T i = cnt; i < last; ++i) // Second pass that generates children
      {
        _array[i].child = r;
        r               = _init(_array[i].clen, str, r, level);
        str += _array[i].clen;
      }

      _array[cnt].clen = len;
      return !len ? 1 + r : r;
    }

    T _length;
  };
}

template<typename T, template<class> class TQual, template<class> class UQual>
struct std::basic_common_reference<bun::internal::TRIE_NODE<T>, char, TQual, UQual>
{
  using type = char;
};

template<typename T, template<class> class TQual, template<class> class UQual>
struct std::basic_common_reference<char, bun::internal::TRIE_NODE<T>, TQual, UQual>
{
  using type = char;
};

#endif