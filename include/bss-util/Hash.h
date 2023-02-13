// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __HASH_H__BSS__
#define __HASH_H__BSS__

#include "khash.h"
#include "bss_util.h"
#include "Array.h"
#include "Str.h"
#include "Serializer.h"
#include <wchar.h>
#include <utility>
#include <iterator>

namespace bss {
  // Default equality function
  template<typename T>
  BSS_FORCEINLINE bool KH_DEFAULT_EQUAL(const T& a, const T& b) { return a == b; }
  template<class T>
  BSS_FORCEINLINE khint_t KH_INT_HASH(T key)
  {
    if constexpr(sizeof(T) == sizeof(khint64_t))
    {
      khint64_t i = *reinterpret_cast<const khint64_t*>(&key);
      return kh_int64_hash_func(i);
    }
    else if constexpr(sizeof(T) == sizeof(khint_t))
    {
      khint_t i = *reinterpret_cast<const khint_t*>(&key);
      return kh_int_hash_func2(i);
    }
    else
      return static_cast<const khint_t>(key);
  }

  // String hash function
  template<class T, bool IgnoreCase>
  inline khint_t KH_STR_HASH(const T* s) = delete;
  template<>
  inline khint_t KH_STR_HASH<char, false>(const char * s)
  {
    khint_t h = *s;
    if(h)
      for(++s; *s; ++s)
        h = (h << 5) - h + *s;
    return h;
  }
  template<>
  inline khint_t KH_STR_HASH<char, true>(const char *s)
  {
    khint_t h = ((*s) > 64 && (*s) < 91) ? (*s) + 32 : *s;
    if(h)
      for(++s; *s; ++s)
        h = (h << 5) - h + (((*s) > 64 && (*s) < 91) ? (*s) + 32 : *s);
    return h;
  }
  template<>
  inline khint_t KH_STR_HASH<wchar_t, false>(const wchar_t * s)
  {
    khint_t h = *s;
    if(h)
      for(++s; *s; ++s)
        h = (h << 5) - h + *s;
    return h;
  }
  template<>
  inline khint_t KH_STR_HASH<wchar_t, true>(const wchar_t *s)
  {
    khint_t h = towlower(*s);
    if(h)
      for(++s; *s; ++s)
        h = (h << 5) - h + towlower(*s);
    return h;
  }

  // String equality function
  template<class T, bool IgnoreCase>
  inline bool KH_STR_EQUAL(const T* a, const T* b) = delete;
  template<> inline bool KH_STR_EQUAL<char, false>(const char* a, const char* b) { return strcmp(a, b) == 0; }
  template<> inline bool KH_STR_EQUAL<wchar_t, false>(const wchar_t* a, const wchar_t* b) { return wcscmp(a, b) == 0; }
  template<> inline bool KH_STR_EQUAL<char, true>(const char* a, const char* b) { return STRICMP(a, b) == 0; }
  template<> inline bool KH_STR_EQUAL<wchar_t, true>(const wchar_t* a, const wchar_t* b) { return WCSICMP(a, b) == 0; }

  template<typename T, bool INS = false> // forward declaration for the multihash
  BSS_FORCEINLINE khint_t KH_AUTO_HASH(const T& k);

  // Hash for tuples, pairs, and arrays that combines the hashes of each element
  template<typename T, int I>
  inline khint_t KH_MULTI_HASH(const T& k)
  {
    if constexpr(I > 0)
      return KH_INT_HASH<uint64_t>(uint64_t(KH_MULTI_HASH<T, I - 1>(k)) | (uint64_t(KH_AUTO_HASH<std::tuple_element_t<I, T>>(std::get<I>(k))) << 32));
    else if constexpr(I == 0)
      return KH_AUTO_HASH<std::tuple_element_t<0, T>>(std::get<0>(k));
  }

  // Equality function for tuples, pairs, and arrays
  template<typename T, int I>
  inline khint_t KH_MULTI_EQUAL(const T& a, const T& b)
  {
    if constexpr(I > 0)
      return (std::get<I>(a) == std::get<I>(b)) && KH_MULTI_EQUAL<T, I - 1>(a, b);
    else if constexpr(I == 0)
      return std::get<0>(a) == std::get<0>(b);
  }

