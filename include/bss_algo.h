// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_ALGO_H__
#define __BSS_ALGO_H__

#include "bss_util.h"
#include <algorithm>
#include <random>

namespace bss_util {
  // Performs a binary search on "arr" between first and last. if CEQ=NEQ and char CVAL=-1, uses an upper bound, otherwise uses lower bound.
  template<typename T, typename D, typename ST_, char (*CFunc)(const D&, const T&), char (*CEQ)(const char&, const char&), char CVAL>
  inline ST_ BSS_FASTCALL binsearch_near(const T* arr, const D& data, ST_ first, ST_ last)
  {
    TSignPick<sizeof(ST_)>::SIGNED c = last-first; // Must be a signed version of whatever ST_ is
    ST_ c2; //No possible operation can make this negative so we leave it as possibly unsigned.
    ST_ m;
	  while(c>0)
		{
		  c2 = (c>>1);
      m = first+c2;

		  //if(!(_Val < *_Mid))
      if((*CEQ)((*CFunc)(data,arr[m]),CVAL))
			{	// try top half
			  first = m+1;
			  c -= c2+1;
			}
		  else
			  c = c2;
		}
	  return first;
  }
  // Either gets the element that matches the value in question or one immediately before the closest match. Could return an invalid -1 value.
  template<typename T, typename ST_, char (*CFunc)(const T&, const T&)>
  inline BSS_FORCEINLINE ST_ BSS_FASTCALL binsearch_before(const T* arr, const T& data, ST_ first, ST_ last) { return binsearch_near<T,T,ST_,CFunc,CompT_NEQ<char>,-1>(arr,data,first,last)-1; }

  template<typename T, typename ST_, char (*CFunc)(const T&, const T&)>
  inline BSS_FORCEINLINE ST_ BSS_FASTCALL binsearch_before(const T* arr, ST_ length, const T& data) { return binsearch_near<T,T,ST_,CFunc,CompT_NEQ<char>,-1>(arr,data,0,length)-1; }

  template<typename T, typename ST_, char (*CFunc)(const T&, const T&), ST_ I>
  inline BSS_FORCEINLINE ST_ BSS_FASTCALL binsearch_before(const T (&arr)[I], const T& data) { return binsearch_before<T,ST_,CFunc>(arr,I,data); }

  // Either gets the element that matches the value in question or one immediately after the closest match.
  template<typename T, typename ST_, char (*CFunc)(const T&, const T&)>
  inline BSS_FORCEINLINE ST_ BSS_FASTCALL binsearch_after(const T* arr, const T& data, ST_ first, ST_ last) { return binsearch_near<T,T,ST_,CFunc,CompT_EQ<char>,1>(arr,data,first,last); }

  template<typename T, typename ST_, char (*CFunc)(const T&, const T&)>
  inline BSS_FORCEINLINE ST_ BSS_FASTCALL binsearch_after(const T* arr, ST_ length, const T& data) { return binsearch_near<T,T,ST_,CFunc,CompT_EQ<char>,1>(arr,data,0,length); }

  template<typename T, typename ST_, char (*CFunc)(const T&, const T&), ST_ I>
  inline BSS_FORCEINLINE ST_ BSS_FASTCALL binsearch_after(const T (&arr)[I], const T& data) { return binsearch_after<T,ST_,CFunc>(arr,I,data); }
  
  // Returns index of the item, if it exists, or -1
  template<typename T, typename D, typename ST_, char (*CFunc)(const T&, const D&)>
  inline ST_ BSS_FASTCALL binsearch_exact(const T* arr, const D& data, typename TSignPick<sizeof(ST_)>::SIGNED f, typename TSignPick<sizeof(ST_)>::SIGNED l)
  {
    --l; // Done so l can be an exclusive size parameter even though the algorithm is inclusive.
    ST_ m; // While f and l must be signed ints or the algorithm breaks, m does not.
    char r;
    while(l>=f) // This only works when l is an inclusive max indice
    {
      m=f+((l-f)>>1);

      if((r=(*CFunc)(arr[m],data))<0)
        f=m+1;
      else if(r>0)
        l=m-1;
      else
        return m;
    }
    return (ST_)-1;
  }
  
  template<typename T, typename ST_, char (*CFunc)(const T&, const T&), ST_ I>
  inline BSS_FORCEINLINE ST_ BSS_FASTCALL binsearch_exact(const T (&arr)[I], const T& data) { return binsearch_exact<T,T,ST_,CFunc>(arr,data,0,I); }
  
  // Shuffler using Fisher-Yates/Knuth Shuffle algorithm based on Durstenfeld's implementation.
  // This is an in-place algorithm that works with any data type. Randfunc should be [min,max)
  template<typename T, typename ST, ST (*RandFunc)(ST min, ST max)>
  inline void BSS_FASTCALL shuffle(T* p, ST size)
  {
    for(ST i=size; i>0; --i)
      rswap<T>(p[i-1],p[RandFunc(0,i)]);
  }
  template<typename T, typename ST, ST size, ST (*RandFunc)(ST min, ST max)>
  inline void BSS_FASTCALL shuffle(T (&p)[size]) { shuffle<T,ST,RandFunc>(p,size); }

#ifdef _INC_STDLIB //These shortcuts are only available if you have access to rand() in the first place
  // inline function wrapper to the #define RANDINTGEN
  inline BSS_FORCEINLINE int bss_randfunc(int min, int max)
  {
    return !(max-min)?min:RANDINTGEN(min,max);
  }

