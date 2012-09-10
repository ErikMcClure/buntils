// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_ALLOC_H__
#define __BSS_ALLOC_H__

#include "bss_call.h"
#include <limits>
#include <xmemory>
#include <assert.h>

namespace bss_util {
  template<class T>
  class BSS_COMPILER_DLLEXPORT AllocPolicySize
  {
  public:
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef T *pointer;
    typedef const T *const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

  #ifdef max
  #define REDEFMAX
  #undef max
  #endif
      inline std::size_t max_size() const { return std::numeric_limits<std::size_t>::max(); }

  #ifdef REDEFMAX
  #define max(a,b)            (((a) > (b)) ? (a) : (b))
  #endif
  };

	template<typename T>
  class BSS_COMPILER_DLLEXPORT StandardAllocPolicy : public AllocPolicySize<T> {
	public:
    template<typename U>
    struct rebind { typedef StandardAllocPolicy<U> other; };

    inline explicit StandardAllocPolicy() {}
    inline ~StandardAllocPolicy() {}
    inline explicit StandardAllocPolicy(StandardAllocPolicy const&) {}
    template <typename U>
    inline explicit StandardAllocPolicy(StandardAllocPolicy<U> const&) {}

    inline pointer allocate(std::size_t cnt, 
      typename std::allocator<void>::const_pointer = 0) { 
        return reinterpret_cast<pointer>(::operator new(cnt * sizeof (T))); 
    }
    inline void deallocate(pointer p, std::size_t = 0) { ::operator delete(p); }
	};
  
	template<typename T, typename Alloc>
  class BSS_COMPILER_DLLEXPORT StaticAllocPolicy : public AllocPolicySize<T> {
	public:
    template<typename U>
    struct rebind { typedef StaticAllocPolicy<U,Alloc> other; };

    inline explicit StaticAllocPolicy() {}
    inline ~StaticAllocPolicy() {}
    inline explicit StaticAllocPolicy(StaticAllocPolicy const&) {}
    template <typename U>
    inline explicit StaticAllocPolicy(StaticAllocPolicy<U,Alloc> const&) {}

    inline pointer allocate(std::size_t cnt, typename std::allocator<void>::const_pointer = 0) { return _alloc.allocate(cnt); }
    inline void deallocate(pointer p, std::size_t = 0) { _alloc.deallocate(p); }

    static Alloc _alloc;
	};

#define BSSBUILD_STATIC_POLICY(name,alloc) template<class T> \
  class BSS_COMPILER_DLLEXPORT name : public StaticAllocPolicy<T,alloc<T>> \
  { \
  public: \
    template<typename U> \
    struct rebind { typedef name<U> other; }; \
    inline explicit name() {} \
    inline ~name() {} \
    inline explicit name(name const&) {} \
    template <typename U> \
    inline explicit name(name<U> const&) {} \
  }

	template<typename T, typename T2>
	inline bool operator==(StandardAllocPolicy<T> const&, StandardAllocPolicy<T2> const&) { return true; }
	template<typename T, typename OtherAllocator> inline bool operator==(StandardAllocPolicy<T> const&, OtherAllocator const&) { return false; }

	template<typename T>
	class BSS_COMPILER_DLLEXPORT ObjectTraits {
	public: 
    template<typename U>
    struct rebind { typedef ObjectTraits<U> other; };

    inline explicit ObjectTraits() {}
    inline ~ObjectTraits() {}
    template <typename U>
    inline explicit ObjectTraits(ObjectTraits<U> const&) {}

    inline static T* address(T& r) { return &r; }
    inline static T const* address(T const& r) { return &r; }

    inline static void construct(T* p, const T& t) { new(p) T(t); }
    inline static void destroy(T* p) { p->~T(); }
	}; 