  // Standard auto hashing function
  template<typename T, bool INS>
  BSS_FORCEINLINE khint_t KH_AUTO_HASH(const T& k)
  {
    if constexpr(std::is_same<T, char*>::value || std::is_same<T, wchar_t*>::value || std::is_same<T, const char*>::value || std::is_same<T, const wchar_t*>::value)
      return KH_STR_HASH<std::remove_const_t<std::remove_pointer_t<T>>, INS>(k);
    else if constexpr(std::is_base_of<std::string, T>::value)
      return KH_STR_HASH<char, INS>(k.c_str());
    else if constexpr(std::is_base_of<std::basic_string<wchar_t>, T>::value)
      return KH_STR_HASH<wchar_t, INS>(k.c_str());
    else if constexpr(is_specialization_of<T, std::tuple>::value || is_specialization_of<T, std::pair>::value || is_specialization_of_array<T>::value)
      return KH_MULTI_HASH<T, std::tuple_size<T>::value - 1>(k);
    else
      return KH_INT_HASH<T>(k);
  }

  // Standard auto equality function
  template<typename T, bool INS = false>
  BSS_FORCEINLINE bool KH_AUTO_EQUAL(const T& a, const T& b)
  {
    if constexpr(std::is_same<T, char*>::value || std::is_same<T, wchar_t*>::value || std::is_same<T, const char*>::value || std::is_same<T, const wchar_t*>::value)
      return KH_STR_EQUAL<std::remove_const_t<std::remove_pointer_t<T>>, INS>(a, b);
    else if constexpr(std::is_base_of<std::string, T>::value)
      return KH_STR_EQUAL<char, INS>(a.c_str(), b.c_str());
    else if constexpr(std::is_base_of<std::basic_string<wchar_t>, T>::value)
      return KH_STR_EQUAL<wchar_t, INS>(a.c_str(), b.c_str());
    else if constexpr(is_specialization_of<T, std::tuple>::value || is_specialization_of<T, std::pair>::value || is_specialization_of_array<T>::value)
      return KH_MULTI_EQUAL<T, std::tuple_size<T>::value - 1>(a, b);
    else
      return KH_DEFAULT_EQUAL<T>(a, b);
  }

  namespace internal {
    template<class T> struct _HashGET { using GET = T*; static BSS_FORCEINLINE GET F(T& s) { return &s; } };
    template<class T> struct _HashGET<std::unique_ptr<T>> { using GET = T*; static BSS_FORCEINLINE GET F(std::unique_ptr<T>& s) { return s.get(); } };
    template<> struct _HashGET<Str> { using GET = const char*; static BSS_FORCEINLINE GET F(Str& s) { return s.c_str(); } };
    template<> struct _HashGET<std::string> { using GET = const char*; static BSS_FORCEINLINE GET F(std::string& s) { return s.c_str(); } };
#ifdef BSS_PLATFORM_WIN32
    template<> struct _HashGET<StrW> { using GET = const wchar_t*; static BSS_FORCEINLINE GET F(StrW& s) { return s.c_str(); } };
    template<> struct _HashGET<std::wstring> { using GET = const wchar_t*; static BSS_FORCEINLINE GET F(std::wstring& s) { return s.c_str(); } };
#endif
  }

  // Template hash class based on the khash C implementation
  template<class Key, 
    class Data = void, 
    ARRAY_TYPE ArrayType = ARRAY_SIMPLE, 
    khint_t(*HashFunc)(const Key&) = &KH_AUTO_HASH<Key, false>,
    bool(*HashEqual)(const Key&, const Key&) = &KH_AUTO_EQUAL<Key, false>,
    typename Alloc = StandardAllocator<char>>
  class BSS_COMPILER_DLLEXPORT Hash : protected Alloc
  {
  public:
    static constexpr bool IsMap = !std::is_void<Data>::value;
    using KEY = Key;
    using DATA = Data;
    using FakeData = typename std::conditional<IsMap, Data, char>::type;
    using GET = std::conditional_t<std::is_integral_v<FakeData> || std::is_enum_v<FakeData> || std::is_pointer_v<FakeData> || std::is_member_pointer_v<FakeData>, FakeData, typename internal::_HashGET<FakeData>::GET>;

