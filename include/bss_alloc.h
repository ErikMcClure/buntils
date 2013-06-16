// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_ALLOC_H__
#define __BSS_ALLOC_H__

#include "bss_defines.h"
#include <limits>
#include <memory>
#include <assert.h>

namespace bss_util {
  // typedefs required by the standard library
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

  // An implementation of a standard allocation policy, conforming to standard library requirements.
	template<typename T>
  class BSS_COMPILER_DLLEXPORT StandardAllocPolicy : public AllocPolicySize<T> {
	public:
    typedef typename AllocPolicySize<T>::pointer pointer;
    template<typename U>
    struct rebind { typedef StandardAllocPolicy<U> other; };

    inline explicit StandardAllocPolicy() {}
    inline ~StandardAllocPolicy() {}
    inline explicit StandardAllocPolicy(StandardAllocPolicy const&) {}
    template <typename U>
    inline explicit StandardAllocPolicy(StandardAllocPolicy<U> const&) {}

    inline pointer allocate(std::size_t cnt, 
      typename std::allocator<void>::const_pointer = 0) { 
        //return reinterpret_cast<pointer>(::operator new(cnt * sizeof (T))); // note that while operator new does not call a constructor (it can't), it's much easier to override for leak tests.
        return reinterpret_cast<pointer>(malloc(cnt*sizeof(T)));
    }
    //inline void deallocate(pointer p, std::size_t = 0) { ::operator delete(p); }
    inline void deallocate(pointer p, std::size_t = 0) { free(p); }
    inline pointer reallocate(pointer p, std::size_t cnt) { return reinterpret_cast<pointer>(realloc(p,cnt*sizeof(T))); }
	};

  // Static implementation of the standard allocation policy, used for cArraySimple
	template<typename T>
  class BSS_COMPILER_DLLEXPORT StaticAllocPolicy : public AllocPolicySize<T> {
	public:
    inline static pointer allocate(std::size_t cnt, 
      typename std::allocator<void>::const_pointer = 0) { 
        return reinterpret_cast<pointer>(malloc(cnt*sizeof(T)));
    }
    inline static void deallocate(pointer p, std::size_t = 0) { free(p); }
    inline static pointer reallocate(pointer p, std::size_t cnt) { return reinterpret_cast<pointer>(realloc(p,cnt*sizeof(T))); }
	};

	template<typename T, typename T2>
	inline bool operator==(StandardAllocPolicy<T> const&, StandardAllocPolicy<T2> const&) { return true; }
	template<typename T, typename OtherAllocator> inline bool operator==(StandardAllocPolicy<T> const&, OtherAllocator const&) { return false; }

  // Object traits as required by the standard library (We ignore them because we do not construct or destroy any objects)
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

  // Standard implementation of an allocator using a policy and object traits that is compatible with standard containers.
	template<typename T, typename Policy = StandardAllocPolicy<T>, typename Traits = ObjectTraits<T>>
#ifndef BSS_DISABLE_CUSTOM_ALLOCATORS
	class BSS_COMPILER_DLLEXPORT Allocator : public Policy {
	private:
    typedef Policy AllocationPolicy;
    typedef Traits TTraits;
#else // if BSS_DISABLE_CUSTOM_ALLOCATORS is defined, we disable all nonstandard policies for debugging.
	class BSS_COMPILER_DLLEXPORT Allocator : public StandardAllocPolicy<T> {
	private:
    typedef StandardAllocPolicy<T> AllocationPolicy;
    typedef ObjectTraits<T> TTraits;
#endif

	public: 
    typedef typename AllocationPolicy::size_type size_type;
    typedef typename AllocationPolicy::pointer pointer;
    typedef typename AllocationPolicy::const_pointer const_pointer;
    typedef typename AllocationPolicy::reference reference;
    typedef typename AllocationPolicy::const_reference const_reference;

    template<typename U>
    struct rebind {
        typedef Allocator<U, typename AllocationPolicy::template rebind<U>::other, 
            typename TTraits::template rebind<U>::other > other;
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

  // Internal class used by cAllocTracker
	template<typename T, typename _Ax>
	class i_AllocTracker
	{
    typedef typename _Ax::pointer pointer;
	public:
		inline i_AllocTracker(const i_AllocTracker& copy) : _allocator(copy._alloc_extern?copy._allocator:new _Ax(*copy._allocator)), _alloc_extern(copy._alloc_extern) {}
    inline i_AllocTracker(i_AllocTracker&& mov) : _allocator(mov._allocator), _alloc_extern(mov._alloc_extern) { mov._alloc_extern=true; }
		inline i_AllocTracker(_Ax* ptr=0) :	_allocator((!ptr)?(new _Ax()):(ptr)), _alloc_extern(ptr!=0) {}
		inline ~i_AllocTracker() { if(!_alloc_extern) delete _allocator; }
    inline pointer _allocate(std::size_t cnt, typename std::allocator<void>::const_pointer p = 0) { return _allocator->allocate(cnt,p); }
    inline void _deallocate(pointer p, std::size_t s = 0) { _allocator->deallocate(p,s); }

