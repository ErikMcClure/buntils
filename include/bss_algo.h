// Copyright ©2014 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_ALGO_H__
#define __BSS_ALGO_H__

#include "bss_util.h"
#include "bss_compare.h"
#include "bss_sse.h"
#include "bss_graph.h"
#include "bss_vector.h"
#include "cDynArray.h"
#include "cDisjointSet.h"
#include <algorithm>
#include <random>
#include <array>
#ifdef BSS_COMPILER_GCC
#include <alloca.h>
#endif

namespace bss_util {
  // Performs a binary search on "arr" between first and last. if CEQ=NEQ and char CVAL=-1, uses an upper bound, otherwise uses lower bound.
  template<typename T, typename D, typename ST_, char(*CFunc)(const D&, const T&), char(*CEQ)(const char&, const char&), char CVAL>
  inline static ST_ BSS_FASTCALL binsearch_near(const T* arr, const D& data, ST_ first, ST_ last)
  {
    typename std::make_signed<ST_>::type c = last-first; // Must be a signed version of whatever ST_ is
    ST_ c2; //No possible operation can make this negative so we leave it as possibly unsigned.
    ST_ m;
    while(c>0)
    {
      c2 = (c>>1);
      m = first+c2;

      //if(!(_Val < *_Mid))
      if((*CEQ)((*CFunc)(data, arr[m]), CVAL))
      {	// try top half
        first = m+1;
        c -= c2+1;
      } else
        c = c2;
    }
    return first;
  }
  // Either gets the element that matches the value in question or one immediately before the closest match. Could return an invalid -1 value.
  template<typename T, typename ST_, char(*CFunc)(const T&, const T&)>
  BSS_FORCEINLINE static ST_ BSS_FASTCALL binsearch_before(const T* arr, const T& data, ST_ first, ST_ last) { return binsearch_near<T, T, ST_, CFunc, CompT_NEQ<char>, -1>(arr, data, first, last)-1; }

  template<typename T, typename ST_, char(*CFunc)(const T&, const T&)>
  BSS_FORCEINLINE static ST_ BSS_FASTCALL binsearch_before(const T* arr, ST_ length, const T& data) { return binsearch_near<T, T, ST_, CFunc, CompT_NEQ<char>, -1>(arr, data, 0, length)-1; }

  template<typename T, typename ST_, char(*CFunc)(const T&, const T&), ST_ I>
  BSS_FORCEINLINE static ST_ BSS_FASTCALL binsearch_before(const T(&arr)[I], const T& data) { return binsearch_before<T, ST_, CFunc>(arr, I, data); }

  // Either gets the element that matches the value in question or one immediately after the closest match.
  template<typename T, typename ST_, char(*CFunc)(const T&, const T&)>
  BSS_FORCEINLINE static ST_ BSS_FASTCALL binsearch_after(const T* arr, const T& data, ST_ first, ST_ last) { return binsearch_near<T, T, ST_, CFunc, CompT_EQ<char>, 1>(arr, data, first, last); }

  template<typename T, typename ST_, char(*CFunc)(const T&, const T&)>
  BSS_FORCEINLINE static ST_ BSS_FASTCALL binsearch_after(const T* arr, ST_ length, const T& data) { return binsearch_near<T, T, ST_, CFunc, CompT_EQ<char>, 1>(arr, data, 0, length); }

  template<typename T, typename ST_, char(*CFunc)(const T&, const T&), ST_ I>
  BSS_FORCEINLINE static ST_ BSS_FASTCALL binsearch_after(const T(&arr)[I], const T& data) { return binsearch_after<T, ST_, CFunc>(arr, I, data); }

  // Returns index of the item, if it exists, or -1
  template<typename T, typename D, typename ST_, char(*CFunc)(const T&, const D&)>
  inline static ST_ BSS_FASTCALL binsearch_exact(const T* arr, const D& data, typename std::make_signed<ST_>::type f, typename std::make_signed<ST_>::type l)
  {
    --l; // Done so l can be an exclusive size parameter even though the algorithm is inclusive.
    ST_ m; // While f and l must be signed ints or the algorithm breaks, m does not.
    char r;
    while(l>=f) // This only works when l is an inclusive max indice
    {
      m=f+((l-f)>>1); // Done to avoid overflow on large numbers

      if((r=(*CFunc)(arr[m], data))<0) // This is faster than a switch statement
        f=m+1;
      else if(r>0)
        l=m-1;
      else
        return m;
    }
    return (ST_)-1;
  }

