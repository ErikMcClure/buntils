// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_ALGO_H__
#define __BSS_ALGO_H__

#include "bss_util.h"
#include "bss_compare.h"
#include "bss_sse.h"
#include "cDynArray.h"
#include <algorithm>
#include <random>
#include <array>

namespace bss_util {
  // Performs a binary search on "arr" between first and last. if CEQ=NEQ and char CVAL=-1, uses an upper bound, otherwise uses lower bound.
  template<typename T, typename D, typename ST_, char (*CFunc)(const D&, const T&), char (*CEQ)(const char&, const char&), char CVAL>
  static inline ST_ BSS_FASTCALL binsearch_near(const T* arr, const D& data, ST_ first, ST_ last)
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
  BSS_FORCEINLINE ST_ BSS_FASTCALL binsearch_before(const T* arr, const T& data, ST_ first, ST_ last) { return binsearch_near<T,T,ST_,CFunc,CompT_NEQ<char>,-1>(arr,data,first,last)-1; }

  template<typename T, typename ST_, char (*CFunc)(const T&, const T&)>
  BSS_FORCEINLINE ST_ BSS_FASTCALL binsearch_before(const T* arr, ST_ length, const T& data) { return binsearch_near<T,T,ST_,CFunc,CompT_NEQ<char>,-1>(arr,data,0,length)-1; }

  template<typename T, typename ST_, char (*CFunc)(const T&, const T&), ST_ I>
  BSS_FORCEINLINE ST_ BSS_FASTCALL binsearch_before(const T (&arr)[I], const T& data) { return binsearch_before<T,ST_,CFunc>(arr,I,data); }

  // Either gets the element that matches the value in question or one immediately after the closest match.
  template<typename T, typename ST_, char (*CFunc)(const T&, const T&)>
  BSS_FORCEINLINE ST_ BSS_FASTCALL binsearch_after(const T* arr, const T& data, ST_ first, ST_ last) { return binsearch_near<T,T,ST_,CFunc,CompT_EQ<char>,1>(arr,data,first,last); }

  template<typename T, typename ST_, char (*CFunc)(const T&, const T&)>
  BSS_FORCEINLINE ST_ BSS_FASTCALL binsearch_after(const T* arr, ST_ length, const T& data) { return binsearch_near<T,T,ST_,CFunc,CompT_EQ<char>,1>(arr,data,0,length); }

  template<typename T, typename ST_, char (*CFunc)(const T&, const T&), ST_ I>
  BSS_FORCEINLINE ST_ BSS_FASTCALL binsearch_after(const T (&arr)[I], const T& data) { return binsearch_after<T,ST_,CFunc>(arr,I,data); }
  
  // Returns index of the item, if it exists, or -1
  template<typename T, typename D, typename ST_, char (*CFunc)(const T&, const D&)>
  static inline ST_ BSS_FASTCALL binsearch_exact(const T* arr, const D& data, typename TSignPick<sizeof(ST_)>::SIGNED f, typename TSignPick<sizeof(ST_)>::SIGNED l)
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
  BSS_FORCEINLINE ST_ BSS_FASTCALL binsearch_exact(const T (&arr)[I], const T& data) { return binsearch_exact<T,T,ST_,CFunc>(arr,data,0,I); }
  
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
  template<class T>
  BSS_FORCEINLINE T bss_randint(T min, T max)
  {
    static_assert(std::is_integral<T>::value,"T must be integral");
    return !(max-min)?min:((min)+(rand()%((T)((max)-(min)))));
  }

  /* Shuffler using default random number generator.*/
  template<typename T>
  BSS_FORCEINLINE void BSS_FASTCALL shuffle(T* p, int size)
  {
    shuffle<T,int,&bss_randint<int>>(p,size);
  }
  template<typename T, int size>
  BSS_FORCEINLINE void BSS_FASTCALL shuffle(T (&p)[size])
  {
    shuffle<T,int,size,&bss_randint<int>>(p);
  }