		inline i_AllocTracker& operator =(const i_AllocTracker& copy) { if(&copy==this) return *this; if(!_alloc_extern) delete _allocator; _allocator = copy._alloc_extern?copy._allocator:new _Ax(*copy._allocator); _alloc_extern=copy._alloc_extern; return *this; }
		inline i_AllocTracker& operator =(i_AllocTracker&& mov) { if(&mov==this) return *this; if(!_alloc_extern) delete _allocator; _allocator = mov._allocator; _alloc_extern=mov._alloc_extern; mov._alloc_extern=true; return *this; }

	protected:
		_Ax* _allocator;
		bool _alloc_extern;
	};

  // Explicit specialization of cAllocTracker that removes the memory usage for a standard allocation policy, as it isn't needed.
	template<typename T>
	class i_AllocTracker<T, Allocator<T>>
	{
    typedef typename Allocator<T>::pointer pointer;
	public:
		inline i_AllocTracker(Allocator<T>* ptr=0) {}
    //inline pointer _allocate(std::size_t cnt, typename std::allocator<void>::const_pointer=0) { return reinterpret_cast<pointer>(::operator new(cnt * sizeof (T))); } // note that while operator new does not call a constructor (it can't), it's much easier to override for leak tests.
    inline pointer _allocate(std::size_t cnt, typename std::allocator<void>::const_pointer=0) { return reinterpret_cast<pointer>(malloc(cnt*sizeof(T))); }
    //inline void _deallocate(pointer p, std::size_t = 0) { ::operator delete(p); }
    inline void _deallocate(pointer p, std::size_t = 0) { free(p); }
	};

  // This implements stateful allocators.
	template<typename _Ax>
	class cAllocTracker : public i_AllocTracker<typename _Ax::value_type, _Ax>
  {
    typedef i_AllocTracker<typename _Ax::value_type, _Ax> BASE;
  public:
    inline cAllocTracker(const cAllocTracker& copy) : BASE(copy) {}
    inline cAllocTracker(cAllocTracker&& mov) : BASE(std::move(mov)) {}
		inline cAllocTracker(_Ax* ptr=0) : BASE(ptr) {}
  };
}



//  
//  // This takes an allocator and makes a static implementation that can be used in standard containers, but its heavily DLL-dependent.
//	template<typename T, typename Alloc=void>
//  class BSS_COMPILER_DLLEXPORT StaticAllocPolicy : public AllocPolicySize<T> {
//	public:
//    typedef typename AllocPolicySize<T>::pointer pointer;
//    template<typename U>
//    struct rebind { typedef StaticAllocPolicy<U,Alloc> other; };
//
//    inline explicit StaticAllocPolicy() {}
//    inline ~StaticAllocPolicy() {}
//    inline explicit StaticAllocPolicy(StaticAllocPolicy const&) {}
//    template <typename U>
//    inline explicit StaticAllocPolicy(StaticAllocPolicy<U,Alloc> const&) {}
//
//    inline static pointer allocate(std::size_t cnt, typename std::allocator<void>::const_pointer = 0) { return _alloc.allocate(cnt); }
//    inline static void deallocate(pointer p, std::size_t = 0) { _alloc.deallocate(p); }
//
//    static Alloc _alloc;
//	};
//
// template<typename T>
//  class BSS_COMPILER_DLLEXPORT StaticAllocPolicy<T,void> : public AllocPolicySize<T> {
//  public:
//    typedef typename AllocPolicySize<T>::pointer pointer;
//    template<typename U>
//    struct rebind { typedef StaticAllocPolicy<U,void> other; };
//
//    inline explicit StaticAllocPolicy() {}
//    inline ~StaticAllocPolicy() {}
//    inline explicit StaticAllocPolicy(StaticAllocPolicy const&) {}
//    template <typename U>
//    inline explicit StaticAllocPolicy(StaticAllocPolicy<U,void> const&) {}
//
//    inline static pointer allocate(std::size_t cnt, 
//      typename std::allocator<void>::const_pointer = 0) { 
//        //return reinterpret_cast<pointer>(::operator new(cnt * sizeof (T))); // note that while operator new does not call a constructor (it can't), it's much easier to override for leak tests.
//        return reinterpret_cast<pointer>(malloc(cnt*sizeof(T)));
//    }
//    //inline static void deallocate(pointer p, std::size_t = 0) { ::operator delete(p); }
//    inline static void deallocate(pointer p, std::size_t = 0) { free(p); }
//    inline static pointer reallocate(pointer p, std::size_t cnt) { return reinterpret_cast<pointer>(realloc(p,cnt*sizeof(T))); }
//  };
//
//#define BSSBUILD_STATIC_POLICY(name,alloc) template<class T> \
//  class BSS_COMPILER_DLLEXPORT name : public StaticAllocPolicy<T,alloc<T>> \
//  { \
//  public: \
//    template<typename U> \
//    struct rebind { typedef name<U> other; }; \
//    inline explicit name() {} \
//    inline ~name() {} \
//    inline explicit name(name const&) {} \
//    template <typename U> \
//    inline explicit name(name<U> const&) {} \
//  }

#endif