    Hash(const Hash& copy) : Alloc(copy), n_buckets(0), flags(0), keys(0), vals(0), size(0), n_occupied(0), upper_bound(0)
    {
      if(copy.n_buckets > 0)
        _docopy(copy);
    }
    Hash(Hash&& mov) : Alloc(std::move(mov))
    {
      memcpy(this, &mov, sizeof(Hash));
      bssFill(mov, 0);
    }
    template<bool U = std::is_void_v<typename Alloc::policy_type>, std::enable_if_t<!U, int> = 0>
    Hash(khint_t nbuckets, typename Alloc::policy_type* policy) : Alloc(policy), n_buckets(0), flags(0), keys(0), vals(0), size(0), n_occupied(0), upper_bound(0)
    {
      if(nbuckets > 0)
        _resize(nbuckets);
    }
    explicit Hash(khint_t nbuckets = 0) : n_buckets(0), flags(0), keys(0), vals(0), size(0), n_occupied(0), upper_bound(0)
    {
      if(nbuckets > 0)
        _resize(nbuckets);
    }
    ~Hash()
    {
      Clear(); // calls all destructors.
      _freeall();
    }

    template<bool U = IsMap>
    inline typename std::enable_if<U, khiter_t>::type Insert(const Key& key, const FakeData& value) { return _insert<const Key&, const Data&>(key, value); }
    template<bool U = IsMap>
    inline typename std::enable_if<U, khiter_t>::type Insert(const Key& key, FakeData&& value) { return _insert<const Key&, Data&&>(key, std::move(value)); }
    template<bool U = IsMap>
    inline typename std::enable_if<U, khiter_t>::type Insert(Key&& key, const FakeData& value) { return _insert<Key&&, const Data&>(std::move(key), value); }
    template<bool U = IsMap>
    inline typename std::enable_if<U, khiter_t>::type Insert(Key&& key, FakeData&& value) { return _insert<Key&&, Data&&>(std::move(key), std::move(value)); }
    template<bool U = IsMap>
    inline typename std::enable_if<!U, khiter_t>::type Insert(const Key& key) { int r; return _put<const Key&>(key, &r); }
    template<bool U = IsMap>
    inline typename std::enable_if<!U, khiter_t>::type Insert(Key&& key) { int r; return _put<Key&&>(std::move(key), &r); }