  /* Shuffler using default random number generator.*/
  template<typename T>
  inline BSS_FORCEINLINE void BSS_FASTCALL shuffle(T* p, int size)
  {
    shuffle<T,int,&bss_randfunc>(p,size);
  }
  template<typename T, int size>
  inline BSS_FORCEINLINE void BSS_FASTCALL shuffle(T (&p)[size])
  {
    shuffle<T,int,size,&bss_randfunc>(p);
  }
#endif

  template<class F, typename T, size_t SIZE>
  inline BSS_FORCEINLINE void transform(T (&t)[SIZE],T (&result)[SIZE], F func) { std::transform(std::begin(t),std::end(t),result,func); }
  template<class F, typename T, size_t SIZE>
  inline BSS_FORCEINLINE void transform(T (&t)[SIZE], F func) { std::transform(std::begin(t),std::end(t),t,func); }
  template<class F, typename T, size_t SIZE>
  inline BSS_FORCEINLINE void for_each(T (&t)[SIZE], F func) { std::for_each(std::begin(t),std::end(t),func); }

  //template<class F, typename T>
  //inline void for_all(const F& f, size_t size, T* t) { for(size_t i = 0; i<size; ++i) f(t[i]); }

  //template<class F, typename T, size_t SIZE>
  //inline void for_all(F&& f, T (&t)[SIZE]) { for_all<F,T>(f,SIZE,t); }
  //
  //template<class F, typename T, typename T2>
  //inline void for_all(const F& f, size_t size, T* t1,  T2* t2) { for(size_t i = 0; i<size; ++i) f(t1[i],t2[i]); }

  //template<class F, typename T, typename T2, size_t SIZE>
  //inline void for_all(F&& f, T (&t1)[SIZE], T2 (&t2)[SIZE]) { for_all<F,T,T2>(f,SIZE,t1,t2); }

  //template<class F, typename R, typename T>
  //inline void for_all(R* r, const F& f, size_t size, T* t) { for(size_t i = 0; i<size; ++i) r[i]=f(t[i]); }

  //template<class F, typename R, typename T, size_t SIZE>
  //inline void for_all(R (&r)[SIZE], const F& f, T (&t)[SIZE]) { for_all<F,R,T>(r,f,SIZE,t); }
  //
  //template<class F, typename R, typename T, typename T2>
  //inline void for_all(R* r, const F& f, size_t size, T* t1,  T2* t2) { for(size_t i = 0; i<size; ++i) r[i]=f(t1[i],t2[i]); }

  //template<class F, typename R, typename T, typename T2, size_t SIZE>
  //inline void for_all(R (&r)[SIZE], const F& f, T (&t1)[SIZE], T2 (&t2)[SIZE]) { for_all<F,R,T,T2>(r,f,SIZE,t1,t2); }

  //template<class F, typename T>
  //inline void for_all(const F& f, T& t) { auto it_end=std::end(t); for(auto it=std::begin(t); it != it_end; ++it) f(*it); }
  
  //template<class F, typename T, typename T2>
  //inline void for_all(const F& f, T& t1, T2& t2) { 
  //  auto it1 = std::begin(t1); auto it2 = std::begin(t2); auto it1_end = std::end(t1); auto it2_end = std::end(t2);
  //  while(it1 != it1_end && it2 != it2_end)
  //    f(*(it1++),*(it2)++);
  //}

  /*
#ifdef __GNUC__
  template<typename T> // Helper function in case args[i]... isn't valid.
  inline T& i_forall_at(size_t i, T* arr) { return arr[i]; }

  template<class F, typename ...Args>
  inline void for_all(const F& f, size_t size, Args*... args) { for(size_t i = 0; i<size; ++i) std::bind<F,...Args>(f,i_forall_at(i,args)...)(); }

  template<class F, size_t SIZE, typename ...Args>
  inline void for_all(const F& f, Args (&args)[SIZE]...) { for_all(f,SIZE,args...); }

  template<class F, typename R, typename ...Args>
  inline void for_all(R* r, const F& f, size_t size, Args*... args) { for(size_t i = 0; i<size; ++i) r[i]=std::bind<F,...Args>(f,i_forall_at(i,args)...)(); }

  template<class F, typename R, size_t SIZE, typename ...Args>
  inline void for_all(R (&r)[SIZE], const F& f, Args (&args)[SIZE]...) { for_all(r,f,SIZE,args...); }
#endif
  */
   /* if(!_length) return (ST_)(-1);
    ST_ last=_length;
    ST_ first=0;
    ST_ retval=last>>1;
    char compres;
    for(;;) //we do not preform an equality check here because an equality will only occur once.
    {
      if((compres=CFunc(data,_array[retval]))<0)
      {
        last=retval;
        retval=first+((last-first)>>1); //R = F+((L-F)/2)
        if(last==retval)
          return before?--retval:retval; //interestingly, if retval is 0, we end up with -1 which is exactly what we'd want anyway
      }
      else if(compres>0)
      {
        first=retval;
        //retval=first+((last-first)>>1); //R = F+((L-F)/2)
        retval+=(last-first)>>1;
        if(first==retval)
          return before?retval:++retval;
      }
      else //otherwise they are equal
        return retval;
    }
    return retval;*/

}

#endif