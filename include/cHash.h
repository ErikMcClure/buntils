// Copyright ©2014 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_HASH_H__BSS__
#define __C_HASH_H__BSS__

#include "khash.h"
#include "bss_defines.h"
#include <wchar.h>
#include <iterator>

template<typename T>
inline khint_t khint_hashfunc(T in) { return (khint_t)in; }
template<typename T>
inline bool khint_equalfunc(T a, T b) { return a==b; }

//#define KHASH_INIT(name, khkey_t, khval_t, kh_is_map, __hash_func, __hash_equal)
template<typename khkey_t, typename khval_t, bool kh_is_map, khint_t (*__hash_func)(khkey_t), bool (*__hash_equal)(khkey_t, khkey_t)>
class BSS_COMPILER_DLLEXPORT kh_template_t
{					
public:
		khint_t n_buckets, size, n_occupied, upper_bound;	
		khint32_t *flags;
		khkey_t *keys;
		khval_t *vals;
};

template<typename khkey_t, typename khval_t, bool kh_is_map, khint_t (*__hash_func)(khkey_t), bool (*__hash_equal)(khkey_t, khkey_t)>
static inline kh_template_t<khkey_t,khval_t,kh_is_map,__hash_func,__hash_equal>* kh_init_template() {
	return (kh_template_t<khkey_t,khval_t,kh_is_map,__hash_func,__hash_equal>*)calloc(1, sizeof(kh_template_t<khkey_t,khval_t,kh_is_map,__hash_func,__hash_equal>));
}																			\
template<typename khkey_t, typename khval_t, bool kh_is_map, khint_t (*__hash_func)(khkey_t), bool (*__hash_equal)(khkey_t, khkey_t)>
static inline void kh_destroy_template(kh_template_t<khkey_t,khval_t,kh_is_map,__hash_func,__hash_equal> *h)
{
	if (h) {
		free(h->keys); free(h->flags);
		free(h->vals);
		free(h);
	}
}

template<typename khkey_t, typename khval_t, bool kh_is_map, khint_t (*__hash_func)(khkey_t), bool (*__hash_equal)(khkey_t, khkey_t)>
static inline void kh_clear_template(kh_template_t<khkey_t,khval_t,kh_is_map,__hash_func,__hash_equal> *h)				
{																	
	if (h && h->flags) {											
		memset(h->flags, 0xaa, ((h->n_buckets>>4) + 1) * sizeof(khint32_t)); 
		h->size = h->n_occupied = 0;								
	}																
}								

template<typename khkey_t, typename khval_t, bool kh_is_map, khint_t (*__hash_func)(khkey_t), bool (*__hash_equal)(khkey_t, khkey_t)>
static khint_t kh_get_template(const kh_template_t<khkey_t,khval_t,kh_is_map,__hash_func,__hash_equal> *h, khkey_t key)
{																	
	if (h->n_buckets) {												
		khint_t inc, k, i, last;									
		k = __hash_func(key); i = k % h->n_buckets;					
		inc = 1 + k % (h->n_buckets - 1); last = i;					
		while (!__ac_isempty(h->flags, i) && (__ac_isdel(h->flags, i) || !__hash_equal(h->keys[i], key))) { 
			if (i + inc >= h->n_buckets) i = i + inc - h->n_buckets; 
			else i += inc;											
			if (i == last) return h->n_buckets;						
		}															
		return __ac_iseither(h->flags, i)? h->n_buckets : i;		
	} else return 0;												
}