    void Clear()
    {
      if(flags)
      {
        for(khint_t i = 0; i < n_buckets; ++i)
        {
          if(!__ac_iseither(flags, i))
          {
            keys[i].~Key();

            if constexpr(IsMap)
              vals[i].~Data();
          }
        }
        memset(flags, 2, n_buckets);
        size = n_occupied = 0;
      }
    }
    inline khiter_t Iterator(const Key& key) const { return _get(key); }
    inline const Key& GetKey(khiter_t i) const { return keys[i]; }
    template<bool U = IsMap>
    inline typename std::enable_if<U, GET>::type GetValue(khiter_t i) const
    {
      if(!ExistsIter(i))
      {
        if constexpr(std::is_integral<GET>::value)
          return (GET)~0;
        else if constexpr(std::is_enum<GET>::value) // If we try to combine this into the if statement above, VC++ gets confused
          return (GET)~0;
        else if constexpr(std::is_pointer<GET>::value | std::is_member_pointer<GET>::value)
          return nullptr;
        else
          return GET{ 0 };
      }
      if constexpr(std::is_integral_v<FakeData> || std::is_enum_v<FakeData> || std::is_pointer_v<FakeData> || std::is_member_pointer_v<FakeData>)
        return vals[i];
      else
        return internal::_HashGET<FakeData>::F(vals[i]);
    }
    template<bool U = IsMap>
    inline typename std::enable_if<U, GET>::type Get(const Key& key) const { return GetValue(Iterator(key)); }
    template<bool U = IsMap>
    inline typename std::enable_if<U, const FakeData&>::type Value(khiter_t i) const { return vals[i]; }
    template<bool U = IsMap>
    inline typename std::enable_if<U, FakeData&>::type Value(khiter_t i) { return vals[i]; }
    template<bool U = IsMap>
    inline typename std::enable_if<U, FakeData>::type* PointerValue(khiter_t i)
    {
      if(!ExistsIter(i))
        return nullptr;
      return &vals[i];
    }
    template<bool U = IsMap>
    inline typename std::enable_if<U, bool>::type SetValue(khiter_t iterator, const FakeData& newvalue) { return _setvalue<const Data&>(iterator, newvalue); }
    template<bool U = IsMap>
    inline typename std::enable_if<U, bool>::type SetValue(khiter_t iterator, FakeData&& newvalue) { return _setvalue<Data&&>(iterator, std::move(newvalue)); }
    template<bool U = IsMap>
    inline typename std::enable_if<U, bool>::type Set(const Key& key, const FakeData& newvalue) { return _setvalue<const Data&>(Iterator(key), newvalue); }
    template<bool U = IsMap>
    inline typename std::enable_if<U, bool>::type Set(const Key& key, FakeData&& newvalue) { return _setvalue<Data&&>(Iterator(key), std::move(newvalue)); }
    inline void SetCapacity(khint_t capacity) { if(n_buckets < capacity) _resize(capacity); }
    inline bool Remove(const Key& key)
    {
      khiter_t iterator = Iterator(key);
      if(n_buckets == iterator) // This isn't ExistsIter because _get will return n_buckets if key doesn't exist
        return false;

      _delete(iterator);
      return true;
    }
    inline bool RemoveIter(khiter_t iterator)
    {
      if(!ExistsIter(iterator))
        return false;

      _delete(iterator);
      return true;
    }
    BSS_FORCEINLINE khint_t Length() const { return size; }
    BSS_FORCEINLINE khint_t Capacity() const { return n_buckets; }
    BSS_FORCEINLINE khiter_t Front() const { return 0; }
    BSS_FORCEINLINE khiter_t Back() const { return n_buckets; }
    inline bool ExistsIter(khiter_t iterator) const { return (std::make_unsigned_t<khiter_t>(iterator) < std::make_unsigned_t<khiter_t>(n_buckets)) && _exists(iterator); }
    inline bool Exists(const Key& key) const { return ExistsIter(Iterator(key)); }
    template<bool U = IsMap>
    inline typename std::enable_if<U, GET>::type operator[](const Key& key) const { return Get(key); }
    inline bool operator()(const Key& key) const { return Exists(key); }
    template<bool U = IsMap>
    inline typename std::enable_if<U, bool>::type operator()(const Key& key, FakeData& v) const
    {
      khiter_t i = Iterator(key);

      if(!ExistsIter(i))
        return false;

      v = vals[i];
      return true;
    }

    Hash& operator=(const Hash& copy)
    {
      Clear();
      if(!copy.n_buckets)
      {
        _freeall();
        bssFill(*this, 0);
        return *this;
      }
      _docopy(copy);
      return *this;
    }

    Hash& operator=(Hash&& mov)
    {
      Clear();
      _freeall();
      Alloc::operator=(std::move(mov));
      memcpy(this, &mov, sizeof(Hash));
      bssFill(mov);
      return *this;
    }

    friend struct HashIterator;
    template<typename U>
    struct BSS_TEMPLATE_DLLEXPORT HashIterator
    {
      using iterator_category = std::bidirectional_iterator_tag;
      using value_type = std::conditional_t<IsMap, std::tuple<Key, U>, Key>;
      using difference_type = ptrdiff_t;
      using reference = std::conditional_t<IsMap, std::tuple<const Key&, U&>, const Key&>;
      using pointer = std::conditional_t<IsMap, std::tuple<const Key*, U*>, const Key*>;

