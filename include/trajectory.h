// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __TRAJECTORY_H__BSS__
#define __TRAJECTORY_H__BSS__

#include "bss_algo.h"
#include "cDynArray.h"

namespace bss_util {
  // Given a scalar field f(x,y,...) and it's gradient fd(x,y,...,&out), returns a vector pointing in the direction of the
  // optimal trajectory as estimated by the function.
  template<typename T, typename F, typename FD, uint DIM>
  void SolveTrajectory(F f, FD fd,T (&src)[DIM],T (&dest)[DIM], T (&out)[DIM]) 
  {
    SmoothTrajectory(f,fd,src,dest,out);
  }

  // Basic perturbation function. If your scalar field is dynamical, you will want to calculate an appropriate acceleration and then
  // assign values according to that acceleration. You may also return a cost value that will be appended to the result.
  template<typename T, uint DIM, T X>
  inline T BSS_FASTCALL Perturb(T (&src)[DIM],T (&dest)[DIM])
  {
    for(uint i = 0; i < DIM; ++i) dest[i]=src[i]+RANDFLOATGEN(-X,X);
    return 0.0f;
  }

  // Uses a stochastic beam search an optimal path through a scalar field f.
  // Outputs the first point of that path, and returns an estimate of the fitness value of the path in question.
  template<typename T, typename F, typename FD, uint DIM, T (*SMPERTURB)(T (&)[DIM],T (&)[DIM]),T (*PERTURB)(T (&)[DIM],T (&)[DIM]), uint K1, uint K2>
  T BSS_FASTCALL BeamSearch(T last, F f, FD fd,T (&src)[DIM],T (&dest)[DIM], T (&best)[DIM], T (&out)[DIM])
  {
    T vres[2+K+K2][DIM];
    T res[2+K+K2];
    T tmp[DIM];

    for(uint i = 0; i < DIM; ++i) vres[0][i]=best[i];
    res[0]=Integrate(f,src,best); // Get value of our previous solution
    res[1]=DirEstimate(f,fd,src,dest,vres[1]); // Then attempt to follow the gradient
    for(uint i = 0; i < K1; ++i)// Do K1 perturbations on the previous solution
    {
      res[2+i]=SMPERTURB(best,tmp);
      res[2+i]+=BeamSearch(Integrate(f,src,tmp),f,fd,tmp,dest,best,vres[2+i]);
    }
    for(uint i = 0; i < K2; ++i)// Do K2 completely random attempts
    {
      res[2+K1+i]=PERTURB(best,tmp);
      res[2+K1+i]+=BeamSearch(f,fd,src,dest,tmp,vres[2+K1+i]);
    }

    uint min=0;
    for(uint i=1; i<2+K+K2; ++i) // Find lowest value
      if(res[min]>res[i])
        min=i;

    for(uint i = 0; i < DIM; ++i) out[i]=vres[min][i]; // Set output to lowest corresponding vector
    return res[min]+last; // Return resulting fitness value
  }

  // Constructs a tangent surface using the gradient and the current point, then projects the vector pointing from the current point
  // to the destination on to this surface. The vector is normalized outside of the function.
  template<typename T, typename F, typename FD, uint DIM>
  inline void BSS_FASTCALL DirEstimate(F f, FD fd,T (&src)[DIM],T (&dest)[DIM], T (&out)[DIM])
  {
    T grad[DIM];
    fd(src,grad);
    T d = -NDot<T,DIM>(src,grad);
    //float sn = NDot<T,DIM>(src,grad) + d;
    float en = NDot<T,DIM>(dest,grad) + d;
    float n2 = NDot<T,DIM>(grad,grad); // square length of normal

    //src -= (sn / n2) * N; // projection on plane
    NVectMul<T,DIM>(grad,en/n2,grad);
    NVectSub<T,DIM>(dest,grad,out);
  }

  // Approximates the definite integral of f from a to b with Simpson's rule using n subintervals
  template<typename T, typename F, uint DIM>
  inline T Integrate(F f,T (&a)[DIM],T (&b)[DIM], uint n=8)
  {
    //T h = (b - a) / n;
    T h[DIM];
    T t[DIM];
    NVectSub<T,DIM>(b,a,h);
    NVectMul<T,DIM>(h,1.0f/n,h);
    T s = f(a) + f(b);
 
    for(uint i = 1; i < n; i+=2)
    {
      NVecMul<T,DIM>(h,i,t);
      NVectAdd<T,DIM>(t,a,t);
      s += 4 * f(t); //f(a + i * h)
    }
    --n;
    for(uint i = 2; i < n; i+=2)
    {
      NVecMul<T,DIM>(h,i,t);
      NVectAdd<T,DIM>(t,a,t);
      s += 2 * f(t); //f(a + i * h)
    }
 
    return (s * h) / 3;
  }
}

#endif