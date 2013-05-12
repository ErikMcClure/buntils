// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_ALLOC_ADDITIVE_H__
#define __BSS_ALLOC_ADDITIVE_H__

#include "bss_alloc.h"
#include "bss_util.h"

namespace bss_util {
  struct AFLISTITEM
  {
    AFLISTITEM* next;
    size_t size;
  };

  // Dynamic additive allocator that can allocate any number of bytes
	class BSS_COMPILER_DLLEXPORT cAdditiveAlloc
  {
  public:
    explicit cAdditiveAlloc(size_t init=64) : _curpos(0), _root(0)
    {
      _allocchunk(init);
    }
    ~cAdditiveAlloc()
    {
      AFLISTITEM* hold=_root;
      while(_root=hold)
      {
        hold=_root->next;
        free(_root);
      }
    }
    template<typename T>
	  inline T* BSS_FASTCALL allocT(size_t num)
    {
      return (T*)alloc(num*sizeof(T));
    }
	  inline void* BSS_FASTCALL alloc(size_t _sz)
    {
#ifdef BSS_DISABLE_CUSTOM_ALLOCATORS
      return malloc(_sz);
#endif
      if((_curpos+_sz)>=_root->size) { _allocchunk(fbnext(bssmax(_root->size,_sz))); _curpos=0; }

      void* retval= ((char*)(_root+1))+_curpos;
      _curpos+=_sz;
      return retval;
    }
    inline void BSS_FASTCALL dealloc(void* p)
    {
#ifdef BSS_DISABLE_CUSTOM_ALLOCATORS
      delete p; return;
#endif
#ifdef BSS_DEBUG
      AFLISTITEM* cur=_root;
      bool found=false;
      while(cur)
      {
        if(p>=(cur+1) && p<(((char*)(cur+1))+cur->size)) { found=true; break; }
        cur=cur->next;
      }
      assert(found);
      //memset(p,0xFEEEFEEE,sizeof(T)); //No way to know how big this is
#endif
    }
    inline void Clear()
    {
      _curpos=0;
      if(!_root->next) return;
      size_t nsize=0;
      AFLISTITEM* hold;
      while(_root)
      {
        hold=_root->next;
        nsize+=_root->size;
        free(_root);
        _root=hold;
      }
      _allocchunk(nsize); //consolidates all memory into one chunk to try and take advantage of data locality
    }
  protected:
    inline BSS_FORCEINLINE void _allocchunk(size_t nsize)
    {
      AFLISTITEM* retval=(AFLISTITEM*)malloc(sizeof(AFLISTITEM)+nsize);
      retval->next=_root;
      retval->size=nsize;
      _root=retval;
      assert(_prepDEBUG());
    }
    inline bool _prepDEBUG()
    {
      if(!_root) return false;
      memset(_root+1,0xcdcdcdcd,_root->size);
      return true;
    }

    AFLISTITEM* _root;
    size_t _curpos;
  };

	template<typename T>
  class BSS_COMPILER_DLLEXPORT AdditivePolicy : public AllocPolicySize<T>, protected cAdditiveAlloc {
	public:
    typedef typename AllocPolicySize<T>::pointer pointer;
    template<typename U>
    struct rebind { typedef AdditivePolicy<U> other; };

    inline explicit AdditivePolicy() {}
    inline ~AdditivePolicy() {}
    inline explicit AdditivePolicy(AdditivePolicy const&) {}
    template <typename U>
    inline explicit AdditivePolicy(AdditivePolicy<U> const&) {}

    inline pointer allocate(std::size_t cnt, 
      typename std::allocator<void>::const_pointer = 0) {
        return cAdditiveAlloc::allocT<T>(cnt);
    }
    inline void deallocate(pointer p, std::size_t num = 0) { }
    inline void BSS_FASTCALL clear() { cAdditiveAlloc::Clear(); } //done for functor reasons, BSS_FASTCALL has no effect here
	};
}