	template<typename T, typename Policy = StandardAllocPolicy<T>, typename Traits = ObjectTraits<T>>
#ifndef BSS_DISABLE_CUSTOM_ALLOCATORS
	class BSS_COMPILER_DLLEXPORT Allocator : public Policy {
	private:
    typedef Policy AllocationPolicy;
#else
	class BSS_COMPILER_DLLEXPORT Allocator : public StandardAllocPolicy<T>, public ObjectTraits<T> {
	private:
    typedef StandardAllocPolicy<T> AllocationPolicy;
#endif
    typedef Traits TTraits;

	public: 
    typedef typename AllocationPolicy::size_type size_type;
    typedef typename AllocationPolicy::pointer pointer;
    typedef typename AllocationPolicy::const_pointer const_pointer;
    typedef typename AllocationPolicy::reference reference;
    typedef typename AllocationPolicy::const_reference const_reference;

    template<typename U>
    struct rebind {
        typedef Allocator<U, typename AllocationPolicy::rebind<U>::other, 
            typename TTraits::rebind<U>::other > other;
    };
    template<typename U>
    Allocator& operator=(const Allocator<U>&) { return *this; }

    inline explicit Allocator() {}
    inline ~Allocator() {}
    inline Allocator(Allocator const& rhs) : AllocationPolicy(rhs) {}
    template <typename U>
    inline Allocator(Allocator<U> const&) {}
    template <typename U, typename P, typename T2>
    inline Allocator(Allocator<U, P, T2> const& rhs) : AllocationPolicy(rhs) {}

    // These methods are called for classes that have constructors and destructors
    //void construct(pointer ptr, const_reference val) { new ((void *)ptr) T(val); }
    //void destroy(pointer ptr) { ptr->T::~T(); }
	};

	template<typename T, typename P, typename Tr>
	inline bool operator==(Allocator<T, P, 
		 Tr> const& lhs, Allocator<T, 
		 P, Tr> const& rhs) { 
			return operator==(static_cast<P const&>(lhs), 
												 static_cast<P const&>(rhs)); 
	}
	template<typename T, typename P, typename Tr, 
					typename T2, typename P2, typename Tr2>
	inline bool operator==(Allocator<T, P, 
			Tr> const& lhs, Allocator<T2, P2, Tr2> const& rhs) { 
				return operator==(static_cast<P const&>(lhs), 
												 static_cast<P2 const&>(rhs)); 
	}
	template<typename T, typename P, typename Tr, typename OtherAllocator>
	inline bool operator==(Allocator<T, P, 
						Tr> const& lhs, OtherAllocator const& rhs) { 
			return operator==(static_cast<P const&>(lhs), rhs); 
	}
	template<typename T, typename P, typename Tr>
	inline bool operator!=(Allocator<T, P, Tr> const& lhs, 
													 Allocator<T, P, Tr> const& rhs) { 
			return !operator==(lhs, rhs); 
	}
	template<typename T, typename P, typename Tr, 
						 typename T2, typename P2, typename Tr2>
	inline bool operator!=(Allocator<T, P, Tr> const& lhs, 
										 Allocator<T2, P2, Tr2> const& rhs) { 
			return !operator==(lhs, rhs); 
	}
	template<typename T, typename P, typename Tr, 
																typename OtherAllocator>
	inline bool operator!=(Allocator<T, P, 
					Tr> const& lhs, OtherAllocator const& rhs) { 
			return !operator==(lhs, rhs); 
	}

	template<typename T, typename _Ax>
	class i_AllocTracker
	{
    typedef typename _Ax::pointer pointer;
	public:
		inline i_AllocTracker(const i_AllocTracker& copy) : _alloca(copy._alloc_extern?copy._alloca:new _Ax(*copy._alloca)) {}
		inline i_AllocTracker(_Ax* ptr=0) :	_alloca((!ptr)?(new _Ax()):(ptr)), _alloc_extern(ptr!=0) {}
		inline ~i_AllocTracker() { if(!_alloc_extern) delete _alloca; }
    inline pointer _allocate(std::size_t cnt, typename std::allocator<void>::const_pointer p = 0) { return _alloca->allocate(cnt,p); }
    inline void _deallocate(pointer p, std::size_t s = 0) { _alloca->deallocate(p,s); }