      using PTR = std::conditional_t<std::is_const_v<U>, const Hash*, Hash*>;
      inline HashIterator(khiter_t c, PTR s) : cur(c), src(s) { _next(); }
      inline std::conditional_t<IsMap, std::tuple<const Key&, U&>, const Key&> operator*() const
      {
        if constexpr(IsMap)
          return { src->GetKey(cur), src->Value(cur) };
        else
          return src->GetKey(cur);
      }
      inline HashIterator& operator++() { ++cur; _next(); return *this; } //prefix
      inline HashIterator operator++(int) { HashIterator r(*this); ++*this; return r; } //postfix
      inline HashIterator& operator--()   //prefix
      {
        std::make_unsigned_t<khiter_t> c = cur; // This is complicated because khiter_t could be signed or unsigned
        while((--c) < src->n_buckets && !src->_exists(c));
        if(c > src->n_buckets) c = src->n_buckets; 
        cur = c;
        return *this;
      } 
      inline HashIterator operator--(int) { HashIterator r(*this); --*this; return r; } //postfix
      inline bool operator==(const HashIterator& _Right) const { return (cur == _Right.cur); }
      inline bool operator!=(const HashIterator& _Right) const { return (cur != _Right.cur); }

      khiter_t cur;
      PTR src;

    protected:
      inline void _next() { while(cur < src->n_buckets && !src->_exists(cur)) ++cur; }
    };

    BSS_FORCEINLINE HashIterator<const FakeData> begin() const { return HashIterator<const FakeData>(Front(), this); }
    BSS_FORCEINLINE HashIterator<const FakeData> end() const { return HashIterator<const FakeData>(Back(), this); }
    BSS_FORCEINLINE HashIterator<FakeData> begin() { return HashIterator<FakeData>(Front(), this); }
    BSS_FORCEINLINE HashIterator<FakeData> end() { return HashIterator<FakeData>(Back(), this); }

    using SerializerArray = std::conditional_t<IsMap, void, Key>;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id)
    {
      if constexpr(!IsMap)
        s.template EvaluateArray<Hash, Key, &_serializeAdd<Engine>, size_t, nullptr>(*this, size, id);
      else
        s.template EvaluateKeyValue<Hash>(*this, [this](Serializer<Engine>& e, const char* name)
      {
        Key k = internal::serializer::FromString<Key>(name);
        Data* v = PointerValue(Iterator(k));
        if(!v)
          v = PointerValue(Insert(k, Data()));
        if(v)
          Serializer<Engine>::template ActionBind<Data>::Parse(e, *v, name);
      });
    }

