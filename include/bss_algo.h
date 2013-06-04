// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_ALGO_H__
#define __BSS_ALGO_H__

#include "bss_util.h"
#include "bss_compare.h"
#include "bss_sse.h"
#include <algorithm>
#include <random>

namespace bss_util {
  // Performs a binary search on "arr" between first and last. if CEQ=NEQ and char CVAL=-1, uses an upper bound, otherwise uses lower bound.
  template<typename T, typename D, typename ST_, char (*CFunc)(const D&, const T&), char (*CEQ)(const char&, const char&), char CVAL>
  inline ST_ BSS_FASTCALL binsearch_near(const T* arr, const D& data, ST_ first, ST_ last)
  {
    typename TSignPick<sizeof(ST_)>::SIGNED c = last-first; // Must be a signed version of whatever ST_ is
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

  // inline function wrapper to the #define RANDINTGEN
  inline BSS_FORCEINLINE int bss_randint(int min, int max)
  {
    return !(max-min)?min:RANDINTGEN(min,max);
  }

  /* Shuffler using default random number generator.*/
  template<typename T>
  inline BSS_FORCEINLINE void BSS_FASTCALL shuffle(T* p, int size)
  {
    shuffle<T,int,&bss_randint>(p,size);
  }
  template<typename T, int size>
  inline BSS_FORCEINLINE void BSS_FASTCALL shuffle(T (&p)[size])
  {
    shuffle<T,int,size,&bss_randint>(p);
  }

  template<class F, typename T, size_t SIZE>
  inline BSS_FORCEINLINE void transform(T (&t)[SIZE],T (&result)[SIZE], F func) { std::transform(std::begin(t),std::end(t),result,func); }
  template<class F, typename T, size_t SIZE>
  inline BSS_FORCEINLINE void transform(T (&t)[SIZE], F func) { std::transform(std::begin(t),std::end(t),t,func); }
  template<class F, typename T, size_t SIZE>
  inline BSS_FORCEINLINE void for_each(T (&t)[SIZE], F func) { std::for_each(std::begin(t),std::end(t),func); }
  
  // Gets the squared distance between two n-dimensional points
  template<typename T, int N>
  inline T NVectDistSq(const T (&t1)[N], const T (&t2)[N])
  {
    T tp = t2[0]-t1[0];
    T r = tp*tp;
    for(int i = 1; i < N; ++i)
    {
      tp = t2[i]-t1[i];
      r += tp*tp;
    }
    return r;
  }

  // Gets the distance between two n-dimensional points
  template<typename T, int N>
  inline T NVectDist(const T (&t1)[N], const T (&t2)[N])
  {
    return FastSqrt<T>(NVectDistSq<T,N>(t1,t2));
  }

  // Find the area of an n-dimensional triangle using Heron's formula
  template<typename T, int N>
  inline T NTriangleArea(const T (&x1)[N], const T (&x2)[N], const T (&x3)[N])
  {
    T a = NVectDist(x1,x2);
    T b = NVectDist(x1,x3);
    T c = NVectDist(x2,x3);
    T s = (a+b+c)/((T)2);
    return FastSqrt<T>(s*(s-a)*(s-b)*(s-c));
  }

  // n-dimensional dot product
  template<typename T, int N>
  inline T BSS_FASTCALL NDot(const T (&x1)[N], const T (&x2)[N])
  {
    T r=0.0f;
    for(int i=0; i<N; ++i)
      r+=(x1[i]*x2[i]);
    return r;
  }  
  
  // Applies an operator to two vectors
  template<typename T, int N, T (BSS_FASTCALL *F)(T,T), sseVecT<T> (BSS_FASTCALL *sseF)(const sseVecT<T>&,const sseVecT<T>&)>
  BSS_FORCEINLINE void BSS_FASTCALL NVectOp(const T (&x1)[N], const T (&x2)[N], T (&out)[N])
  {
    assert(((size_t)x1)%16==0);
    assert(((size_t)x2)%16==0);
    assert(((size_t)out)%16==0);
    int low=(N/4)*4; // Gets number of SSE iterations we can do
    int i;
    for(i=0; i < low; i+=4)
      BSS_SSE_STORE_APS(out+i,sseF(sseVecT<T>(BSS_SSE_LOAD_APS(x1+i)),sseVecT<T>(BSS_SSE_LOAD_APS(x2+i))));
    for(;i<N;++i)
      out[i]=F(x1[i],x2[i]);
  }

  // Applies an operator to a vector and a scalar
  template<typename T, int N, T (BSS_FASTCALL *F)(T,T), sseVecT<T> (BSS_FASTCALL *sseF)(const sseVecT<T>&,const sseVecT<T>&)>
  BSS_FORCEINLINE void BSS_FASTCALL NVectOp(const T (&x1)[N], T x2, T (&out)[N])
  {
    assert(((size_t)x1)%16==0);
    assert(((size_t)out)%16==0);
    int low=(N/4)*4; // Gets number of SSE iterations we can do
    int i;
    for(i=0; i < low; i+=4)
      BSS_SSE_STORE_APS(out+i,sseF(sseVecT<T>(BSS_SSE_LOAD_APS(x1+i)),sseVecT<T>(x2)));
    for(;i<N;++i)
      out[i]=F(x1[i],x2);
  }

  template<typename T, typename R> BSS_FORCEINLINE R BSS_FASTCALL NVectFAdd(T a, T b) { return a+b; }
  template<typename T, typename R> BSS_FORCEINLINE R BSS_FASTCALL NVectFSub(T a, T b) { return a-b; }
  template<typename T, typename R> BSS_FORCEINLINE R BSS_FASTCALL NVectFMul(T a, T b) { return a*b; }
  template<typename T, typename R> BSS_FORCEINLINE R BSS_FASTCALL NVectFDiv(T a, T b) { return a/b; }
  template<typename T, int N> BSS_FORCEINLINE void BSS_FASTCALL NVectAdd(const T (&x1)[N], const T (&x2)[N], T (&out)[N]) 
  { return NVectOp<T,N,NVectFAdd<T,T>,NVectFAdd<const sseVecT<T>&,sseVecT<T>>>(x1,x2,out); }
  template<typename T, int N> BSS_FORCEINLINE void BSS_FASTCALL NVectAdd(const T (&x1)[N], T x2, T (&out)[N]) 
  { return NVectOp<T,N,NVectFAdd<T,T>,NVectFAdd<const sseVecT<T>&,sseVecT<T>>>(x1,x2,out); }
  template<typename T, int N> BSS_FORCEINLINE void BSS_FASTCALL NVectSub(const T (&x1)[N], const T (&x2)[N], T (&out)[N]) 
  { return NVectOp<T,N,NVectFSub<T,T>,NVectFSub<const sseVecT<T>&,sseVecT<T>>>(x1,x2,out); }
  template<typename T, int N> BSS_FORCEINLINE void BSS_FASTCALL NVectSub(const T (&x1)[N], T x2, T (&out)[N]) 
  { return NVectOp<T,N,NVectFSub<T,T>,NVectFSub<const sseVecT<T>&,sseVecT<T>>>(x1,x2,out); }
  template<typename T, int N> BSS_FORCEINLINE void BSS_FASTCALL NVectMul(const T (&x1)[N], const T (&x2)[N], T (&out)[N]) 
  { return NVectOp<T,N,NVectFMul<T,T>,NVectFMul<const sseVecT<T>&,sseVecT<T>>>(x1,x2,out); }
  template<typename T, int N> BSS_FORCEINLINE void BSS_FASTCALL NVectMul(const T (&x1)[N], T x2, T (&out)[N]) 
  { return NVectOp<T,N,NVectFMul<T,T>,NVectFMul<const sseVecT<T>&,sseVecT<T>>>(x1,x2,out); }
  template<typename T, int N> BSS_FORCEINLINE void BSS_FASTCALL NVectDiv(const T (&x1)[N], const T (&x2)[N], T (&out)[N]) 
  { return NVectOp<T,N,NVectFDiv<T,T>,NVectFDiv<const sseVecT<T>&,sseVecT<T>>>(x1,x2,out); }
  template<typename T, int N> BSS_FORCEINLINE void BSS_FASTCALL NVectDiv(const T (&x1)[N], T x2, T (&out)[N]) 
  { return NVectOp<T,N,NVectFDiv<T,T>,NVectFDiv<const sseVecT<T>&,sseVecT<T>>>(x1,x2,out); }

  // n-dimensional cross product
  //template<typename T, int N=2>
  //inline void BSS_FASTCALL NCross(const T (&x1)[N], const T (&x2)[N], T (&out)[N])
  //{
  //  for(int i=0; i < N; ++i)
  //    for(int j=0; j < N; ++j)
  //      out[j]+=
  //}

  // Finds the area between two n-dimensional discrete trajectories represented by a series of points
  //template<typename T, int N>
  //inline T CompareTrajectories(T (*t1)[N], int K1, T (*t2)[N], int K2)
  //{
  //  // Check first vertices of both trajectories to find the one that doesn't fall on the other.
  //}
  //template<typename T, int N, int K1, int K2>
  //inline BSS_FORCEINLINE T CompareTrajectories(T t1[K1][N], T t2[K2][N])
  //{
  //  return CompareTrajectories<T,N>(t1,K1,t2,K2);
  //}
}

#endif