  template<typename T, typename ST_, char(*CFunc)(const T&, const T&), ST_ I>
  BSS_FORCEINLINE static ST_ BSS_FASTCALL binsearch_exact(const T(&arr)[I], const T& data) { return binsearch_exact<T, T, ST_, CFunc>(arr, data, 0, I); }

  inline static unsigned __int64 mersennerand64(unsigned __int64 seed=0) {
    static std::mt19937_64 m;
#ifdef BSS_COMPILER_MSC
    if(seed) m.seed((unsigned long)seed); // VC++ doesn't implement the seed function properly, forcing the value to a ULONG, which is a flagrant violation of the standard.
#else
    if(seed) m.seed(seed);
#endif
    return m();
  }
  inline static unsigned int mersennerand(unsigned int seed=0) {
    static std::mt19937 m;
    if(seed) m.seed((unsigned long)seed);
    return m();
  }

  template<class T>
  BSS_FORCEINLINE T bss_randmersenne(T min, T max)
  {
#ifdef BSS_COMPILER_MSC
    static const unsigned __int64 WMSK = ~((~(unsigned __int64)(0) << (64 - 1)) << 1); // Because Microsoft is full of illiterate morons, despite the standard defining max() as a static function, it's not in VC++, which means we can't use it.
#else
    static const unsigned __int64 WMSK = std::mt19937_64::max();
#endif
    return min+(T)((mersennerand64()/(WMSK + 1.0))*(max-min));
  }

  // inline function wrapper around RANDINTGEN
  template<class T>
  BSS_FORCEINLINE T bss_randint(T min, T max)
  {
    static_assert(std::is_integral<T>::value, "T must be integral");
    return !(max-min)?min:RANDINTGEN(min, max);
  }

  // Shuffler using Fisher-Yates/Knuth Shuffle algorithm based on Durstenfeld's implementation.
  // This is an in-place algorithm that works with any data type. Randfunc should be [min,max)
  template<typename T, typename ST, ST(*RandFunc)(ST min, ST max)>
  inline static void BSS_FASTCALL shuffle(T* p, ST size)
  {
    for(ST i=size; i>0; --i)
      rswap<T>(p[i-1], p[RandFunc(0, i)]);
  }
  template<typename T, typename ST, ST size, ST(*RandFunc)(ST min, ST max)>
  inline static void BSS_FASTCALL shuffle(T(&p)[size]) { shuffle<T, ST, RandFunc>(p, size); }

  /* Shuffler using default random number generator.*/
  template<typename T>
  BSS_FORCEINLINE static void BSS_FASTCALL shuffle(T* p, int size)
  {
    shuffle<T, int, &bss_randint<int>>(p, size);
  }
  template<typename T, int size>
  BSS_FORCEINLINE static void BSS_FASTCALL shuffle(T(&p)[size])
  {
    shuffle<T, int, size, &bss_randint<int>>(p);
  }

  template<class F, typename T, size_t SIZE>
  BSS_FORCEINLINE static void transform(T(&t)[SIZE], T(&result)[SIZE], F func) { std::transform(std::begin(t), std::end(t), result, func); }
  template<class F, typename T, size_t SIZE>
  BSS_FORCEINLINE static void transform(T(&t)[SIZE], F func) { std::transform(std::begin(t), std::end(t), t, func); }
  template<class F, typename T, size_t SIZE>
  BSS_FORCEINLINE static void for_each(T(&t)[SIZE], F func) { std::for_each(std::begin(t), std::end(t), func); }

  // Random queue 
  template<typename T, typename SizeType = unsigned int, SizeType(*RandFunc)(SizeType min, SizeType max)=&bss_randint, ARRAY_TYPE ArrayType = CARRAY_SIMPLE, typename Alloc = StaticAllocPolicy<T>>
  class BSS_COMPILER_DLLEXPORT cRandomQueue : protected cDynArray<T, SizeType, ArrayType, Alloc>
  {
  protected:
    typedef typename SizeType ST_;
    typedef cDynArray<T, SizeType, ArrayType, Alloc> AT_;
    using AT_::_array;
    using AT_::_length;

