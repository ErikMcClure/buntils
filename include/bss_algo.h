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

  /*
  // Implementation of Fast Poisson Disk Sampling by Robert Bridson
  template<typename T>
  std::unique_ptr<T[]> PoissonDiskSample(T (&rect)[4], T mindist, int pointsPerIteration=30)
  {
    //Create the grid
    cellSize = min_dist/sqrt(2);
    T w = rect[2]-rect[0];
    T h = rect[3]-rect[1];
    grid = Grid2D(Point(
      (ceil(w/cell_size),         //grid width
      ceil(h/cell_size))));      //grid height

    //RandomQueue works like a queue, except that it
    //pops a random element from the queue instead of
    //the element at the head of the queue
    processList = RandomQueue();
    int len = (1+(int)(w/mindist)) * (1+(int)(h/mindist));
    std::unique_ptr<float[]> ret(new float[len]);
    int count=0;

    float pt[2] = { RANDFLOATGEN(rect[0],rect[2]), RANDFLOATGEN(rect[1],rect[3]) };

    //update containers
    processList.push(firstPoint);
    ret[count]=pt[0];
    ret[++count]=pt[1];
    grid[imageToGrid(firstPoint, cellSize)] = firstPoint;

    //generate other points from points in queue.
    while (not processList.empty())
    {
      point = processList.pop();
      for (i = 0; i < new_points_count; i++)
      {
        newPoint = generateRandomPointAround(point, min_dist);
        //check that the point is in the image region
        //and no points exists in the point's neighbourhood
        if (inRectangle(newPoint) and
          not inNeighbourhood(grid, newPoint, min_dist,
            cellSize))
        {
          //update containers
          processList.push(newPoint);
          samplePoints.push(newPoint);
          grid[imageToGrid(newPoint, cellSize)] =  newPoint;
        }
      }
    }
    return ret;
  }
    static void BSS_FASTCALL Sample(float (&rect)[4], float mindist, int pointsPerIteration=30)
	  {
      Settings settings =  { rect, { rect[2]-rect[0],rect[3]-rect[1] }, { (rect[2]-rect[0])*0.5f,(rect[3]-rect[1])*0.5f },
        mindist, mindist/SQRT_TWO, 0, 0, 0 };
      settings.gwidth = (int) (settings.dim[0] / settings.cellsz) + 1;
      settings.gheight = (int) (settings.dim[1] / settings.cellsz) + 1;
      settings.grid = new float[settings.gwidth*settings.gheight][2];
      
		  AddFirstPoint(settings);

      while (state.ActivePoints.Count != 0)
		  {
        var listIndex = RandomHelper.Random.Next(state.ActivePoints.Count);

        var point = state.ActivePoints[listIndex];
        var found = false;

        for (var k = 0; k < pointsPerIteration; k++)
          found |= AddNextPoint(point, ref settings, ref state);

        if (!found)
          state.ActivePoints.RemoveAt(listIndex);
		  }

		  return state.Points;
	  }

    inline static void BSS_FASTCALL AddFirstPoint(Settings& settings)
    {
        var added = false;
        while (!added)
        {
            var d = RandomHelper.Random.NextDouble();
            var xr = settings.TopLeft.X + settings.Dimensions.X * d;

            d = RandomHelper.Random.NextDouble();
            var yr = settings.TopLeft.Y + settings.Dimensions.Y * d;

            var p = new Vector2((float) xr, (float) yr);
                continue;
            added = true;

            var index = Denormalize(p, settings.TopLeft, settings.CellSize);

            state.Grid[(int) index.X, (int) index.Y] = p;

            state.ActivePoints.Add(p);
            state.Points.Add(p);
        } 
    }

    inline static bool BSS_FASTCALL AddNextPoint(Vector2 point, ref Settings settings, ref State state)
	  {
		  var found = false;
          var q = GenerateRandomAround(point, settings.MinimumDistance);

          if (q.X >= settings.TopLeft.X && q.X < settings.LowerRight.X && 
              q.Y > settings.TopLeft.Y && q.Y < settings.LowerRight.Y &&
		  {
              var qIndex = Denormalize(q, settings.TopLeft, settings.CellSize);
			  var tooClose = false;

              for (var i = (int)Math.Max(0, qIndex.X - 2); i < Math.Min(settings.GridWidth, qIndex.X + 3) && !tooClose; i++)
                  for (var j = (int)Math.Max(0, qIndex.Y - 2); j < Math.Min(settings.GridHeight, qIndex.Y + 3) && !tooClose; j++)
					  if (state.Grid[i, j].HasValue && Vector2.Distance(state.Grid[i, j].Value, q) < settings.MinimumDistance)
						tooClose = true;

			  if (!tooClose)
			  {
				  found = true;
				  state.ActivePoints.Add(q);
				  state.Points.Add(q);
                  state.Grid[(int)qIndex.X, (int)qIndex.Y] = q;
			  }
		  }
		  return found;
	  }

    static Vector2 GenerateRandomAround(Vector2 center, float minimumDistance)
    {
        var d = RandomHelper.Random.NextDouble();
        var radius = minimumDistance + minimumDistance * d;

        d = RandomHelper.Random.NextDouble();
        var angle = MathHelper.TwoPi * d;

        var newX = radius * Math.Sin(angle);
        var newY = radius * Math.Cos(angle);

        return new Vector2((float) (center.X + newX), (float) (center.Y + newY));
    }

    static Vector2 Denormalize(Vector2 point, Vector2 origin, double cellSize)
    {
        return new Vector2((int) ((point.X - origin.X) / cellSize), (int) ((point.Y - origin.Y) / cellSize));
    }
  };*/
}


#endif