		inline i_AllocTracker& operator =(const i_AllocTracker& copy) { if(!_alloc_extern) delete _alloca; _alloca = copy._alloc_extern?copy._alloca:new _Ax(*copy._alloca); return *this; }

	protected:
		_Ax* _alloca;
		bool _alloc_extern;
	};

	template<typename T>
	class i_AllocTracker<T, Allocator<T>>
	{
    typedef typename Allocator<T>::pointer pointer;
	public:
		inline i_AllocTracker(Allocator<T>* ptr=0) {}
    inline pointer _allocate(std::size_t cnt, typename std::allocator<void>::const_pointer=0) { return reinterpret_cast<pointer>(::operator new(cnt * sizeof (T))); }
    inline void _deallocate(pointer p, std::size_t = 0) { ::operator delete(p); }
	};

	template<typename _Ax>
	class cAllocTracker : public i_AllocTracker<typename _Ax::value_type, _Ax>
  {
  public:
		inline cAllocTracker(_Ax* ptr=0) : i_AllocTracker(ptr) {}
  };
}

  //template<class T>
  //struct BSS_COMPILER_DLLEXPORT ALLOCATOR_FUNCPTR
  //{
	 // virtual T* Allocate()=0;
	 // virtual void BSS_FASTCALL Deallocate(T* pointer)=0;
	 // inline virtual ALLOCATOR_FUNCPTR* Clone() const { return 0; }
  //};

  //template<class T>
  //struct BSS_COMPILER_DLLEXPORT ALLOCATOR_FUNCPTR_DEFAULT : ALLOCATOR_FUNCPTR<T>
  //{
	 // inline virtual T* Allocate() { return new T(); }
  //  inline virtual void BSS_FASTCALL Deallocate(T* pointer) { delete pointer; }
	 // inline virtual ALLOCATOR_FUNCPTR_DEFAULT* Clone() const { return new ALLOCATOR_FUNCPTR_DEFAULT(*this); }
  //};

  //template<int size>
  //struct BSS_COMPILER_DLLEXPORT STATICBIN
  //{
  //  STATICBIN() { memset(_mem,0,sizeof(char)*size); _next=0; _numfree=size; }
  //  char _mem[size];
  //  STATICBIN* _next;
  //  unsigned int _numfree;
  //};

  //template<class T, int numitems>
  //class BSS_COMPILER_DLLEXPORT ALLOCATOR_STATICBIN
  //{
  //public:
  //  ALLOCATOR_STATICBIN() : _root(new STATICBIN<sizeof(T)*numitems>()), _base(_root) { }
  //  ~ALLOCATOR_STATICBIN() { Destroy(); }
  //  inline T* Allocate()
  //  {
  //    T* retval=(T*)&_root->_mem[(sizeof(T)*numitems)-_root->_numfree];
  //    _root->_numfree-=sizeof(T);
  //    if(!_root->_numfree)
  //    {
  //      STATICBIN<sizeof(T)*numitems>* hold=_root;
  //      _root = new STATICBIN<sizeof(T)*numitems>();
  //      _root->_next = hold;
  //    }
  //    return retval;
  //  }
  //  inline void Destroy() { STATICBIN<sizeof(T)*numitems>* hold=0; while(_root) { hold=_root->_next; delete _root; _root=hold; } }
  //  //inline void Clear() { STATICBIN<sizeof(T)*numitems>* hold=_root; while(hold) { hold->_numfree=(sizeof(T)*numitems)*sizeof(char); hold=hold->_next; } }

  //protected:
  //  STATICBIN<sizeof(T)*numitems>* _root;
  //  STATICBIN<sizeof(T)*numitems>* _base;
  //};

  //template<class T, int numitems>
  //struct BSS_COMPILER_DLLEXPORT ALLOCATOR_FUNCPTR_STATICBIN : ALLOCATOR_FUNCPTR<T>
  //{
  //  ALLOCATOR_FUNCPTR_STATICBIN(ALLOCATOR_STATICBIN<T,numitems>& staticbin) : _staticbin(staticbin) {}
	 // inline virtual T* Allocate() { return _staticbin.Allocate(); }
  //  inline virtual void BSS_FASTCALL Deallocate(T* pointer) { } //additive
	 // inline virtual ALLOCATOR_FUNCPTR_STATICBIN* Clone() const { return new ALLOCATOR_FUNCPTR_STATICBIN(*this); }