template<typename khkey_t, typename khval_t, bool kh_is_map, khint_t (*__hash_func)(khkey_t), bool (*__hash_equal)(khkey_t, khkey_t)>
static void kh_resize_template(kh_template_t<khkey_t,khval_t,kh_is_map,__hash_func,__hash_equal> *h, khint_t new_n_buckets)
{																	
  khint32_t *new_flags = 0;										
  khint_t j = 1;													
  {																
	  khint_t t = __ac_HASH_PRIME_SIZE - 1;						
	  while (__ac_prime_list[t] > new_n_buckets) --t;				
	  new_n_buckets = __ac_prime_list[t+1];						
	  if (h->size >= (khint_t)(new_n_buckets * __ac_HASH_UPPER + 0.5)) j = 0;	
	  else {														
		  new_flags = (khint32_t*)malloc(((new_n_buckets>>4) + 1) * sizeof(khint32_t));	
		  memset(new_flags, 0xaa, ((new_n_buckets>>4) + 1) * sizeof(khint32_t)); 
		  if (h->n_buckets < new_n_buckets) {						
			  h->keys = (khkey_t*)realloc(h->keys, new_n_buckets * sizeof(khkey_t));
			  if (kh_is_map)										
				  h->vals = (khval_t*)realloc(h->vals, new_n_buckets * sizeof(khval_t)); 
		  }														
	  }															
  }																
  if (j) {														
	  for (j = 0; j != h->n_buckets; ++j) {						
		  if (__ac_iseither(h->flags, j) == 0) {					
			  khkey_t key = h->keys[j];							
			  khval_t val;										
			  if (kh_is_map) val = h->vals[j];					
			  __ac_set_isdel_true(h->flags, j);					
			  while (1) {											
				  khint_t inc, k, i;								
				  k = __hash_func(key);							
				  i = k % new_n_buckets;							
				  inc = 1 + k % (new_n_buckets - 1);				
				  while (!__ac_isempty(new_flags, i)) {			
					  if (i + inc >= new_n_buckets) i = i + inc - new_n_buckets; 
					  else i += inc;								
				  }												
				  __ac_set_isempty_false(new_flags, i);			
				  if (i < h->n_buckets && __ac_iseither(h->flags, i) == 0) { 
					  { khkey_t tmp = h->keys[i]; h->keys[i] = key; key = tmp; } 
					  if (kh_is_map) { khval_t tmp = h->vals[i]; h->vals[i] = val; val = tmp; } 
					  __ac_set_isdel_true(h->flags, i);			
				  } else {										
					  h->keys[i] = key;							
					  if (kh_is_map) h->vals[i] = val;			
					  break;										
				  }												
			  }													
		  }														
	  }															
	  if (h->n_buckets > new_n_buckets) {							
		  h->keys = (khkey_t*)realloc(h->keys, new_n_buckets * sizeof(khkey_t)); 
		  if (kh_is_map)											
			  h->vals = (khval_t*)realloc(h->vals, new_n_buckets * sizeof(khval_t)); 
	  }															
	  free(h->flags);												
	  h->flags = new_flags;										
	  h->n_buckets = new_n_buckets;								
	  h->n_occupied = h->size;									
	  h->upper_bound = (khint_t)(h->n_buckets * __ac_HASH_UPPER + 0.5); 
  }																
}

template<typename khkey_t, typename khval_t, bool kh_is_map, khint_t (*__hash_func)(khkey_t), bool (*__hash_equal)(khkey_t, khkey_t)>
static khint_t kh_put_template(kh_template_t<khkey_t,khval_t,kh_is_map,__hash_func,__hash_equal> *h, khkey_t key, int *ret) 
{											
	khint_t x;														
	if (h->n_occupied >= h->upper_bound) {							
		if (h->n_buckets > (h->size<<1)) kh_resize_template(h, h->n_buckets - 1); 
		else kh_resize_template(h, h->n_buckets + 1);					
	}																
  {																
		khint_t inc, k, i, site, last;								
		x = site = h->n_buckets; k = __hash_func(key); i = k % h->n_buckets; 
		if (__ac_isempty(h->flags, i)) x = i;						
    else {													
			inc = 1 + k % (h->n_buckets - 1); last = i;				
			while (!__ac_isempty(h->flags, i) && (__ac_isdel(h->flags, i) || !__hash_equal(h->keys[i], key))) { 
				if (__ac_isdel(h->flags, i)) site = i;				
				if (i + inc >= h->n_buckets) i = i + inc - h->n_buckets; 
				else i += inc;										
				if (i == last) { x = site; break; }					
			}														
			if (x == h->n_buckets) {								
				if (__ac_isempty(h->flags, i) && site != h->n_buckets) x = site; 
				else x = i;											
			}														
		}															
	}																
	if (__ac_isempty(h->flags, x)) {								
		h->keys[x] = key;											
		__ac_set_isboth_false(h->flags, x);							
		++h->size; ++h->n_occupied;									
		*ret = 1;													
	} else if (__ac_isdel(h->flags, x)) {							
		h->keys[x] = key;											
		__ac_set_isboth_false(h->flags, x);							
		++h->size;													
		*ret = 2;													
	} else *ret = 0;												
  return x;														
}