#endif


  // Begin Additive Fixed Allocator

	/*template<typename T>
	class BSS_COMPILER_DLLEXPORT cAdditiveFixedAllocator
  {
  public:
    cAdditiveFixedAllocator(size_t init=8) : _curpos(0), _root(0)
    {
      _allocchunk(init*sizeof(T));
    }
    ~cAdditiveFixedAllocator()
    {
      AFLISTITEM* hold=_root;
      while(_root=hold)
      {
        hold=_root->next;
        free(_root->mem);
        free(_root);
      }
    }
	  inline T* BSS_FASTCALL alloc(size_t num)
    {
      assert(num==1);
#ifdef BSS_DISABLE_CUSTOM_ALLOCATORS
      return (T*)malloc(num*sizeof(T));
#endif
      if(_curpos>=_root->size) { _allocchunk(fbnext(_root->size/sizeof(T))*sizeof(T)); _curpos=0; }

      T* retval= (T*)(_root->mem+_curpos);
      _curpos+=sizeof(T);
      return retval;
    }
    inline void BSS_FASTCALL dealloc(void* p)
    {
#ifdef BSS_DISABLE_CUSTOM_ALLOCATORS
      delete p; return;
#endif
      assert(p!=0);
#ifdef BSS_DEBUG
      AFLISTITEM* cur=_root;
      bool found=false;
      while(cur)
      {
        if(p>=cur->mem && p<(cur->mem+cur->size)) { found=true; break; }
        cur=cur->next;
      }
      assert(found);
      memset(p,0xFEEEFEEE,sizeof(T));
#endif
    }
    inline void Clear()
    {
      _curpos=0;
      if(!_root->next) return;
      size_t nsize=0;
      AFLISTITEM* hold;
      while(_root)
      {
        hold=_root->next;
        nsize+=_root->size;
        free(_root->mem);
        free(_root);
        _root=hold;
      }
      //_allocchunk(nsize*sizeof(T)); //don't do this, nsize is already in bytes
      _allocchunk(nsize); //consolidates all memory into one chunk to try and take advantage of data locality
    }
  protected:
    inline BSS_FORCEINLINE void _allocchunk(size_t nsize)
    {
      AFLISTITEM* retval=(AFLISTITEM*)malloc(sizeof(AFLISTITEM));
      retval->next=_root;
      retval->size=nsize;
      retval->mem=(char*)malloc(retval->size);
      _root=retval;
      assert(_prepDEBUG());
    }
    inline BSS_FORCEINLINE bool _prepDEBUG()
    {
      if(!_root || !_root->mem) return false;
      memset(_root->mem,0xcdcdcdcd,_root->size);
      return true;
    }

    AFLISTITEM* _root;
    size_t _curpos;
  };
  
	template<typename T>
  class BSS_COMPILER_DLLEXPORT AdditiveFixedPolicy : public AllocPolicySize<T>, protected cAdditiveFixedAllocator<T> {
	public:
    typedef typename AllocPolicySize<T>::pointer pointer;
    template<typename U>
    struct rebind { typedef AdditiveFixedPolicy<U> other; };

    inline explicit AdditiveFixedPolicy() {}
    inline ~AdditiveFixedPolicy() {}
    inline explicit AdditiveFixedPolicy(AdditiveFixedPolicy const&) {}
    template <typename U>
    inline explicit AdditiveFixedPolicy(AdditiveFixedPolicy<U> const&) {}

    inline pointer allocate(std::size_t cnt, 
      typename std::allocator<void>::const_pointer = 0) {
        return cAdditiveFixedAllocator<T>::alloc(cnt);
    }
    inline void deallocate(pointer p, std::size_t num = 0) { }
    inline void BSS_FASTCALL clear() { cAdditiveFixedAllocator<T>::Clear(); } //done for functor reasons, BSS_COMPILER_FASTCALL has no effect here
	};

  // End Additive Fixed Allocator

  BSSBUILD_STATIC_POLICY(StaticAdditiveFixed,AdditiveFixedPolicy);*/

  // Begin AdditiveChunkPolicy