  //protected:
  //  ALLOCATOR_STATICBIN<T,numitems>& _staticbin;
  //};




 // // This allows you to trick the default allocator into accepting a reference to an allocator object
 // template<typename T, typename Policy, typename Traits = ObjectTraits<T>>
	//class BSS_COMPILER_DLLEXPORT AllocatorPolicyRef : public Traits, public AllocPolicySize<T> {
	//private:
 //   typedef Traits TTraits;

	//public: 
 //   typedef typename AllocPolicySize<T>::size_type size_type;
 //   typedef typename AllocPolicySize<T>::pointer pointer;
 //   typedef typename AllocPolicySize<T>::const_pointer const_pointer;
 //   typedef typename AllocPolicySize<T>::reference reference;
 //   typedef typename AllocPolicySize<T>::const_reference const_reference;

 //   template<typename U>
 //   struct rebind {
 //       typedef AllocatorPolicyRef<U, typename AllocPolicySize<T>::rebind<U>::other, 
 //           typename TTraits::rebind<U>::other > other;
 //   };
 //   template<class U>
 //   AllocatorPolicyRef& operator=(const AllocatorPolicyRef<U,Policy>&) { return *this; }

 //   inline explicit AllocatorPolicyRef() {}
 //   inline ~AllocatorPolicyRef() {}
 //   inline AllocatorPolicyRef(AllocatorPolicyRef const& rhs):Traits(rhs) {}
 //   template <typename U, typename P>
 //   inline AllocatorPolicyRef(AllocatorPolicyRef<U, P> const&) {}
 //   template <typename U, typename P, typename T2>
 //   inline AllocatorPolicyRef(AllocatorPolicyRef<U, P, T2> const& rhs):Traits(rhs) {}
	//};

 // template<typename T, typename P, typename Tr>
	//inline bool operator==(AllocatorPolicyRef<T, P, 
	//	 Tr> const& lhs, AllocatorPolicyRef<T, 
	//	 P, Tr> const& rhs) { 
	//		return operator==(static_cast<P const&>(lhs), 
	//											 static_cast<P const&>(rhs)); 
	//}
	//template<typename T, typename P, typename Tr, 
	//				typename T2, typename P2, typename Tr2>
	//inline bool operator==(AllocatorPolicyRef<T, P, 
	//		Tr> const& lhs, AllocatorPolicyRef<T2, P2, Tr2> const& rhs) { 
	//			return operator==(static_cast<P const&>(lhs), 
	//											 static_cast<P2 const&>(rhs)); 
	//}
	//template<typename T, typename P, typename Tr, typename OtherAllocatorPolicyRef>
	//inline bool operator==(AllocatorPolicyRef<T, P, 
	//					Tr> const& lhs, OtherAllocatorPolicyRef const& rhs) { 
	//		return operator==(static_cast<P const&>(lhs), rhs); 
	//}
	//template<typename T, typename P, typename Tr>
	//inline bool operator!=(AllocatorPolicyRef<T, P, Tr> const& lhs, 
	//												 AllocatorPolicyRef<T, P, Tr> const& rhs) { 
	//		return !operator==(lhs, rhs); 
	//}
	//template<typename T, typename P, typename Tr, 
	//					 typename T2, typename P2, typename Tr2>
	//inline bool operator!=(AllocatorPolicyRef<T, P, Tr> const& lhs, 
	//									 AllocatorPolicyRef<T2, P2, Tr2> const& rhs) { 
	//		return !operator==(lhs, rhs); 
	//}
	//template<typename T, typename P, typename Tr, 
	//															typename OtherAllocatorPolicyRef>
	//inline bool operator!=(AllocatorPolicyRef<T, P, 
	//				Tr> const& lhs, OtherAllocatorPolicyRef const& rhs) { 
	//		return !operator==(lhs, rhs); 
	//}