template<typename khkey_t, typename khval_t, bool kh_is_map, khint_t (*__hash_func)(khkey_t), bool (*__hash_equal)(khkey_t, khkey_t)>
static inline void kh_del_template(kh_template_t<khkey_t,khval_t,kh_is_map,__hash_func,__hash_equal> *h, khint_t x)		
{																	
	if (x != h->n_buckets && !__ac_iseither(h->flags, x)) {			
		__ac_set_isdel_true(h->flags, x);							
		--h->size;													
	}																
}
//KHASH_MAP_INIT_STR(cls, void*);
//KHASH_MAP_INIT_STRINS(clsins, void*);
//KHASH_MAP_INIT_INT(intcls, void*);
//KHASH_MAP_INIT_INT64(int64clr, void*);

namespace bss_util {
  template<bool CHK, typename KHGET, typename KHTYPE> // GCC doesn't like it when you try to embed a templatized function inside a class
  struct __getval_KHASH { static inline KHGET BSS_FASTCALL _getval(KHTYPE* _h, khiter_t i) { return kh_val(_h,i); } };
  template<typename KHGET, typename KHTYPE>
  struct __getval_KHASH<true,KHGET,KHTYPE> { static inline KHGET BSS_FASTCALL _getval(KHTYPE* _h, khiter_t i) { return &kh_val(_h,i); } };

  // Base template for cKhash. If you want a set instead of a map, set khval_t to 'char' and kh_is_map to 'false'.
  template<typename khkey_t, typename khval_t, bool kh_is_map, khint_t(*__hash_func)(khkey_t), bool(*__hash_equal)(khkey_t, khkey_t), typename khget_t=khval_t*, khget_t INVALID=0>
  class BSS_COMPILER_DLLEXPORT cKhash
  {
  protected:
    typedef khkey_t KHKEY;
    typedef khval_t KHVAL;
    typedef kh_template_t<KHKEY,KHVAL,kh_is_map,__hash_func,__hash_equal> KHTYPE;
    typedef khget_t KHGET;

