// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_KHASH_H__BSS__
#define __C_KHASH_H__BSS__

#include "khash.h"
#include "bss_defines.h"
#include "bss_deprecated.h"
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
  template<typename khkey_t, typename khval_t, bool kh_is_map, khint_t (*__hash_func)(khkey_t), bool (*__hash_equal)(khkey_t, khkey_t), typename KHGET=khval_t*, KHGET INVALID=0>
  class BSS_COMPILER_DLLEXPORT cKhash
  {
  protected:
    typedef khkey_t KHKEY;
    typedef khval_t KHVAL;
    typedef kh_template_t<KHKEY,KHVAL,kh_is_map,__hash_func,__hash_equal> KHTYPE;

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
    inline bool SetValue(khiter_t iterator, KHVAL&& newvalue) { return _setvalue<KHVAL&&>(iterator,std::move(newvalue)); } 
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
    //inline KHVAL& operator[](khiter_t iterator) { return kh_val(_h, iterator); }
    //inline const KHVAL& operator[](khiter_t iterator) const { return kh_val(_h, iterator); }
    inline KHGET operator[](KHKEY key) const { return Get(key); }

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

  inline khint_t KH_INT64_HASHFUNC(__int64 key) { return (khint32_t)((key)>>33^(key)^(key)<<11); }
  inline khint_t KH_UINT64_HASHFUNC(unsigned __int64 key) { return KH_INT64_HASHFUNC((__int64)key); }
  template<class T>
  BSS_FORCEINLINE khint_t KH_INT_HASHFUNC(T key) { return (khint32_t)key; }
  inline khint_t KH_STR_HASHFUNC(const char * s) { khint_t h = *s; if (h) for (++s ; *s; ++s) h = (h << 5) - h + *s; return h; }
  inline khint_t KH_STRINS_HASHFUNC(const char *s) { khint_t h = ((*s)>64&&(*s)<91)?(*s)+32:*s;	if (h) for (++s ; *s; ++s) h = (h << 5) - h + (((*s)>64&&(*s)<91)?(*s)+32:*s); return h; }
  inline khint_t KH_STRW_HASHFUNC(const wchar_t * s) { khint_t h = *s; if (h) for (++s ; *s; ++s) h = (h << 5) - h + *s; return h; }
  inline khint_t KH_STRWINS_HASHFUNC(const wchar_t *s) { khint_t h = towlower(*s); if (h) for (++s ; *s; ++s) h = (h << 5) - h + towlower(*s); return h; }
  template<class T>
  BSS_FORCEINLINE khint_t KH_POINTER_HASHFUNC(T p) {
#ifdef BSS_64BIT
  return KH_INT64_HASHFUNC((__int64)p);
#else
  return (khint32_t)p;
#endif
  }

  inline bool KH_STR_EQUALFUNC(const char* a,const char* b) { return strcmp(a,b)==0; }
  inline bool KH_STRINS_EQUALFUNC(const char* a,const char* b) { return STRICMP(a,b)==0; }
  inline bool KH_STRW_EQUALFUNC(const wchar_t* a,const wchar_t* b) { return wcscmp(a,b)==0; }
  inline bool KH_STRWINS_EQUALFUNC(const wchar_t* a,const wchar_t* b) { return WCSICMP(a,b)==0; }
  template<class T>
  BSS_FORCEINLINE bool KH_INT_EQUALFUNC(T a,T b) { return a==b; }

  template<typename T,bool I> struct __cKh_KHGET { typedef typename std::remove_pointer<T>::type* KHGET; static const unsigned int INV=0; };
  template<typename T> struct __cKh_KHGET<T,true> { typedef T KHGET; static const KHGET INV=(KHGET)-1; };
  template<typename T> struct KHGET_cKh : __cKh_KHGET<T,std::is_integral<T>::value> { typedef typename __cKh_KHGET<T,std::is_integral<T>::value>::KHGET KHGET; };

  // Catch-all templatization for integral types. Automatically handles 64-bit integers
  template<typename T=void*, typename K=__int32, bool ismap=true>
  class BSS_COMPILER_DLLEXPORT cKhash_Int : public cKhash<K, T, ismap, &KH_INT_HASHFUNC<K>, &KH_INT_EQUALFUNC<K>, typename KHGET_cKh<T>::KHGET, (typename KHGET_cKh<T>::KHGET)KHGET_cKh<T>::INV>
  {
    typedef cKhash<K, T, ismap, &KH_INT_HASHFUNC<K>, &KH_INT_EQUALFUNC<K>, typename KHGET_cKh<T>::KHGET, (typename KHGET_cKh<T>::KHGET)KHGET_cKh<T>::INV> BASE;
  public:
    cKhash_Int() : BASE() {}
    cKhash_Int(cKhash_Int&& mov) : BASE(std::move(mov)) {}
    inline cKhash_Int& operator=(cKhash_Int&& right) { BASE::operator=(std::move(right)); return *this; }
  };

  template<typename T, bool ismap>
  class BSS_COMPILER_DLLEXPORT cKhash_Int<T,__int64,ismap> : public cKhash<__int64, T, ismap, &KH_INT64_HASHFUNC, &KH_INT_EQUALFUNC<__int64>, typename KHGET_cKh<T>::KHGET, (typename KHGET_cKh<T>::KHGET)KHGET_cKh<T>::INV>
  {
    typedef cKhash<__int64, T, ismap, &KH_INT64_HASHFUNC, &KH_INT_EQUALFUNC<__int64>, typename KHGET_cKh<T>::KHGET, (typename KHGET_cKh<T>::KHGET)KHGET_cKh<T>::INV> BASE;
  public:
    cKhash_Int() : BASE() {}
    cKhash_Int(cKhash_Int&& mov) : BASE(std::move(mov)) {}
    inline cKhash_Int& operator=(cKhash_Int&& right) { BASE::operator=(std::move(right)); return *this; }
  };

  template<typename T, bool ismap>
  class BSS_COMPILER_DLLEXPORT cKhash_Int<T,unsigned __int64,ismap> : public cKhash<unsigned __int64, T, ismap, &KH_UINT64_HASHFUNC, &KH_INT_EQUALFUNC<unsigned __int64>, typename KHGET_cKh<T>::KHGET, (typename KHGET_cKh<T>::KHGET)KHGET_cKh<T>::INV>
  {
    typedef cKhash<unsigned __int64, T, ismap, &KH_UINT64_HASHFUNC, &KH_INT_EQUALFUNC<unsigned __int64>, typename KHGET_cKh<T>::KHGET, (typename KHGET_cKh<T>::KHGET)KHGET_cKh<T>::INV> BASE;
  public:
    cKhash_Int() : BASE() {}
    cKhash_Int(cKhash_Int&& mov) : BASE(std::move(mov)) {}
    inline cKhash_Int& operator=(cKhash_Int&& right) { BASE::operator=(std::move(right)); return *this; }
  };

  template<typename T=void*, bool ismap=true>
  class BSS_COMPILER_DLLEXPORT cKhash_String : public cKhash<const char*, T, ismap, &KH_STR_HASHFUNC, &KH_STR_EQUALFUNC, typename KHGET_cKh<T>::KHGET, (typename KHGET_cKh<T>::KHGET)KHGET_cKh<T>::INV>
  {
    typedef cKhash<const char*, T, ismap, &KH_STR_HASHFUNC, &KH_STR_EQUALFUNC, typename KHGET_cKh<T>::KHGET, (typename KHGET_cKh<T>::KHGET)KHGET_cKh<T>::INV> BASE;
  public:
    cKhash_String() : BASE() {}
    cKhash_String(cKhash_String&& mov) : BASE(std::move(mov)) {}
    inline cKhash_String& operator=(cKhash_String&& right) { BASE::operator=(std::move(right)); return *this; }
  };

  template<typename T=void*, bool ismap=true>
  class BSS_COMPILER_DLLEXPORT cKhash_StringIns : public cKhash<const char*, T, ismap, &KH_STRINS_HASHFUNC, &KH_STRINS_EQUALFUNC, typename KHGET_cKh<T>::KHGET, (typename KHGET_cKh<T>::KHGET)KHGET_cKh<T>::INV>
  {
    typedef cKhash<const char*, T, ismap, &KH_STRINS_HASHFUNC, &KH_STRINS_EQUALFUNC, typename KHGET_cKh<T>::KHGET, (typename KHGET_cKh<T>::KHGET)KHGET_cKh<T>::INV> BASE;
  public:
    cKhash_StringIns() : BASE() {}
    cKhash_StringIns(cKhash_StringIns&& mov) : BASE(std::move(mov)) {}
    inline cKhash_StringIns& operator=(cKhash_StringIns&& right) { BASE::operator=(std::move(right)); return *this; }
  };

  template<typename T=void*, bool ismap=true>
  class BSS_COMPILER_DLLEXPORT cKhash_StringW : public cKhash<const wchar_t*, T, ismap, &KH_STRW_HASHFUNC, &KH_STRW_EQUALFUNC, typename KHGET_cKh<T>::KHGET, (typename KHGET_cKh<T>::KHGET)KHGET_cKh<T>::INV>
  {
    typedef cKhash<const wchar_t*, T, ismap, &KH_STRW_HASHFUNC, &KH_STRW_EQUALFUNC, typename KHGET_cKh<T>::KHGET, (typename KHGET_cKh<T>::KHGET)KHGET_cKh<T>::INV> BASE;
  public:
    cKhash_StringW() : BASE() {}
    cKhash_StringW(cKhash_StringW&& mov) : BASE(std::move(mov)) {}
    inline cKhash_StringW& operator=(cKhash_StringW&& right) { BASE::operator=(std::move(right)); return *this; }
  };

  template<typename T=void*, bool ismap=true>
  class BSS_COMPILER_DLLEXPORT cKhash_StringWIns : public cKhash<const wchar_t*, T, ismap, &KH_STRWINS_HASHFUNC, &KH_STRWINS_EQUALFUNC, typename KHGET_cKh<T>::KHGET, (typename KHGET_cKh<T>::KHGET)KHGET_cKh<T>::INV>
  {
    typedef cKhash<const wchar_t*, T, ismap, &KH_STRWINS_HASHFUNC, &KH_STRWINS_EQUALFUNC, typename KHGET_cKh<T>::KHGET, (typename KHGET_cKh<T>::KHGET)KHGET_cKh<T>::INV> BASE;
  public:
    cKhash_StringWIns() : BASE() {}
    cKhash_StringWIns(cKhash_StringWIns&& mov) : BASE(std::move(mov)) {}
    inline cKhash_StringWIns& operator=(cKhash_StringWIns&& right) { BASE::operator=(std::move(right)); return *this; }
  };

  template<typename T=void*, typename K=const void*, bool ismap=true>
  class BSS_COMPILER_DLLEXPORT cKhash_Pointer : public cKhash<K, T, ismap, &KH_POINTER_HASHFUNC<K>, &KH_INT_EQUALFUNC<K>, typename KHGET_cKh<T>::KHGET, (typename KHGET_cKh<T>::KHGET)KHGET_cKh<T>::INV>
  {
    typedef cKhash<K, T, ismap, &KH_POINTER_HASHFUNC<K>, &KH_INT_EQUALFUNC<K>, typename KHGET_cKh<T>::KHGET, (typename KHGET_cKh<T>::KHGET)KHGET_cKh<T>::INV> BASE;
  public:
    cKhash_Pointer() : BASE() {}
    cKhash_Pointer(cKhash_Pointer&& mov) : BASE(std::move(mov)) {}
    inline cKhash_Pointer& operator=(cKhash_Pointer&& right) { BASE::operator=(std::move(right)); return *this; }
  };
}

#endif