  template<class F, typename T, size_t SIZE>
  BSS_FORCEINLINE void transform(T (&t)[SIZE],T (&result)[SIZE], F func) { std::transform(std::begin(t),std::end(t),result,func); }
  template<class F, typename T, size_t SIZE>
  BSS_FORCEINLINE void transform(T (&t)[SIZE], F func) { std::transform(std::begin(t),std::end(t),t,func); }
  template<class F, typename T, size_t SIZE>
  BSS_FORCEINLINE void for_each(T (&t)[SIZE], F func) { std::for_each(std::begin(t),std::end(t),func); }
  
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
  //BSS_FORCEINLINE T CompareTrajectories(T t1[K1][N], T t2[K2][N])
  //{
  //  return CompareTrajectories<T,N>(t1,K1,t2,K2);
  //}

  // standard 4x4 matrix multiplication using SSE2
  BSS_FORCEINLINE void BSS_FASTCALL Mult4x4(float (&out)[4][4], const float (&l)[4][4], const float (&r)[4][4])
  {
    sseVec a(r[0]);
    sseVec b(r[1]);
    sseVec c(r[2]);
    sseVec d(r[3]);

    (a*sseVec(l[0][0]))+(b*sseVec(l[0][1]))+(c*sseVec(l[0][2]))+(d*sseVec(l[0][3])) >> out[0];
    (a*sseVec(l[1][0]))+(b*sseVec(l[1][1]))+(c*sseVec(l[1][2]))+(d*sseVec(l[1][3])) >> out[1];
    (a*sseVec(l[2][0]))+(b*sseVec(l[2][1]))+(c*sseVec(l[2][2]))+(d*sseVec(l[2][3])) >> out[2];
    (a*sseVec(l[3][0]))+(b*sseVec(l[3][1]))+(c*sseVec(l[3][2]))+(d*sseVec(l[3][3])) >> out[3];
  }

  // Random queue 
  template<class ArrayType, typename ArrayType::ST_ (*RandFunc)(typename ArrayType::ST_ min, typename ArrayType::ST_ max)=&bss_randint>
  class BSS_COMPILER_DLLEXPORT cRandomQueue : protected cDynArray<ArrayType>
  {
  protected:
    typedef typename ArrayType::ST_ ST_;
    typedef typename ArrayType::T_ T_;
    using ArrayType::_array;
    using cDynArray<ArrayType>::_length;

  public:
    cRandomQueue(const cRandomQueue& copy) : cDynArray<ArrayType>(copy) {}
    cRandomQueue(cRandomQueue&& mov) : cDynArray<ArrayType>(std::move(mov)) {}
    explicit cRandomQueue(ST_ size=0): cDynArray<ArrayType>(size) {}
    inline void Push(const T_& t) { Add(t); }
    inline void Push(T_&& t) { Add(std::move(t)); }
    inline T_ Pop() { ST_ i=RandFunc(0,_length); T_ r = std::move(_array[i]); Remove(i); return r; }
    inline void Remove(ST_ index) { _array[index]=std::move(_array[--_length]); }
    inline bool Empty() const { return !_length; }
    inline void Clear() { _length=0; }
    inline void SetLength(ST_ length) { cDynArray<ArrayType>::SetLength(length); }
    inline ST_ Length() const { return _length; }
    inline const T_* begin() const { return _array; }
    inline const T_* end() const { return _array+_length; }
    inline T_* begin() { return _array; }
    inline T_* end() { return _array+_length; }

    inline operator T_*() { return _array; }
    inline operator const T_*() const { return _array; }
    inline cRandomQueue& operator=(const cRandomQueue& copy) { cDynArray<ArrayType>::operator=(copy); return *this; }
    inline cRandomQueue& operator=(cRandomQueue&& mov) { cDynArray<ArrayType>::operator=(std::move(mov)); return *this; }
    inline cRandomQueue& operator +=(const cRandomQueue& add) { cDynArray<ArrayType>::operator+=(add); return *this; }
    inline const cRandomQueue operator +(const cRandomQueue& add) const { cRandomQueue r(*this); return (r+=add); }
  };

  static const double ZIGNOR_R = 3.442619855899;

  // Instance for generating random samples from a normal distribution.
  template<int ZIGNOR_C=128,typename T=double>
  struct NormalZig
  {
    T s_adZigX[ZIGNOR_C + 1], s_adZigR[ZIGNOR_C];