  public:
    inline cKhash(const cKhash& copy) : _h(kh_init_template<KHKEY,KHVAL,kh_is_map,__hash_func,__hash_equal>()) { operator =(copy); }
    inline cKhash(cKhash&& mov) : _h(mov._h) { mov._h=0; }
    inline cKhash(unsigned int size=0) : _h(kh_init_template<KHKEY,KHVAL,kh_is_map,__hash_func,__hash_equal>()) { kh_resize_template(_h, size); };
    inline ~cKhash() { kh_destroy_template(_h); }
    inline khiter_t Iterator(KHKEY key) const { return kh_get_template(_h, key); }
    inline void Insert(KHKEY key, const KHVAL& value) { _insert<const KHVAL&>(key,value); }
    inline void Insert(KHKEY key, KHVAL&& value) { _insert<KHVAL&&>(key,std::move(value)); }
    //inline KHKEY GetIterKey(khiter_t iterator) { return kh_key(_h,iterator); }
    inline KHKEY GetKey(khiter_t iterator) { return kh_key(_h,iterator); }
    inline bool SetKey(khiter_t iterator, KHKEY key) { if(kh_end(_h) == iterator) return false; kh_key(_h,iterator)=key; return true; }
    inline KHGET Get(KHKEY key) const { return GetValue(Iterator(key)); }
    inline KHGET GetValue(khiter_t iter) const { if(!ExistsIter(iter)) return INVALID; return __getval_KHASH<std::is_same<KHGET,khval_t*>::value,KHGET,KHTYPE>::_getval(_h,iter); }
    inline const KHVAL& UnsafeValue(khiter_t iterator) const { return kh_val(_h,iterator); }
    inline bool SetValue(khiter_t iterator, const KHVAL& newvalue) { return _setvalue<const KHVAL&>(iterator,newvalue); } 
    inline bool SetValue(khiter_t iterator, KHVAL&& newvalue) { return _setvalue<KHVAL&&>(iterator, std::move(newvalue)); }
    inline bool Set(khiter_t iterator, const KHVAL& newvalue) { return _setvalue<const KHVAL&>(Iterator(iterator), newvalue); }
    inline bool Set(khiter_t iterator, KHVAL&& newvalue) { return _setvalue<KHVAL&&>(Iterator(iterator), std::move(newvalue)); }
    inline void SetSize(unsigned int size) { if(kh_end(_h) < size) kh_resize_template(_h,size); }
		inline bool Remove(KHKEY key) const
    {
      khiter_t iterator = Iterator(key);
      if(kh_end(_h) == iterator) return false; // This isn't ExistsIter because kh_get_template will return kh_end(_h) if key doesn't exist
		  kh_del_template(_h, iterator);
			return true;
    }
    inline bool RemoveIter(khiter_t iterator) const
    {
      if(!ExistsIter(iterator)) return false;
		  kh_del_template(_h, iterator);
			return true;
    }
		inline void Clear() { kh_clear_template(_h); }
		inline unsigned int Length() const { return kh_size(_h); }
		inline unsigned int Capacity() const { return kh_end(_h); }
		inline khiter_t Start() const { return kh_begin(_h); }
		inline khiter_t End() const { return kh_end(_h); }
    inline bool ExistsIter(khiter_t iterator) const { return iterator<kh_end(_h) && kh_exist(_h, iterator)!=0; }
    inline bool Exists(KHKEY key) const { return ExistsIter(Iterator(key)); }
		inline cKhash& operator =(const cKhash& right)
		{
      if(&right == this) return *this;
			kh_clear_template(_h);
			kh_resize_template(_h, kh_end(right._h));
			int r;
			khiter_t cur=(khiter_t)-1;
      if(kh_is_map) {
			  while(++cur != right.End())
          if(kh_exist(right._h,cur)!=0)
				    kh_val(_h, kh_put_template(_h, kh_key(right._h, cur),&r))=kh_val(right._h, cur);
      } else {
			  while(++cur != right.End())
          if(kh_exist(right._h,cur)!=0)
				    kh_put_template(_h, kh_key(right._h, cur),&r);
      }
			return *this; 
		}
		inline cKhash& operator =(cKhash&& mov)
    {
      if(&mov == this) return *this;
			kh_destroy_template(_h);
      _h=mov._h;
      mov._h=0;
      return *this;
    }
    inline KHGET operator[](KHKEY key) const { return Get(key); }
    //inline KHVAL& operator[](khiter_t iterator) { return kh_val(_h, iterator); }
    //inline const KHVAL& operator[](khiter_t iterator) const { return kh_val(_h, iterator); }

    class BSS_COMPILER_DLLEXPORT cKhash_Iter : public std::iterator<std::bidirectional_iterator_tag,khiter_t>
	  {
    public:
      inline explicit cKhash_Iter(const cKhash& src) : _src(&src), cur(0) { _chknext(); }
      inline cKhash_Iter(const cKhash& src, khiter_t start) : _src(&src), cur(start) { _chknext(); }
      inline khiter_t operator*() const { return cur; }
      inline cKhash_Iter& operator++() { ++cur; _chknext(); return *this; } //prefix
      inline cKhash_Iter operator++(int) { cKhash_Iter r(*this); ++*this; return r; } //postfix
      inline cKhash_Iter& operator--() {  while((--cur)<_src->End() && !_src->ExistsIter(cur)); return *this; } //prefix
      inline cKhash_Iter operator--(int) { cKhash_Iter r(*this); --*this; return r; } //postfix
      inline bool operator==(const cKhash_Iter& _Right) const { return (cur == _Right.cur); }
	    inline bool operator!=(const cKhash_Iter& _Right) const { return (cur != _Right.cur); }
      inline bool operator!() const { return !IsValid(); }
      inline bool IsValid() { return cur<_src->End(); }

      khiter_t cur; //khiter_t is unsigned (this is why operator--() works)

    protected:
      inline void _chknext() { while(cur<_src->End() && !_src->ExistsIter(cur)) ++cur; }

      const cKhash* _src;
	  };

    inline cKhash_Iter begin() const { return cKhash_Iter(*this,Start()); }
    inline cKhash_Iter end() const { return cKhash_Iter(*this,End()); }

	protected:
    template<typename U>
    inline void _insert(KHKEY key, U && value)
		{
			//if(kh_size(_h) >= kh_end(_h)) _resize(); // Not needed, kh_put_template resizes as necessary
			int r;
			khiter_t retval = kh_put_template(_h,key,&r);
      if(kh_is_map) // only set value if this is actually a map
        kh_val(_h,retval)=std::forward<U>(value);
		}
    template<typename U>
    inline bool _setvalue(khiter_t i, U && newvalue) { if(i>=kh_end(_h)) return false; kh_val(_h, i)=std::forward<U>(newvalue); return true; }