  public:
    cRandomQueue(const cRandomQueue& copy) : AT_(copy) {}
    cRandomQueue(cRandomQueue&& mov) : AT_(std::move(mov)) {}
    explicit cRandomQueue(ST_ size=0) : AT_(size) {}
    inline void Push(const T& t) { AT_::Add(t); }
    inline void Push(T&& t) { AT_::Add(std::move(t)); }
    inline T Pop() { ST_ i=RandFunc(0, _length); T r = std::move(_array[i]); Remove(i); return r; }
    inline void Remove(ST_ index) { _array[index]=std::move(_array[--_length]); }
    inline bool Empty() const { return !_length; }
    inline void Clear() { _length=0; }
    inline void SetLength(ST_ length) { AT_::SetLength(length); }
    inline ST_ Length() const { return _length; }
    inline const T* begin() const { return _array; }
    inline const T* end() const { return _array+_length; }
    inline T* begin() { return _array; }
    inline T* end() { return _array+_length; }

    inline operator T*() { return _array; }
    inline operator const T*() const { return _array; }
    inline cRandomQueue& operator=(const cRandomQueue& copy) { AT_::operator=(copy); return *this; }
    inline cRandomQueue& operator=(cRandomQueue&& mov) { AT_::operator=(std::move(mov)); return *this; }
    inline cRandomQueue& operator +=(const cRandomQueue& add) { AT_::operator+=(add); return *this; }
    inline const cRandomQueue operator +(const cRandomQueue& add) const { cRandomQueue r(*this); return (r+=add); }
  };

  static const double ZIGNOR_R = 3.442619855899;