    NormalZig(int iC=ZIGNOR_C, T dV=9.91256303526217e-3, T dR=ZIGNOR_R) // (R * phi(R) + Pr(X>=R)) * sqrt(2\pi)
    {
      int i; T f;
      f = exp(T(-0.5) * (dR * dR));
      s_adZigX[0] = dV / f; /* [0] is bottom block: V / f(R) */
      s_adZigX[1] = dR;
      s_adZigX[iC] = 0;
      for (i = 2; i < iC; ++i)
      {
        s_adZigX[i] = sqrt(-2 * log(dV / s_adZigX[i - 1] + f));
        f = exp(T(-0.5) * (s_adZigX[i] * s_adZigX[i]));
      }
      for (i = 0; i < iC; ++i)
        s_adZigR[i] = s_adZigX[i + 1] / s_adZigX[i];
    }

    static T DRanNormalTail(T dMin, int iNegative)
    {
      T x, y;
      do
      { 
        x = log(RANDFLOATGEN(0,1.0)) / dMin;
        y = log(RANDFLOATGEN(0,1.0));
      } while (-2 * y < x * x);
      return iNegative ? x - dMin : dMin - x;
    }

    // Returns a random normally distributed number using the ziggurat method
    T Get() const
    {
      unsigned int i;
      T x, u, f0, f1;
      for (;;)
      {
        u = 2 * RANDFLOATGEN(0,1.0) - 1;
        i = ((unsigned int)rand()) & 0x7F;
        /* first try the rectangular boxes */
        if (fabs(u) < s_adZigR[i])
          return u * s_adZigX[i];
        /* bottom box: sample from the tail */
        if (i == 0)
          return DRanNormalTail(ZIGNOR_R, u < 0);
        /* is this a sample from the wedges? */
        x = u * s_adZigX[i];
        f0 = exp(T(-0.5) * (s_adZigX[i] * s_adZigX[i] - x * x) );
        f1 = exp(T(-0.5) * (s_adZigX[i+1] * s_adZigX[i+1] - x * x) );
        if (f1 + RANDFLOATGEN(0,1.0) * (f0 - f1) < 1.0)
          return x;
      }
    }

    T operator()() const { return Get(); }
  };

  // Randomly subdivides a rectangular area into smaller rects of varying size. F1 takes (depth,rect) and returns how likely it is that a branch will terminate.
  template<typename T, typename F1, typename F2, typename F3> // F2 takes (const float (&rect)[4]) and is called when a branch terminates on a rect.
  void StochasticSubdivider(const T (&rect)[4], const F1& f1, const F2& f2, const F3& f3, unsigned int depth=0) // F3 returns a random number from [0,1]
  {
    if(RANDFLOATGEN(0,1.0)<f1(depth,rect))
    {
      f2(rect);
      return;
    }
    unsigned char axis=depth%2;
    T div=lerp(rect[axis],rect[2+axis],f3(depth,rect));
    T r1[4] = { rect[0],rect[1],rect[2],rect[3] };
    T r2[4] = { rect[0],rect[1],rect[2],rect[3] };
    r1[axis]=div;
    r2[2+axis]=div;
    StochasticSubdivider(r1,f1,f2,f3,++depth);
    StochasticSubdivider(r2,f1,f2,f3,depth);
  }

  template<typename T>
  BSS_FORCEINLINE size_t BSS_FASTCALL _PDS_imageToGrid(const std::array<T,2>& pt, T cell, size_t gw, T (&rect)[4])
  {
    return (size_t)((pt[0]-rect[0]) / cell) + gw*(size_t)((pt[1]-rect[1]) / cell) + 2 + gw + gw;
  }