		KHTYPE* _h;
  };

  inline static khint_t KH_INT64_HASHFUNC(__int64 key) { return (khint32_t)((key)>>33^(key)^(key)<<11); }
  template<class T> 
  BSS_FORCEINLINE khint_t KH_INT_HASHFUNC(T key) { return (khint32_t)key; }
  inline static khint_t KH_STR_HASHFUNC(const char * s) { khint_t h = *s; if(h) for(++s; *s; ++s) h = (h << 5) - h + *s; return h; }
  inline static khint_t KH_STRINS_HASHFUNC(const char *s) { khint_t h = ((*s)>64&&(*s)<91)?(*s)+32:*s;	if(h) for(++s; *s; ++s) h = (h << 5) - h + (((*s)>64&&(*s)<91)?(*s)+32:*s); return h; }
  inline static khint_t KH_STRW_HASHFUNC(const wchar_t * s) { khint_t h = *s; if(h) for(++s; *s; ++s) h = (h << 5) - h + *s; return h; }
  inline static khint_t KH_STRWINS_HASHFUNC(const wchar_t *s) { khint_t h = towlower(*s); if(h) for(++s; *s; ++s) h = (h << 5) - h + towlower(*s); return h; }
  template<class T>
  BSS_FORCEINLINE khint_t KH_POINTER_HASHFUNC(T p) {
#ifdef BSS_64BIT
  return KH_INT64_HASHFUNC((__int64)p);
#else
  return (khint32_t)p;
#endif
  }
  template<typename T, int I> struct KH_AUTO_HELPER { };
  template<typename T> struct KH_AUTO_HELPER<T, 1> { BSS_FORCEINLINE static khint_t hash(T k) { return KH_POINTER_HASHFUNC<T>(k); } };
  template<typename T> struct KH_AUTO_HELPER<T, 2> { BSS_FORCEINLINE static khint_t hash(T k) { return KH_INT_HASHFUNC<T>((__int32)k); } };
  template<typename T> struct KH_AUTO_HELPER<T, 4> { BSS_FORCEINLINE static khint_t hash(T k) { return KH_INT64_HASHFUNC((__int64)k); } };

#ifdef BSS_COMPILER_GCC // GCC decides that "force inline" doesn't really mean "force inline" so we have to put static here to protect it from it's own idiocy.
#define FIX_PISS_POOR_PROGRAMMING_IN_GCC static
#else
#define FIX_PISS_POOR_PROGRAMMING_IN_GCC 
#endif

  template<typename T>
  FIX_PISS_POOR_PROGRAMMING_IN_GCC BSS_FORCEINLINE khint_t KH_AUTO_HASHFUNC(T k) { return KH_AUTO_HELPER<T, std::is_pointer<T>::value + std::is_integral<T>::value*2*(1+(sizeof(T)==8))>::hash(k); }
  template<> BSS_FORCEINLINE khint_t KH_AUTO_HASHFUNC<const char*>(const char* k) { return KH_STR_HASHFUNC(k); }
  template<> BSS_FORCEINLINE khint_t KH_AUTO_HASHFUNC<char*>(char* k) { return KH_STR_HASHFUNC(k); }
  template<> BSS_FORCEINLINE khint_t KH_AUTO_HASHFUNC<const wchar_t*>(const wchar_t* k) { return KH_STRW_HASHFUNC(k); }
  template<> BSS_FORCEINLINE khint_t KH_AUTO_HASHFUNC<wchar_t*>(wchar_t* k) { return KH_STRW_HASHFUNC(k); }
  template<> BSS_FORCEINLINE khint_t KH_AUTO_HASHFUNC<double>(double k) { return KH_INT64_HASHFUNC(*(__int64*)&k); }
  template<> BSS_FORCEINLINE khint_t KH_AUTO_HASHFUNC<float>(float k) { return *(khint32_t*)&k; }
  template<typename T> FIX_PISS_POOR_PROGRAMMING_IN_GCC BSS_FORCEINLINE khint_t KH_AUTOINS_HASHFUNC(T k) { return KH_STRINS_HASHFUNC(k); }
  template<> BSS_FORCEINLINE khint_t KH_AUTOINS_HASHFUNC<const wchar_t*>(const wchar_t* k) { return KH_STRWINS_HASHFUNC(k); }
  template<> BSS_FORCEINLINE khint_t KH_AUTOINS_HASHFUNC<wchar_t*>(wchar_t* k) { return KH_STRWINS_HASHFUNC(k); }