  /*template<typename T, typename _Sz>
  struct FIXED_SIZE_BUCKET_MEM
  {
  public:
    T* _mem;
    _Sz _size;

  protected:
    FIXED_SIZE_BUCKET_MEM(_Sz size) : _mem((T*)malloc(sizeof(T)*size)), _size(size) {}
  };
  template<typename T, typename _Sz>
  struct FIXED_SIZE_BUCKET_SINGLE : FIXED_SIZE_BUCKET_MEM<T,_Sz>
  {
    inline static void BSS_FASTCALL _modnum(_Sz &num) { }

  protected:
    FIXED_SIZE_BUCKET_SINGLE(_Sz size) : FIXED_SIZE_BUCKET_MEM<T,_Sz>(size) {}
    ~FIXED_SIZE_BUCKET_SINGLE() {}
    inline static _Sz BSS_FASTCALL _getlength(T* p) { return 1; }
    inline _Sz BSS_FASTCALL _getoffset(T* p) { return p-_mem; }
    inline T* BSS_FASTCALL _getretval(_Sz offset, _Sz num) const { return _mem+offset; }
  };
  template<typename T, typename _Sz>
  struct FIXED_SIZE_BUCKET_MULTI : FIXED_SIZE_BUCKET_MEM<unsigned char,_Sz>
  {
    inline static void BSS_FASTCALL _modnum(_Sz &num) { num *= sizeof(T); num+=sizeof(_Sz); }

  protected:
    FIXED_SIZE_BUCKET_MULTI(_Sz size) : FIXED_SIZE_BUCKET_MEM<unsigned char,_Sz>(sizeof(T)*size) {}
    ~FIXED_SIZE_BUCKET_MULTI() {}
    inline static _Sz BSS_FASTCALL _getlength(T* p) { return ((_Sz*)p)[-1]; }
    inline _Sz BSS_FASTCALL _getoffset(T* p) { return (((unsigned char*)p)-_mem)-sizeof(_Sz); }
    inline T* BSS_FASTCALL _getretval(_Sz offset, _Sz num) const { unsigned char* retval=_mem+offset; *((_Sz*)retval) = num; return (T*)(retval+=sizeof(_Sz)); }
  };

  template<typename T, class LCLASS, typename _Sz>
  struct FIXED_SIZE_BUCKET : LCLASS
  {
    FIXED_SIZE_BUCKET(_Sz size) : LCLASS(size), _maxfreesecond(0), _maxfreeoffset(0), _next(0)
    { 
      _maxfree=_size;
      _freeslots.Insert(0,_size);
    }
    T* BSS_FASTCALL alloc(_Sz num)
    {
      _Sz slot;
      _Sz offset = _freeslots.Length();
      for(slot = 0; slot < offset; ++slot)
        if(_freeslots[slot]>=num)
          break;
      assert(slot!=offset);
      
      offset = _freeslots.KeyIndex(slot);
      //T* retval = _mem+offset;
      T* retval = LCLASS::_getretval(offset,num);

      _freeslots[slot] -=num;

      bool ismaxfree=_maxfreeoffset==offset;
      if(ismaxfree) //if this is true we have to check for a maxfree violation
      {
        _maxfree=_freeslots[slot];
        if(_maxfree<_maxfreesecond)
        {
          _maxfree=0;
          _maxfreesecond=0; //we do this in case _freeslots is about to be set to a length of 0.

          for(_Sz i = _freeslots.Length()-1; i != ((_Sz)-1); --i)
            if(_maxfreesecond<=_freeslots[i])
            {
              if(_maxfree<=_freeslots[i])
              {
                _maxfreesecond=_maxfree;
                _maxfree=_freeslots[i];
                _maxfreeoffset=_freeslots.KeyIndex(i);
              }
              else
                _maxfreesecond=_freeslots[i];
            }

          ismaxfree=_maxfreeoffset==offset; //we do this because we don't want the below code to try and "update" the offset anymore
        }
      }
      else if(_freeslots[slot]>_maxfreesecond)
        _maxfreesecond=_freeslots[slot];

      if(!_freeslots[slot]) //if this is true we remove the whole thing
        _freeslots.RemoveIndex(slot);
      else if(ismaxfree) 
        _freeslots.ReplaceKey(slot, _maxfreeoffset=offset+num);
      else //otherwise just edit it
        _freeslots.ReplaceKey(slot, offset+=num);
         
      return retval;
    }
    void BSS_FASTCALL dealloc(T* p)
    {
      //_Sz offset = p-_mem;
      _Sz offset=LCLASS::_getoffset(p);
      //_Sz length=1;
      _Sz length=LCLASS::_getlength(p);

      _Sz find = _freeslots.GetNear(offset,true); //we use this to figure out our immediate area. If we're right next to a free slow we don't want to create a new one, and in some cases we actually fill a hole in the free spacee and end up removing one
      _Sz prevoffset;
      _Sz prevlength;

      bool store;
      _Sz slotoffsetlength=_freeslots.Length();
      if(store=find<slotoffsetlength && find>=0) { prevoffset=_freeslots.KeyIndex(find); prevlength = _freeslots[find]; }
      if(store && prevoffset+prevlength==offset) //we are linked to our previous one 
      {
        _Sz nextoffset;
        _Sz nextlength;
        if(store=++find<slotoffsetlength) { nextoffset=_freeslots.KeyIndex(find); nextlength=_freeslots[find]; }
        if(store && nextoffset==offset+length) //in this case we are sandwiched between two free zones and we need to remove one
        {
          _freeslots.RemoveIndex(find);
          _freeslots.Replace(--find, offset=prevoffset, length+=prevlength+nextlength);
        }
        else //here we are only linked to one behind us so we just edit that one
        {
          --find;
          _freeslots.Replace(find, offset=prevoffset,length+=_freeslots[find]);
        }
      }
      else if(++find<slotoffsetlength && (prevoffset=_freeslots.KeyIndex(find)) == offset+length) //in this case we are linked to one in front and only one in front so we just modify that
      {
        _freeslots.Replace(find, offset, length+=_freeslots[find]);
      }
      else //otherwise we are in the middle of nowhere and must add a new free chunk
      {
        _freeslots.Insert(offset, length);
      }
      
      if(length>_maxfree)
      {
        _maxfreesecond=_maxfree;
        _maxfree=length;
        _maxfreeoffset=offset;
      }
      else if(length>_maxfreesecond)
        _maxfreesecond=length;
    }

    _Sz _maxfree;
    _Sz _maxfreeoffset; //offset is used because it doesn't change based on where the key is
    _Sz _maxfreesecond;
    _Sz _index;
    FIXED_SIZE_BUCKET* _next;

  protected:
    cMap<_Sz,_Sz,CompTTraits<_Sz, ValueTraits<_Sz>>, _Sz> _freeslots; //key is offset, data is length
  };

  template<typename T, typename rType, typename _Sz>
  class FSA_BASE
  {
  protected:
    FSA_BASE(_Sz initsize) : _nextsize(!initsize?16:initsize), _root(0) {}
    ~FSA_BASE()
    {
      rType* next;
      while(_root)
      {
        next=_root->_next;
        delete _root;
        _root=next;
      }
    }
    T* BSS_FASTCALL _alloc(_Sz num)
    {
      rType::_modnum(num);
      return _allocbytes(num);
    }
    T* BSS_FASTCALL _allocbytes(_Sz bytes) //This only actually processes bytes if this is a MULTI type. There is no safegaurd to enforce this, however.
    {
      _Sz index;
      while((index=_freesort.GetNear(bytes, false))==((_Sz)-1))
        _addbucket(bytes);

      rType* curalloc = _freesort[index];

      T* retval= curalloc->alloc(bytes);
      ReplaceKeys(curalloc);
      return retval;
    }
    void ReplaceKeys(rType* target)
    {
      if(_freesort.ReplaceKey(target->_index, target->_maxfree)!=target->_index)
      {
        for(_Sz i = _freesort.Length()-1; i != (_Sz)(-1); --i) //We do this backwards because the length cannot change so it saves memory
          _freesort[i]->_index=i;
      }
    }
    bool BSS_FASTCALL _dealloc(T* p)
    {
      rType* curalloc = _root;
      while(curalloc!=0 && ((void*)p < (void*)curalloc->_mem || (void*)p >= (void*)(curalloc->_mem+curalloc->_size)))
      {
        curalloc = curalloc->_next;
      }
      if(!curalloc) return false;
      curalloc->dealloc(p);
      ReplaceKeys(curalloc);
      return true;
    }
    void BSS_FASTCALL _addbucket(_Sz num)
    {
  #ifdef max
  #define REDEFMAX
  #undef max
  #endif
      if(_nextsize>std::numeric_limits<_Sz>::max()>>1) //if this is true if we increment we'll blow past the limit
        _nextsize=std::numeric_limits<_Sz>::max();
      else
        while((_nextsize<<=1)<num)
          if(_nextsize>std::numeric_limits<_Sz>::max()>>1) {
             _nextsize=std::numeric_limits<_Sz>::max();
             break;
          }
  #ifdef REDEFMAX
  #define max(a,b)            (((a) > (b)) ? (a) : (b))
  #endif
      rType* nbucket = new rType(_nextsize);
      nbucket->_next=_root;
      _root=nbucket;
      nbucket->_index = _freesort.Insert(nbucket->_maxfree, nbucket);
    }

    cMap<_Sz, rType*, CompTTraits<_Sz, ValueTraits<_Sz>>, _Sz> _freesort;
    rType* _root;
    _Sz _nextsize;
  };


  // Fixed size chunk allocator
  template<typename T, typename _Sz=unsigned int>
  class BSS_COMPILER_DLLEXPORT cFixedSizeAllocator : protected FSA_BASE<T,FIXED_SIZE_BUCKET<T, FIXED_SIZE_BUCKET_SINGLE<T,_Sz>, _Sz>,_Sz>, protected FSA_BASE<T,FIXED_SIZE_BUCKET<T, FIXED_SIZE_BUCKET_MULTI<T,_Sz>, _Sz>,_Sz>
  {
    typedef FIXED_SIZE_BUCKET<T, FIXED_SIZE_BUCKET_SINGLE<T,_Sz>, _Sz> FSB_SINGLE;
    typedef FIXED_SIZE_BUCKET<T, FIXED_SIZE_BUCKET_MULTI<T,_Sz>, _Sz> FSB_MULTI;
    typedef FSA_BASE<T,FSB_SINGLE,_Sz> FSA_SINGLE;
    typedef FSA_BASE<T,FSB_MULTI,_Sz> FSA_MULTI;

  public:
    cFixedSizeAllocator(_Sz initsize=0) : FSA_SINGLE(initsize), FSA_MULTI(initsize)
    {
    }
    ~cFixedSizeAllocator()
    {
    }

    T* BSS_FASTCALL alloc(_Sz num)
    {
      if(num==1)
        return FSA_SINGLE::_alloc(num);
      return FSA_MULTI::_alloc(num);
    }
    T* BSS_FASTCALL allocbytes(_Sz bytes) //This does a direct bytes allocation and will always use FSA_MULTI
    {
      return FSA_MULTI::_allocbytes(bytes+=4);
    }
    // If this is called we have no idea if its a single or a multiple allocation so we just try both
    bool BSS_FASTCALL dealloc(T* p)
    {
      if(!FSA_SINGLE::_dealloc(p))
        return FSA_MULTI::_dealloc(p);
      return true;
    }
    bool BSS_FASTCALL dealloc_multi(T* p)
    {
      if(!FSA_MULTI::_dealloc(p))
        return FSA_SINGLE::_dealloc(p);
      return true;
    }    
  };*/

#endif