  // Implementation of Fast Poisson Disk Sampling by Robert Bridson
  template<typename T, typename F>
  void PoissonDiskSample(T (&rect)[4], T mindist, F && f, uint pointsPerIteration=30)
  {
    //Create the grid
    T cell = mindist/(T)SQRT_TWO;
    T w = rect[2]-rect[0];
    T h = rect[3]-rect[1];
    size_t gw = ceil(w/cell)+4; //gives us buffer room so we don't have to worry about going outside the grid
    size_t gh = ceil(h/cell)+4;
    std::array<T,2>* grid = new std::array<T,2>[gw*gh];      //grid height
    uint64* ig = (uint64*)grid;
    memset(grid,0xFFFFFFFF,gw*gh*sizeof(std::array<T,2>));
    assert(!(~ig[0]));

    cRandomQueue<cArraySimple<std::array<T,2>>> list;
    std::array<T,2> pt = { (T)RANDFLOATGEN(rect[0],rect[2]), (T)RANDFLOATGEN(rect[1],rect[3]) };

    //update containers 
    list.Push(pt);
    f(pt.data());
    grid[_PDS_imageToGrid<T>(pt, cell, gw, rect)] = pt;

    T mindistsq = mindist*mindist;
    T radius,angle;
    size_t center,edge;
    //generate other points from points in queue.
    while(!list.Empty())
    {
      auto point = list.Pop();
      for(uint i = 0; i < pointsPerIteration; i++)
      {
        radius = mindist*RANDFLOATGEN(1,2); //random point between mindist and 2*mindist
        angle = RANDFLOATGEN(0,PI_DOUBLE);
        pt[0] = point[0] + radius * cos(angle); //the new point is generated around the point (x, y)
        pt[1] = point[1] + radius * sin(angle);
        
        if(pt[0]>rect[0] && pt[0]<rect[2] && pt[1]>rect[1] && pt[1]<rect[3]) //Ensure point is inside recT
        {
          center = _PDS_imageToGrid<T>(pt, cell, gw, rect); // If another point is in the neighborhood, abort this point.
          edge=center-gw-gw;
          assert(edge>0);
#define POISSONSAMPLE_CHECK(edge) if((~ig[edge])!=0 && distsqr(grid[edge][0],grid[edge][1],pt[0],pt[1])<mindistsq) continue
          POISSONSAMPLE_CHECK(edge-1);
          POISSONSAMPLE_CHECK(edge);
          POISSONSAMPLE_CHECK(edge+1);
          edge+=gw;
          if(~(ig[edge-1]&ig[edge]&ig[edge+1])) continue;
          POISSONSAMPLE_CHECK(edge-2);
          POISSONSAMPLE_CHECK(edge+2);
          edge+=gw;
          if(~(ig[edge-1]&ig[edge]&ig[edge+1])) continue;
          POISSONSAMPLE_CHECK(edge-2);
          POISSONSAMPLE_CHECK(edge+2);
          edge+=gw;
          if(~(ig[edge-1]&ig[edge]&ig[edge+1])) continue;
          POISSONSAMPLE_CHECK(edge-2);
          POISSONSAMPLE_CHECK(edge+2);
          edge+=gw;
          POISSONSAMPLE_CHECK(edge-1);
          POISSONSAMPLE_CHECK(edge);
          POISSONSAMPLE_CHECK(edge+1);
          list.Push(pt);
          f(pt.data());
          grid[center] = pt;
        }
      }
    }
  }

  // Implementation of a uniform quadratic B-spline interpolation
  template<typename T, typename D>
  static inline T BSS_FASTCALL UniformQuadraticBSpline(D t, const T& prev, const T& cur, const T& next)
  {
    D t2=t*t;
    return (prev*(1 - 2*t + t2) + cur*(1 + 2*t - 2*t2) + next*t2)*((D)0.5);
  }

  // Implementation of a uniform cubic B-spline interpolation. A uniform cubic B-spline matrix is:
  //                / -1  3 -3  1 \       / p1 \
  // [t^3,t²,t,1] * |  3 -6  3  0 | * ½ * | p2 |
  //                | -3  0  3  0 |       | p3 |
  //                \  1  4  1  0 /       \ p4 /
  template<typename T, typename D>
  static inline T BSS_FASTCALL UniformCubicBSpline(D t, const T& p1, const T& p2, const T& p3, const T& p4)
  {
    D t2=t*t;
    D t3=t2*t;
    return (p1*(1-3*t+3*t2-t3) + p2*(4-6*t2+3*t3)+p3*(1+3*t+3*t2-3*t3)+(p4*t3))/((D)6.0);
  }

  // Implementation of a basic cubic interpolation. The B-spline matrix for this is
  //                / -1  3 -3  1 \       / p1 \
  // [t^3,t²,t,1] * |  2 -5  4 -1 | * ½ * | p2 |
  //                | -1  0  1  0 |       | p3 |
  //                \  0  2  0  0 /       \ p4 /
  template<typename T, typename D>
  static inline T BSS_FASTCALL CubicBSpline(D t, const T& p1, const T& p2, const T& p3, const T& p4)
  {
    D t2=t*t;
    D t3=t2*t;
    return (p1*(-t3+2*t2-t)+p2*(3*t3-5*t2+2)+p3*(-3*t3+4*t2+t)+p4*(t3-t2))/((D)2.0);
  }
}


#endif