  protected:
    template<typename Engine>
    static inline void _serializeAdd(Serializer<Engine>& e, Hash& obj, int& n)
    {
      Key key;
      Serializer<Engine>::template ActionBind<Engine, Key>::Parse(e, key, 0);
      obj.Insert(std::move(key));
    }
    BSS_FORCEINLINE bool _exists(khiter_t iterator) const { return !__ac_iseither(flags, iterator); }
    inline void _freeall()
    {
      if(flags) Alloc::deallocate((char*)flags, n_buckets);
      if(keys) Alloc::deallocate((char*)keys, n_buckets * sizeof(Key));
      if constexpr(IsMap)
      {
        if(vals) Alloc::deallocate((char*)vals, n_buckets * sizeof(Data));
      }
      else
        assert(vals == 0);
    }
    inline void _docopy(const Hash& copy)
    {
      if constexpr(ArrayType != ARRAY_MOVE)
      {
        _resize(copy.n_buckets);
        assert(n_buckets == copy.n_buckets);
        memcpy(flags, copy.flags, n_buckets);
        for(khint_t i = 0; i < n_buckets; ++i)
        {
          if(!__ac_iseither(copy.flags, i))
          {
            new(keys + i) Key((const Key&)copy.keys[i]);

            if constexpr(IsMap)
              new(vals + i) Data((const Data&)copy.vals[i]);
          }
        }

        assert(n_buckets == copy.n_buckets);
        size = copy.size;
        n_occupied = copy.n_occupied;
        upper_bound = copy.upper_bound;
      }
      else
        assert(false);
    }
    template<typename U, typename V>
    inline khiter_t _insert(U && key, V && value)
    {
      int r;
      khiter_t i = _put<const Key&>(std::forward<U>(key), &r);
      if(!r) // If r is 0, this key was already present, so we need to assign, not initialize
        vals[i] = std::forward<V>(value);
      else
        new(vals + i) Data(std::forward<V>(value));
      return i;
    }
    template<typename U>
    inline bool _setvalue(khiter_t i, U && newvalue) { if(!ExistsIter(i)) return false; vals[i] = std::forward<U>(newvalue); return true; }
    char _resize(khint_t new_n_buckets)
    {
      khint8_t *new_flags = 0;
      khint_t j = 1;
      {
        kroundup32(new_n_buckets);
        if(new_n_buckets < 4) new_n_buckets = 32;
        if(size >= (khint_t)(new_n_buckets * __ac_HASH_UPPER + 0.5)) j = 0;	/* requested size is too small */
        else
        { /* hash table size to be changed (shrink or expand); rehash */
          new_flags = (khint8_t*)Alloc::allocate(new_n_buckets);
          if(!new_flags) return -1;
          memset(new_flags, 2, new_n_buckets);
          if(n_buckets < new_n_buckets)
          {	/* expand */
            Key *new_keys = _realloc<Key>(keys, new_n_buckets);
            if(!new_keys) { Alloc::deallocate((char*)new_flags, new_n_buckets); return -1; }
            keys = new_keys;
            if constexpr(IsMap)
            {
              Data *new_vals = _realloc<Data>(vals, new_n_buckets);
              if(!new_vals) { Alloc::deallocate((char*)new_flags, new_n_buckets); return -1; }
              vals = new_vals;
            }
          } /* otherwise shrink */
        }
      }
      if(j)
      { /* rehashing is needed */
        for(j = 0; j != n_buckets; ++j)
        {
          if(__ac_iseither(flags, j) == 0)
          {
            Key key(std::move(keys[j]));
            [[maybe_unused]] FakeData val;
            khint_t new_mask;
            new_mask = new_n_buckets - 1;
            if constexpr(IsMap)val = std::move(vals[j]);
            __ac_set_isdel_true(flags, j);
            while(1)
            { /* kick-out process; sort of like in Cuckoo hashing */
              khint_t k, i, step = 0;
              k = HashFunc(key);
              i = k & new_mask;
              while(!__ac_isempty(new_flags, i)) i = (i + (++step)) & new_mask;
              __ac_set_isempty_false(new_flags, i);
              if(i < n_buckets && __ac_iseither(flags, i) == 0)
              { /* kick out the existing element */
                std::swap(keys[i], key);
                if constexpr(IsMap)std::swap(vals[i], val);
                __ac_set_isdel_true(flags, i); /* mark it as deleted in the old hash table */
              }
              else // this code only runs if this bucket doesn't exist, so initialize
              {
                new(keys + i) Key(std::move(key));
                if constexpr(IsMap) new(vals + i) Data(std::move(val));
                break;
              }
            }
          }
        }
        if(n_buckets > new_n_buckets)
        { /* shrink the hash table */
          keys = _realloc<Key>(keys, new_n_buckets);
          if constexpr(IsMap)
            vals = _realloc<Data>(vals, new_n_buckets);
        }
        if(flags)
          Alloc::deallocate((char*)flags, n_buckets); /* free the working space */
        flags = new_flags;
        n_buckets = new_n_buckets;
        n_occupied = size;
        upper_bound = (khint_t)(n_buckets * __ac_HASH_UPPER + 0.5);
      }
      return 0;
    }