  // Instance for generating random samples from a normal distribution.
  template<int ZIGNOR_C=128, typename T=double>
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
      for(i = 2; i < iC; ++i)
      {
        s_adZigX[i] = sqrt(-2 * log(dV / s_adZigX[i - 1] + f));
        f = exp(T(-0.5) * (s_adZigX[i] * s_adZigX[i]));
      }
      for(i = 0; i < iC; ++i)
        s_adZigR[i] = s_adZigX[i + 1] / s_adZigX[i];
    }

    static T DRanNormalTail(T dMin, int iNegative)
    {
      T x, y;
      do
      {
        x = log(RANDFLOATGEN(0, 1.0)) / dMin;
        y = log(RANDFLOATGEN(0, 1.0));
      } while(-2 * y < x * x);
      return iNegative ? x - dMin : dMin - x;
    }

    // Returns a random normally distributed number using the ziggurat method
    T Get() const
    {
      unsigned int i;
      T x, u, f0, f1;
      for(;;)
      {
        u = 2 * RANDFLOATGEN(0, 1.0) - 1;
        i = ((unsigned int)rand()) & 0x7F;
        /* first try the rectangular boxes */
        if(fabs(u) < s_adZigR[i])
          return u * s_adZigX[i];
        /* bottom box: sample from the tail */
        if(i == 0)
          return DRanNormalTail(ZIGNOR_R, u < 0);
        /* is this a sample from the wedges? */
        x = u * s_adZigX[i];
        f0 = exp(T(-0.5) * (s_adZigX[i] * s_adZigX[i] - x * x));
        f1 = exp(T(-0.5) * (s_adZigX[i+1] * s_adZigX[i+1] - x * x));
        if(f1 + RANDFLOATGEN(0, 1.0) * (f0 - f1) < 1.0)
          return x;
      }
    }

    T operator()() const { return Get(); }
  };

  // Randomly subdivides a rectangular area into smaller rects of varying size. F1 takes (depth,rect) and returns how likely it is that a branch will terminate.
  template<typename T, typename F1, typename F2, typename F3> // F2 takes (const float (&rect)[4]) and is called when a branch terminates on a rect.
  static void StochasticSubdivider(const T(&rect)[4], const F1& f1, const F2& f2, const F3& f3, unsigned int depth=0) // F3 returns a random number from [0,1]
  {
    if(RANDFLOATGEN(0, 1.0)<f1(depth, rect))
    {
      f2(rect);
      return;
    }
    unsigned char axis=depth%2;
    T div=lerp(rect[axis], rect[2+axis], f3(depth, rect));
    T r1[4] ={ rect[0], rect[1], rect[2], rect[3] };
    T r2[4] ={ rect[0], rect[1], rect[2], rect[3] };
    r1[axis]=div;
    r2[2+axis]=div;
    StochasticSubdivider(r1, f1, f2, f3, ++depth);
    StochasticSubdivider(r2, f1, f2, f3, depth);
  }

  template<typename T>
  BSS_FORCEINLINE static size_t BSS_FASTCALL _PDS_imageToGrid(const std::array<T, 2>& pt, T cell, size_t gw, T(&rect)[4])
  {
    return (size_t)((pt[0]-rect[0]) / cell) + gw*(size_t)((pt[1]-rect[1]) / cell) + 2 + gw + gw;
  }

  // Implementation of Fast Poisson Disk Sampling by Robert Bridson
  template<typename T, typename F>
  static void PoissonDiskSample(T(&rect)[4], T mindist, F && f, uint pointsPerIteration=30)
  {
    //Create the grid
    T cell = mindist/(T)SQRT_TWO;
    T w = rect[2]-rect[0];
    T h = rect[3]-rect[1];
    size_t gw = ((size_t)ceil(w/cell))+4; //gives us buffer room so we don't have to worry about going outside the grid
    size_t gh = ((size_t)ceil(h/cell))+4;
    std::array<T, 2>* grid = new std::array<T, 2>[gw*gh];      //grid height
    uint64* ig = (uint64*)grid;
    memset(grid, 0xFFFFFFFF, gw*gh*sizeof(std::array<T, 2>));
    assert(!(~ig[0]));

    cRandomQueue<std::array<T, 2>> list;
    std::array<T, 2> pt ={ (T)RANDFLOATGEN(rect[0], rect[2]), (T)RANDFLOATGEN(rect[1], rect[3]) };

    //update containers 
    list.Push(pt);
    f(pt.data());
    grid[_PDS_imageToGrid<T>(pt, cell, gw, rect)] = pt;

    T mindistsq = mindist*mindist;
    T radius, angle;
    size_t center, edge;
    //generate other points from points in queue.
    while(!list.Empty())
    {
      auto point = list.Pop();
      for(uint i = 0; i < pointsPerIteration; i++)
      {
        radius = mindist*((T)RANDFLOATGEN(1, 2)); //random point between mindist and 2*mindist
        angle = (T)RANDFLOATGEN(0, PI_DOUBLE);
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
  inline static T BSS_FASTCALL UniformQuadraticBSpline(D t, const T& prev, const T& cur, const T& next)
  {
    D t2=t*t;
    return (prev*(1 - 2*t + t2) + cur*(1 + 2*t - 2*t2) + next*t2)/((D)2.0);
  }

  // Implementation of a uniform cubic B-spline interpolation. A uniform cubic B-spline matrix is:
  //                / -1  3 -3  1 \         / p1 \
    // [t^3,t²,t,1] * |  3 -6  3  0 | * 1/6 * | p2 |
  //                | -3  0  3  0 |         | p3 |
  //                \  1  4  1  0 /         \ p4 /
  template<typename T, typename D>
  inline static T BSS_FASTCALL UniformCubicBSpline(D t, const T& p1, const T& p2, const T& p3, const T& p4)
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
  inline static T BSS_FASTCALL CubicBSpline(D t, const T& p1, const T& p2, const T& p3, const T& p4)
  {
    D t2=t*t;
    D t3=t2*t;
    return (p1*(-t3+2*t2-t)+p2*(3*t3-5*t2+2)+p3*(-3*t3+4*t2+t)+p4*(t3-t2))/((D)2.0);
  }

  // This implements all possible B-spline functions, but does it statically without optimizations (so it can be used with any type)
  template<typename T, typename D, const T(&m)[4][4]>
  BSS_FORCEINLINE static T BSS_FASTCALL StaticGenericSpline(D t, const T(&p)[4])
  {
    D t2 = t*t;
    D t3 = t2*t;
    return p[0]*(m[0][0]*t3+m[1][0]*t2+m[2][0]*t+m[3][0]) +
      p[1]*(m[0][1]*t3+m[1][1]*t2+m[2][1]*t+m[3][1]) +
      p[2]*(m[0][2]*t3+m[1][2]*t2+m[2][2]*t+m[3][2]) +
      p[3]*(m[0][3]*t3+m[1][3]*t2+m[2][3]*t+m[3][3]);
  }

  // This implements all possible B-spline functions using a given matrix m, optimized for floats
  inline static float BSS_FASTCALL GenericBSpline(float t, const float(&p)[4], const float(&m)[4][4])
  {
    float a[4] ={ t*t*t, t*t, t, 1 };
    sseVec r = MatrixMultiply1x4(a, m);
    r *= sseVec(p);
    sseVec r2 = r;
    sseVec::Shuffle<0xB1>(r2);
    r += r2;
    sseVec::Shuffle<0x1B>(r2);
    return BSS_SSE_SS_F32(r + r2);
  }

  // Implementation of a bezier curve. The B-spline matrix for this is
  //                / -1  3 -3  1 \   / p1 \
      // [t^3,t²,t,1] * |  3 -6  3  0 | * | p2 |
  //                | -3  3  0  0 |   | p3 |
  //                \  1  0  0  0 /   \ p4 /
  template<typename T, typename D>
  inline static T BSS_FASTCALL BezierCurve(D t, const T& p1, const T& p2, const T& p3, const T& p4)
  {
#ifdef BSS_COMPILER_GCC
    constexpr float m[4][4] ={-1, 3, -3, 1, 3, -6, 3, 0, -3, 3, 0, 0, 1, 0, 0, 0};
#else
    static const float m[4][4] ={ -1, 3, -3, 1, 3, -6, 3, 0, -3, 3, 0, 0, 1, 0, 0, 0 };
#endif
    const float p[4] ={ p1, p2, p3, p4 };
    return StaticGenericSpline<T, D, m>(t, p);
  }

  // Generic breadth-first search for a binary tree. Don't use this on a min/maxheap - it's internal array already IS in breadth-first order.
  template<typename T, bool(*FACTION)(T*), T* (*LCHILD)(T*), T* (*RCHILD)(T*)> // return true to quit
  inline static void BSS_FASTCALL BreadthFirstTree(T* root, size_t n)
  {
    n=(n/2)+1;
    DYNARRAY(T*, queue, n);
    queue[0]=root;
    size_t l=1;
    for(size_t i=0; i!=l; i=(i+1)%n)
    {
      if(FACTION(queue[i])) return;
      queue[l]=LCHILD(queue[i]); //Enqueue the children
      l=(l+(size_t)(queue[l]!=0))%n;
      queue[l]=RCHILD(queue[i]);
      l=(l+(size_t)(queue[l]!=0))%n;
    }
  }
  // Breadth-first search for any directed graph. If FACTION returns true, terminates.
  template<typename G, bool(*FACTION)(typename G::ST_)>
  inline static void BSS_FASTCALL BreadthFirstGraph(G& graph, typename G::ST_ root)
  {
    DYNARRAY(typename G::ST_, queue, graph.NumNodes());
    BreadthFirstGraph<G, FACTION>(graph, root, queue);
  }

  // Breadth-first search for any directed graph. If FACTION returns true, terminates. queue must point to an array at least GetNodes() long.
  template<typename G, bool(*FACTION)(typename G::ST_)>
  static void BSS_FASTCALL BreadthFirstGraph(G& graph, typename G::ST_ root, typename G::ST_* queue)
  {
    typedef typename G::ST_ ST;
    typedef typename std::make_signed<ST>::type SST;
    typedef Edge<typename G::E_, ST> E;
    auto& n = graph.GetNodes();
    if((*FACTION)(root)) return;
    DYNARRAY(ST, aset, graph.Capacity());
    cDisjointSet<ST, StaticNullPolicy<SST>> set((SST*)aset, graph.Capacity());

    // Queue up everything next to the root, checking only for edges that connect the root to itself
    size_t l=0;
    for(E* edge=n[root].to; edge!=0; edge=edge->next) {
      if(edge->to!=root)
        queue[l++]=edge->to;
    }

    for(size_t i=0; i!=l; ++i) // Go through the queue
    {
      if(FACTION(queue[i])) return;
      set.Union(root, queue[i]);
      for(E* edge=n[queue[i]].to; edge!=0; edge=edge->next) {
        if(set.Find(edge->to)!=root) //Enqueue the children if they aren't already in the set.
          queue[l++]=edge->to; // Doesn't need to be circular because we can only enqueue n-1 anyway.
      }
    }
  }
}


#endif