  template<typename T>
  FIX_PISS_POOR_PROGRAMMING_IN_GCC BSS_FORCEINLINE bool KH_AUTO_EQUALFUNC(T a, T b) { return a==b; }
  template<> BSS_FORCEINLINE bool KH_AUTO_EQUALFUNC<const char*>(const char* a, const char* b) { return strcmp(a, b)==0; }
  template<> BSS_FORCEINLINE bool KH_AUTO_EQUALFUNC<const wchar_t*>(const wchar_t* a, const wchar_t* b) { return wcscmp(a, b)==0; }
  template<typename T> FIX_PISS_POOR_PROGRAMMING_IN_GCC BSS_FORCEINLINE bool KH_AUTOINS_EQUALFUNC(T a, T b) { return STRICMP(a, b)==0; }
  template<> BSS_FORCEINLINE bool KH_AUTOINS_EQUALFUNC<const wchar_t*>(const wchar_t* a, const wchar_t* b) { return WCSICMP(a, b)==0; }
  template<> BSS_FORCEINLINE bool KH_AUTOINS_EQUALFUNC<wchar_t*>(wchar_t* a, wchar_t* b) { return WCSICMP(a, b)==0; }

  template<typename T,bool I> struct __cKh_KHGET { typedef typename std::remove_pointer<T>::type* KHGET; static const unsigned int INV=0; };
  template<typename T> struct __cKh_KHGET<T,true> { typedef T KHGET; static const KHGET INV=(KHGET)-1; };
  template<typename T> struct KHGET_cKh : __cKh_KHGET<T,std::is_integral<T>::value> { typedef typename __cKh_KHGET<T,std::is_integral<T>::value>::KHGET KHGET; };
  template<typename T, bool ins> struct CHASH_HELPER {
    BSS_FORCEINLINE static khint_t hash(T k) { return KH_AUTO_HASHFUNC<T>(k); }
    BSS_FORCEINLINE static bool equal(T a, T b) { return KH_AUTO_EQUALFUNC<T>(a,b); }
  };
  template<typename T> struct CHASH_HELPER<T,true> {
    BSS_FORCEINLINE static khint_t hash(T k) { return KH_AUTOINS_HASHFUNC<T>(k); }
    BSS_FORCEINLINE static bool equal(T a, T b) { return KH_AUTOINS_EQUALFUNC<T>(a, b); }
  };

  // Generic hash definition
  template<typename K, typename T, bool ismap=true, bool ins=false>
  class BSS_COMPILER_DLLEXPORT cHash : public cKhash<K, T, ismap, &CHASH_HELPER<K, ins>::hash, &CHASH_HELPER<K, ins>::equal, typename KHGET_cKh<T>::KHGET, (typename KHGET_cKh<T>::KHGET)KHGET_cKh<T>::INV>
  {
    typedef cKhash<K, T, ismap, &CHASH_HELPER<K, ins>::hash, &CHASH_HELPER<K, ins>::equal, typename KHGET_cKh<T>::KHGET, (typename KHGET_cKh<T>::KHGET)KHGET_cKh<T>::INV> BASE;

  public:
    inline cHash(const cHash& copy) : BASE(copy) {}
    inline cHash(cHash&& mov) : BASE(std::move(mov)) {}
    inline cHash(unsigned int size=0) : BASE(size) {}

    inline cHash& operator =(const cHash& right) { BASE::operator=(right); return *this; }
    inline cHash& operator =(cHash&& mov) { BASE::operator=(std::move(mov)); return *this; }
    inline typename BASE::KHGET operator[](typename BASE::KHKEY key) const { return BASE::operator[](key); }
  };

  template<typename _Ty>
  struct cDict
  {
    typedef cHash<const char*, _Ty, true, false> T;
    typedef cHash<const wchar_t*, _Ty, true, false> Tw;
    typedef cHash<const char*, _Ty, true, true> INS;
    typedef cHash<const wchar_t*, _Ty, true, true> INSw;
  };
}

#endif