/*  struct MEMBUCKET
	{
		MEMBUCKET(size_t _size) : size(_size), mem((char*)malloc(_size)), next(0) {}
    ~MEMBUCKET() { if(mem) { free((void*)mem); } } //a failed allocation creates this possibility
		size_t size;
		MEMBUCKET* next;
		char* mem;
	};

	template<typename T>
	class BSS_COMPILER_DLLEXPORT AdditiveChunkPolicy {
	public:
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    template<typename U>
    struct rebind { typedef AdditiveChunkPolicy<U> other; };

    inline explicit AdditiveChunkPolicy(unsigned int startbuffer=4, unsigned int exp_increase=2, unsigned int lin_increase=0)
		{
			_memroot=_memlast=new MEMBUCKET(startbuffer*sizeof(T));

      _lin_inc = exp_increase<=1?sizeof(T):lin_increase*sizeof(T); //this gaurentees that there's at least a linear increase of 1
      _exp_inc = exp_increase<1?1:exp_increase;
			_bufsize = startbuffer*sizeof(T);
			_curpos = 0;
		}

    inline ~AdditiveChunkPolicy()
		{
			while(_memroot)
			{
				_memlast=_memroot->next;
				delete _memroot;
				_memroot=_memlast;
			}
		}
    inline explicit AdditiveChunkPolicy(AdditiveChunkPolicy const& copy)
    {
			_bufsize = copy._bufsize;
			_memroot=_memlast=new MEMBUCKET(_bufsize);

      _lin_inc = copy._lin_inc;
			_exp_inc = copy._exp_inc;
			_curpos = 0;
    }
    template <typename U>
    inline explicit AdditiveChunkPolicy(AdditiveChunkPolicy<U> const& copy)
    {
      _bufsize = copy.BUFSIZE() + copy.BUFSIZE()%sizeof(T);
			_memroot=_memlast=new MEMBUCKET(_bufsize);

      _lin_inc = copy.LININC();
      _exp_inc = copy.EXPINC();
			_curpos = 0;
    }

    inline pointer allocate(size_type cnt, typename std::allocator<void>::const_pointer = 0)
		{
			unsigned int pos = _curpos; //pos is concerned with the BEGINNING of our allocation - _curpos is the end.
      _curpos += cnt*sizeof(T);
      if(_curpos >= _bufsize)
      { 
        unsigned int newamount=(_bufsize*_exp_inc)+_lin_inc;
        while(newamount-_bufsize<cnt*sizeof(T))
          newamount=(newamount*_exp_inc)+_lin_inc;
        _resize(newamount);
      }

			MEMBUCKET* _cur=_memroot;
			while(_cur)
			{
				if(pos < _cur->size) //if this is true we've hit the one we want.
          break;
				pos -= _cur->size;
				_cur=_cur->next;
			}

      //Now we ensure the memory we hit is big enough to hold the allocation
      if(pos+cnt*sizeof(T)>_cur->size) //I have been unable to determine if this is an off by one error.
      {
        pos=0;
        _curpos+=_cur->size-pos; //skip to the end of this one
				_cur=_cur->next;
        while(cnt*sizeof(T)>_cur->size)
        {
          _curpos+=_cur->size;
          _cur=_cur->next;
        }
      }

			return (pointer)(_cur->mem+pos);
    }
		inline void deallocate(pointer p, size_type n=0) { }

#ifdef max
#undef max
#define REDEFINE_MAX_MACRO
#endif

    inline size_type max_size() const { return std::numeric_limits<size_type>::max(); }

#ifdef REDEFINE_MAX_MACRO
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

		inline void Clear() { _curpos=0; }
    inline unsigned int LININC() const { return _lin_inc; }
    inline unsigned int EXPINC() const { return _exp_inc; }
    inline unsigned int BUFSIZE() const { return _bufsize; }

	protected:
		inline void _resize(unsigned int num)
		{
			if(num<=_bufsize) return; //You can't shrink an additive allocator (and for that matter you probably shouldn't be shrinking any allocator)
			unsigned int size = num-_bufsize;
			_memlast->next = new MEMBUCKET(size);
			_memlast=_memlast->next;
			_bufsize=num;
		}
    MEMBUCKET* _memroot;
    MEMBUCKET* _memlast;
    unsigned int _lin_inc;
    unsigned int _exp_inc;
    unsigned int _bufsize;
    unsigned int _curpos;
    bool _exp;
	};

	template<typename T, typename T2>
	inline bool operator==(AdditiveChunkPolicy<T> const&, AdditiveChunkPolicy<T2> const&) { return true; }
	template<typename T, typename OtherAllocator> inline bool operator==(AdditiveChunkPolicy<T> const&, OtherAllocator const&) { return false; }

  // End Additive Chunk Policy
/*
	template<typename T>
	class BSS_COMPILER_DLLEXPORT AdditiveChunkAllocator : public AdditiveChunkPolicy<T>, public ObjectTraits<T> {
	private:
    typedef AdditiveChunkPolicy<T> AllocationPolicy;
    typedef ObjectTraits<T> TTraits;

	public: 
    typedef typename AllocationPolicy::size_type size_type;
    typedef typename AllocationPolicy::difference_type difference_type;
    typedef typename AllocationPolicy::pointer pointer;
    typedef typename AllocationPolicy::const_pointer const_pointer;
    typedef typename AllocationPolicy::reference reference;
    typedef typename AllocationPolicy::const_reference const_reference;
    typedef typename AllocationPolicy::value_type value_type;

    template<typename U>
    struct rebind {
        typedef AdditiveChunkAllocator<U> other;
    };

		inline explicit AdditiveChunkAllocator(unsigned int startbuffer=4, unsigned int exp_increase=0, unsigned int lin_increase=0) : AdditiveChunkPolicy<T>(startbuffer,exp_increase,lin_increase) {}
    inline ~AdditiveChunkAllocator() {}
    inline AdditiveChunkAllocator(AdditiveChunkAllocator const& rhs):ObjectTraits<T>(rhs), AdditiveChunkPolicy<T>(rhs) {}
    template <typename U>
    inline AdditiveChunkAllocator(AdditiveChunkAllocator<U> const& rhs):ObjectTraits<T>(rhs), AdditiveChunkPolicy<T>(rhs) {}
	};*/