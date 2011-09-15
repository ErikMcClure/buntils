// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_KHASH_H__BSS__
#define __C_KHASH_H__BSS__

#include "khash.h"
#include "bss_call.h"
#include "bss_defines.h"
#include "bss_deprecated.h"
#include "bss_traits.h"
#include <wchar.h>

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
static inline khint_t kh_get_template(const kh_template_t<khkey_t,khval_t,kh_is_map,__hash_func,__hash_equal> *h, khkey_t key)
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
static inline void kh_resize_template(kh_template_t<khkey_t,khval_t,kh_is_map,__hash_func,__hash_equal> *h, khint_t new_n_buckets)
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
static inline khint_t kh_put_template(kh_template_t<khkey_t,khval_t,kh_is_map,__hash_func,__hash_equal> *h, khkey_t key, int *ret) 
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

  template<typename khkey_t, typename khval_t, bool kh_is_map, khint_t (*__hash_func)(khkey_t), bool (*__hash_equal)(khkey_t, khkey_t), bool (*__validate_p)(khkey_t), class Traits=ValueTraits<khval_t>>
  class BSS_COMPILER_DLLEXPORT cKhash
  {
  protected:
    typedef khkey_t KHKEY;
    typedef khval_t KHVAL;
    typedef kh_template_t<KHKEY,KHVAL,kh_is_map,__hash_func,__hash_equal> KHTYPE;
    typedef typename Traits::const_reference const_ref;
    typedef typename Traits::pointer ptrtype;
    typedef typename Traits::reference mut_ref;

	public:
    inline cKhash(const cKhash& copy) : _h(kh_init_template<KHKEY,KHVAL,kh_is_map,__hash_func,__hash_equal>()), _cur(0)
		{
			operator =(copy);
		}
    inline cKhash(cKhash&& mov) : _h(mov._h), _cur(0)
    { 
      mov._h=0;
    }
		inline cKhash(unsigned int size=0) : _h(kh_init_template<KHKEY,KHVAL,kh_is_map,__hash_func,__hash_equal>()), _cur(0)
		{
			if(size<8)
        kh_resize_template(_h, 8);
      else
        kh_resize_template(_h, size);
		};
    inline ~cKhash()
		{
			kh_destroy_template(_h);
		}
    inline bool Insert(KHKEY key, const_ref value)
		{
      if(!__validate_p(key)) return false;
			if(kh_size(_h) >= _h->n_buckets) _resize();
			int r;
			khiter_t retval = kh_put_template(_h,key,&r);
			if(r>0) //Only insert the value if the key didn't exist
			{
        kh_val(_h,retval)=value;
        return true;
			}
			return false;
		}
    inline bool SetValue(khiter_t iterator, const_ref newvalue) { if(iterator >= kh_end(_h) || !kh_exist(_h, iterator)) return false; kh_val(_h, iterator)=newvalue; return true; }
    inline KHKEY GetIterKey(khiter_t iterator) { return kh_key(_h,iterator); }
    inline bool OverrideKeyPtr(khiter_t iterator, KHKEY key) { if(!__validate_p(key)) return false; kh_key(_h,iterator)=key; return true; }
    /* Gets the corresponding data attached to the given key */
    inline ptrtype GetKey(KHKEY key) const { if(!__validate_p(key)) return 0; khiter_t iterator= kh_get_template(_h, key); if(kh_end(_h) == iterator) return 0; return &kh_val(_h, iterator); }
    /* This is a pointer-only version of the above that simplifies verification when the type pointed to is a pointer */
    inline mut_ref GetKeyPtrOnly(KHKEY key) const { if(!__validate_p(key)) return 0; khiter_t iterator= kh_get_template(_h, key); if(kh_end(_h) == iterator) return 0; return kh_val(_h, iterator); }
    /* Gets the corresponding data attached to the given iterator */
    inline ptrtype Get(khiter_t iterator) const { if(iterator >= kh_end(_h) || !kh_exist(_h, iterator)) return 0; return &kh_val(_h, iterator); }
    /* This is a pointer-only version of the above that simplifies verification when the type pointed to is a pointer */
    inline mut_ref GetPtrOnly(khiter_t iterator) const { if(iterator >= kh_end(_h) || !kh_exist(_h, iterator)) return 0; return kh_val(_h, iterator); }
    inline khiter_t GetIterator(KHKEY key) const { return !(__validate_p(key))?0:kh_get_template(_h, key); }
    inline void SetSize(unsigned int size) { if(_h->n_buckets < size) kh_resize_template(_h,size); }
		inline bool Remove(KHKEY key) const
    {
      if(!__validate_p(key)) return false;
      khiter_t iterator = kh_get_template(_h, key);
      if(kh_end(_h) == iterator) return false;
			//KHVAL retval=kh_val(_h, iterator);
			//if(retval!=0)
			//{
				kh_del_template(_h, iterator);
			//}
			return true;
    }
    inline bool RemoveIterator(khiter_t iterator) const
    {
      if(!Exists(iterator)) return false;
			//KHVAL retval=kh_val(_h, iterator);
			//if(retval!=0)
			//{
				kh_del_template(_h, iterator);
			//}
			return true;
    }
		inline void Clear() { kh_clear_template(_h); }
		inline unsigned int Length() const { return kh_size(_h); }
		inline unsigned int Size() const { return _h->n_buckets; }
		inline khiter_t Start() const { return kh_begin(_h); }
		inline khiter_t End() const { return kh_end(_h); }
    inline bool Exists(khiter_t iterator) const { if(iterator<_h->n_buckets) return kh_exist(_h, iterator)!=0; return false; }
		inline void ResetWalk() const { _cur=(khiter_t)-1; }
		inline khiter_t GetNext() const { while((++_cur)<kh_end(_h) && !kh_exist(_h, _cur)); return _cur<kh_end(_h)?_cur:kh_end(_h); }
		inline khiter_t GetPrev() const { while((--_cur)<kh_end(_h) && !kh_exist(_h, _cur)); return _cur<kh_end(_h)?_cur:kh_end(_h); }
		inline cKhash& operator =(const cKhash& right)
		{
      if(&right == this) return *this;
			kh_clear_template(_h);
			kh_resize_template(_h, right._h->n_buckets);
			right.ResetWalk();
			int r;
			khiter_t cur;
			while((cur=right.GetNext())!= right.End())
				kh_val(_h, kh_put_template(_h, kh_key(right._h, cur),&r))=kh_val(right._h, cur);
			_cur=0;
			return *this; 
		}
		inline cKhash& operator =(cKhash&& mov)
    {
      if(&mov == this) return *this;
			kh_destroy_template(_h);
      _h=mov._h;
      mov._h=0;
      _cur=0;
      return *this;
    }
    inline mut_ref operator[](khiter_t iterator) { return kh_val(_h, iterator); }
    inline const_ref operator[](khiter_t iterator) const { return kh_val(_h, iterator); }
    inline bool Nullified() const { return !_h; }

	protected:
		void _resize()
		{ 
			kh_resize_template(_h,kh_size(_h)*2);
		}

		KHTYPE* _h;
		mutable khiter_t _cur; //holds current iterator for walking through
  };

  inline khint_t KH_INT64_HASHFUNC(__int64 key) { return (khint32_t)((key)>>33^(key)^(key)<<11); }
  inline khint_t KH_INT_HASHFUNC(__int32 key) { return (khint32_t)key; }
  inline khint_t KH_STR_HASHFUNC(const char * s) { khint_t h = *s; if (h) for (++s ; *s; ++s) h = (h << 5) - h + *s; return h; }
  inline khint_t KH_STRINS_HASHFUNC(const char *s) { khint_t h = ((*s)>64&&(*s)<91)?(*s)+32:*s;	if (h) for (++s ; *s; ++s) h = (h << 5) - h + (((*s)>64&&(*s)<91)?(*s)+32:*s); return h; }
  inline khint_t KH_STRW_HASHFUNC(const wchar_t * s) { khint_t h = *s; if (h) for (++s ; *s; ++s) h = (h << 5) - h + *s; return h; }
  inline khint_t KH_STRWINS_HASHFUNC(const wchar_t *s) { khint_t h = towlower(*s); if (h) for (++s ; *s; ++s) h = (h << 5) - h + towlower(*s); return h; }
  inline khint_t KH_POINTER_HASHFUNC(void* p) {
#ifdef BSS64BIT
  return (khint32_t)((p)>>33^(p)^(p)<<11);
#else
  return (khint32_t)p;
#endif
  }

  inline bool KH_STR_EQUALFUNC(const char* a,const char* b) { return strcmp(a,b)==0; }
  inline bool KH_STRINS_EQUALFUNC(const char* a,const char* b) { return STRICMP(a,b)==0; }
  inline bool KH_STRW_EQUALFUNC(const wchar_t* a,const wchar_t* b) { return wcscmp(a,b)==0; }
  inline bool KH_STRWINS_EQUALFUNC(const wchar_t* a,const wchar_t* b) { return WCSICMP(a,b)==0; }
  template<class T>
  inline bool KH_INT_EQUALFUNC(T a,T b) { return a==b; }

  template<class T>
  inline bool KH_INT_VALIDATEPTR(T p) { return true; }
  template<class T>
  inline bool KH_STR_VALIDATEPTR(T p) { return p!=0; }

  template<typename T=void*, bool ismap=true>
  class BSS_COMPILER_DLLEXPORT cKhash_Int64 : public cKhash<__int64, T, true, &KH_INT64_HASHFUNC, &KH_INT_EQUALFUNC<__int64>, &KH_INT_VALIDATEPTR<__int64>>
  {
  public:
    cKhash_Int64() : cKhash<__int64, T, true, &KH_INT64_HASHFUNC, &KH_INT_EQUALFUNC<__int64>, &KH_INT_VALIDATEPTR<__int64>>() {}
    cKhash_Int64(cKhash_Int64&& mov) : cKhash(std::move(mov)) {}
    cKhash_Int64& operator=(cKhash_Int64&& right) { cKhash::operator=(std::move(right)); return *this; }
  };

  template<typename T=void*, bool ismap=true>
  class BSS_COMPILER_DLLEXPORT cKhash_Int : public cKhash<__int32, T, true, &KH_INT_HASHFUNC, &KH_INT_EQUALFUNC<__int32>, &KH_INT_VALIDATEPTR<__int32>>
  {
  public:
    cKhash_Int() : cKhash<__int32, T, true, &KH_INT_HASHFUNC, &KH_INT_EQUALFUNC<__int32>, &KH_INT_VALIDATEPTR<__int32>>() {}
    cKhash_Int(cKhash_Int&& mov) : cKhash(std::move(mov)) {}
    cKhash_Int& operator=(cKhash_Int&& right) { cKhash::operator=(std::move(right)); return *this; }
  };

  template<typename T=void*, bool ismap=true>
  class BSS_COMPILER_DLLEXPORT cKhash_String : public cKhash<const char*, T, true, &KH_STR_HASHFUNC, &KH_STR_EQUALFUNC, &KH_STR_VALIDATEPTR<const char*>>
  {
  public:
    cKhash_String() : cKhash<const char*, T, true, &KH_STR_HASHFUNC, &KH_STR_EQUALFUNC, &KH_STR_VALIDATEPTR<const char*>>() {}
    cKhash_String(cKhash_String&& mov) : cKhash(std::move(mov)) {}
    cKhash_String& operator=(cKhash_String&& right) { cKhash::operator=(std::move(right)); return *this; }
  };

  template<typename T=void*, bool ismap=true>
  class BSS_COMPILER_DLLEXPORT cKhash_StringIns : public cKhash<const char*, T, true, &KH_STRINS_HASHFUNC, &KH_STRINS_EQUALFUNC, &KH_STR_VALIDATEPTR<const char*>>
  {
  public:
    cKhash_StringIns() : cKhash<const char*, T, true, &KH_STRINS_HASHFUNC, &KH_STRINS_EQUALFUNC, &KH_STR_VALIDATEPTR<const char*>>() {}
    cKhash_StringIns(cKhash_StringIns&& mov) : cKhash(std::move(mov)) {}
    cKhash_StringIns& operator=(cKhash_StringIns&& right) { cKhash::operator=(std::move(right)); return *this; }
  };

  template<typename T=void*, bool ismap=true>
  class BSS_COMPILER_DLLEXPORT cKhash_StringW : public cKhash<const wchar_t*, T, true, &KH_STRW_HASHFUNC, &KH_STRW_EQUALFUNC, &KH_STR_VALIDATEPTR<const wchar_t*>>
  {
  public:
    cKhash_StringW() : cKhash<const wchar_t*, T, true, &KH_STRW_HASHFUNC, &KH_STRW_EQUALFUNC, &KH_STR_VALIDATEPTR<const wchar_t*>>() {}
    cKhash_StringW(cKhash_StringW&& mov) : cKhash(std::move(mov)) {}
    cKhash_StringW& operator=(cKhash_StringW&& right) { cKhash::operator=(std::move(right)); return *this; }
  };

  template<typename T=void*, bool ismap=true>
  class BSS_COMPILER_DLLEXPORT cKhash_StringWIns : public cKhash<const wchar_t*, T, true, &KH_STRWINS_HASHFUNC, &KH_STRWINS_EQUALFUNC, &KH_STR_VALIDATEPTR<const wchar_t*>>
  {
  public:
    cKhash_StringWIns() : cKhash<const wchar_t*, T, true, &KH_STRWINS_HASHFUNC, &KH_STRWINS_EQUALFUNC, &KH_STR_VALIDATEPTR<const wchar_t*>>() {}
    cKhash_StringWIns(cKhash_StringWIns&& mov) : cKhash(std::move(mov)) {}
    cKhash_StringWIns& operator=(cKhash_StringWIns&& right) { cKhash::operator=(std::move(right)); return *this; }
  };

  template<typename T=void*, bool ismap=true>
  class BSS_COMPILER_DLLEXPORT cKhash_Pointer : public cKhash<void*, T, true, &KH_POINTER_HASHFUNC, &KH_INT_EQUALFUNC<void*>, &KH_STR_VALIDATEPTR<void*>>
  {
  public:
    cKhash_Pointer() : cKhash<void*, T, true, &KH_POINTER_HASHFUNC, &KH_INT_EQUALFUNC<void*>, &KH_STR_VALIDATEPTR<void*>>() {}
    cKhash_Pointer(cKhash_Pointer&& mov) : cKhash(std::move(mov)) {}
    cKhash_Pointer& operator=(cKhash_Pointer&& right) { cKhash::operator=(std::move(right)); return *this; }
  };

  
  //These are partial explicitely initialized template versions of the string classes for use in template situations. I'd use typedef'd versions of these instead of the ones above, but because of C++ restrictions they'd be extremely messy.
  template<typename K=char, typename T=void*, bool ismap=true>
  class BSS_COMPILER_DLLEXPORT cKhash_StringT : public cKhash<const K*, T, true, &KH_STR_HASHFUNC, &KH_STR_EQUALFUNC, &KH_STR_VALIDATEPTR<const K*>>
  {
  public:
    cKhash_StringT() : cKhash<const K*, T, true, &KH_STR_HASHFUNC, &KH_STR_EQUALFUNC, &KH_STR_VALIDATEPTR<const K*>>() {}
    cKhash_StringT(cKhash_StringT&& mov) : cKhash(std::move(mov)) {}
    cKhash_StringT& operator=(cKhash_StringT&& right) { cKhash::operator=(std::move(right)); return *this; }
  };
  
  template<typename T, bool ismap>
  class BSS_COMPILER_DLLEXPORT cKhash_StringT<char,T,ismap> : public cKhash<const char*, T, true, &KH_STR_HASHFUNC, &KH_STR_EQUALFUNC, &KH_STR_VALIDATEPTR<const char*>>
  {
  public:
    cKhash_StringT() : cKhash<const char*, T, true, &KH_STR_HASHFUNC, &KH_STR_EQUALFUNC, &KH_STR_VALIDATEPTR<const char*>>() {}
    cKhash_StringT(cKhash_StringT&& mov) : cKhash(std::move(mov)) {}
    cKhash_StringT& operator=(cKhash_StringT&& right) { cKhash::operator=(std::move(right)); return *this; }
  };
  
  template<typename T, bool ismap>
  class BSS_COMPILER_DLLEXPORT cKhash_StringT<wchar_t,T,ismap> : public cKhash<const wchar_t*, T, true, &KH_STRW_HASHFUNC, &KH_STRW_EQUALFUNC, &KH_STR_VALIDATEPTR<const wchar_t*>>
  {
  public:
    cKhash_StringT() : cKhash<const wchar_t*, T, true, &KH_STRW_HASHFUNC, &KH_STRW_EQUALFUNC, &KH_STR_VALIDATEPTR<const wchar_t*>>() {}
    cKhash_StringT(cKhash_StringT&& mov) : cKhash(std::move(mov)) {}
    cKhash_StringT& operator=(cKhash_StringT&& right) { cKhash::operator=(std::move(right)); return *this; }
  };

  template<typename K=char, typename T=void*, bool ismap=true>
  class BSS_COMPILER_DLLEXPORT cKhash_StringTIns : public cKhash<const K*, T, true, &KH_STRINS_HASHFUNC, &KH_STRINS_EQUALFUNC, &KH_STR_VALIDATEPTR<const K*>>
  {
  public:
    cKhash_StringTIns() : cKhash<const K*, T, true, &KH_STRINS_HASHFUNC, &KH_STRINS_EQUALFUNC, &KH_STR_VALIDATEPTR<const K*>>() {}
    cKhash_StringTIns(cKhash_StringTIns&& mov) : cKhash(std::move(mov)) {}
    cKhash_StringTIns& operator=(cKhash_StringTIns&& right) { cKhash::operator=(std::move(right)); return *this; }
  };

  template<typename T, bool ismap>
  class BSS_COMPILER_DLLEXPORT cKhash_StringTIns<char,T,ismap> : public cKhash<const char*, T, true, &KH_STRINS_HASHFUNC, &KH_STRINS_EQUALFUNC, &KH_STR_VALIDATEPTR<const char*>>
  {
  public:
    cKhash_StringTIns() : cKhash<const char*, T, true, &KH_STRINS_HASHFUNC, &KH_STRINS_EQUALFUNC, &KH_STR_VALIDATEPTR<const char*>>() {}
    cKhash_StringTIns(cKhash_StringTIns&& mov) : cKhash(std::move(mov)) {}
    cKhash_StringTIns& operator=(cKhash_StringTIns&& right) { cKhash::operator=(std::move(right)); return *this; }
  };

  template<typename T, bool ismap>
  class BSS_COMPILER_DLLEXPORT cKhash_StringTIns<wchar_t,T,ismap> : public cKhash<const wchar_t*, T, true, &KH_STRWINS_HASHFUNC, &KH_STRWINS_EQUALFUNC, &KH_STR_VALIDATEPTR<const wchar_t*>>
  {
  public:
    cKhash_StringTIns() : cKhash<const wchar_t*, T, true, &KH_STRWINS_HASHFUNC, &KH_STRWINS_EQUALFUNC, &KH_STR_VALIDATEPTR<const wchar_t*>>() {}
    cKhash_StringTIns(cKhash_StringTIns&& mov) : cKhash(std::move(mov)) {}
    cKhash_StringTIns& operator=(cKhash_StringTIns&& right) { cKhash::operator=(std::move(right)); return *this; }
  };
}

#endif