    template<typename U>
    khint_t _put(U && key, int* ret)
    {
      khint_t x;
      if(n_occupied >= upper_bound)
      { /* update the hash table */
        if(n_buckets > (size << 1))
        {
          \
            if(_resize(n_buckets - 1) < 0)
            { /* clear "deleted" elements */
              *ret = -1; return n_buckets;
            }
        }
        else if(_resize(n_buckets + 1) < 0)
        { /* expand the hash table */
          *ret = -1; return n_buckets;
        }
      } /* TODO: to implement automatically shrinking; resize() already support shrinking */
      {
        khint_t k, i, site, last, mask = n_buckets - 1, step = 0;
        x = site = n_buckets; k = HashFunc(key); i = k & mask;
        if(__ac_isempty(flags, i)) x = i; /* for speed up */
        else
        {
          last = i;
          while(!__ac_isempty(flags, i) && (__ac_isdel(flags, i) || !HashEqual(keys[i], key)))
          {
            if(__ac_isdel(flags, i)) site = i;
            i = (i + (++step)) & mask;
            if(i == last) { x = site; break; }
          }
          if(x == n_buckets)
          {
            if(__ac_isempty(flags, i) && site != n_buckets) x = site;
            else x = i;
          }
        }
      }
      if(__ac_isempty(flags, x))
      { /* not present at all */
        new(keys + x) Key(std::move(key));
        __ac_set_isboth_false(flags, x);
        ++size; ++n_occupied;
        *ret = 1;
      }
      else if(__ac_isdel(flags, x))
      { /* deleted */
        new(keys + x) Key(std::move(key));
        __ac_set_isboth_false(flags, x);
        ++size;
        *ret = 2;
      }
      else *ret = 0; /* Don't touch keys[x] if present and not deleted */
      return x;
    }
    khint_t _get(const Key& key) const
    {
      if(n_buckets)
      {
        khint_t k, i, last, mask, step = 0;
        mask = n_buckets - 1;
        k = HashFunc(key); i = k & mask;
        last = i;
        while(!__ac_isempty(flags, i) && (__ac_isdel(flags, i) || !HashEqual(keys[i], key)))
        {
          i = (i + (++step)) & mask;
          if(i == last) return n_buckets;
        }
        return __ac_iseither(flags, i) ? n_buckets : i;
      }
      else return 0;
    }
    inline void _delete(khint_t x)
    {
      if(x != n_buckets && !__ac_iseither(flags, x))
      {
        keys[x].~Key();
        if constexpr(IsMap)
          vals[x].~Data();
        __ac_set_isdel_true(flags, x);
        --size;
      }
    }
    template<typename T>
    inline T* _realloc(T* src, khint_t new_n_buckets) noexcept
    {
      if constexpr(ArrayType == ARRAY_SIMPLE || ArrayType == ARRAY_CONSTRUCT)
        return (T*)Alloc::allocate(new_n_buckets * sizeof(T), (char*)src, n_buckets * sizeof(T));
      else if(ArrayType == ARRAY_SAFE || ArrayType == ARRAY_MOVE)
      {
        T* n = (T*)Alloc::allocate(new_n_buckets * sizeof(T), 0);
        if(n != nullptr)
        {
          for(khint_t i = 0; i < n_buckets; ++i)
          {
            if(!__ac_iseither(flags, i))
              new(n + i) T(std::move(src[i]));
          }
        }

        if(src)
          Alloc::deallocate((char*)src, n_buckets * sizeof(T));

        return n;
      }
    }

    khint_t n_buckets, size, n_occupied, upper_bound;
    khint8_t* flags;
    Key* keys;
    Data* vals;
  };

  // Case-insensitive hash definition
  template<typename K, typename T, ARRAY_TYPE ArrayType = ARRAY_SIMPLE, typename Alloc = StandardAllocator<char>>
  class BSS_COMPILER_DLLEXPORT HashIns : public Hash<K, T, ArrayType, &KH_AUTO_HASH<K, true>, &KH_AUTO_EQUAL<K, true>, Alloc>
  {
  public:
    using BASE = Hash<K, T, ArrayType, &KH_AUTO_HASH<K, true>, &KH_AUTO_EQUAL<K, true>, Alloc>;

    inline HashIns(const HashIns& copy) = default;
    inline HashIns(HashIns&& mov) = default;
    template<bool U = std::is_void_v<typename Alloc::policy_type>, std::enable_if_t<!U, int> = 0>
    inline HashIns(khint_t capacity, typename Alloc::policy_type* policy) : BASE(capacity, policy) {}
    inline explicit HashIns(khint_t capacity = 0) : BASE(capacity) {}

    inline HashIns& operator =(const HashIns& right) = default;
    inline HashIns& operator =(HashIns&& mov) = default;
    inline bool operator()(const K& key) const { return BASE::operator()(key); }
    template<bool U = !std::is_void<T>::value>
    inline typename std::enable_if<U, bool>::type operator()(const K& key, typename BASE::FakeData& v) const { return BASE::operator()(key, v); }
    template<bool U = !std::is_void<T>::value>
    inline typename std::enable_if<U, typename BASE::GET>::type operator[](const K& key) const { return BASE::operator[](key); }
  };
}

#endif
