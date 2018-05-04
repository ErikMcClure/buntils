// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __RANDOM_QUEUE_BSS_H__
#define __RANDOM_QUEUE_BSS_H__

#include "DynArray.h"
#include "XorshiftEngine.h"
#include "algo.h"

namespace bss {
  // Random queue that pops a random item instead of the last item.
  template<typename T, typename CType = uint32_t, ARRAY_TYPE ArrayType = ARRAY_SIMPLE, typename Alloc = StandardAllocator<T>>
  class BSS_COMPILER_DLLEXPORT RandomQueue : protected DynArray<T, CType, ArrayType, Alloc>
  {
  protected:
    typedef DynArray<T, CType, ArrayType, Alloc> BASE;
    using BASE::_array;
    using BASE::_length;
    typedef typename BASE::CT CT;
    typedef typename BASE::Ty Ty;

  public:
    RandomQueue(const RandomQueue& copy) = default;
    RandomQueue(RandomQueue&& mov) = default;
    explicit RandomQueue(CT size = 0) : BASE(size) {}
    BSS_FORCEINLINE void Push(const T& item) { BASE::Add(item); }
    BSS_FORCEINLINE void Push(T&& item) { BASE::Add(std::move(item)); }
    template<class ENGINE>
    inline T Pop(ENGINE& e)
    {
      CT i = bssrand<CT, ENGINE>(0, _length, e);
      T r = std::move(_array[i]);
      Remove(i);
      return r;
    }
    BSS_FORCEINLINE T Pop() { return Pop(bss_getdefaultengine()); }
    inline void Remove(CT index) { _array[index] = std::move(_array[--_length]); }
    inline bool Empty() const { return !_length; }
    inline void Clear() { _length = 0; }
    BSS_FORCEINLINE void SetLength(CT length) { BASE::SetLength(length); }
    inline CT Length() const { return _length; }
    inline const T* begin() const { return _array; }
    inline const T* end() const { return _array + _length; }
    inline T* begin() { return _array; }
    inline T* end() { return _array + _length; }

    inline operator T*() { return _array; }
    inline operator const T*() const { return _array; }
    inline RandomQueue& operator=(const RandomQueue& copy) = default;
    inline RandomQueue& operator=(RandomQueue&& mov) = default;
    inline RandomQueue& operator +=(const RandomQueue& add) { BASE::operator+=(add); return *this; }
    inline const RandomQueue operator +(const RandomQueue& add) const { RandomQueue r(*this); return (r += add); }

    using BASE::SerializerArray;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { BASE::template Serialize<Engine>(s, id); }
  };

  namespace internal {
    template<typename T>
    BSS_FORCEINLINE size_t _PDS_imageToGrid(const std::array<T, 2>& pt, T cell, size_t gw, T(&rect)[4])
    {
      return (size_t)((pt[0] - rect[0]) / cell) + gw*(size_t)((pt[1] - rect[1]) / cell) + 2 + gw + gw;
    }
  }

  // Implementation of Fast Poisson Disk Sampling by Robert Bridson
  template<typename T, typename F>
  inline void PoissonDiskSample(T(&rect)[4], T mindist, F && f, size_t pointsPerIteration = 30)
  {
    typedef std::array<T, 2> GRID;
    //Create the grid
    T cell = mindist / (T)SQRT_TWO;
    T w = rect[2] - rect[0];
    T h = rect[3] - rect[1];
    size_t gw = ((size_t)ceil(w / cell)) + 4; //gives us buffer room so we don't have to worry about going outside the grid
    size_t gh = ((size_t)ceil(h / cell)) + 4;
    VARARRAY(GRID, grid, (gw*gh));    //grid height
    uint64_t* ig = reinterpret_cast<uint64_t*>((GRID*)grid);
    bssFillN<GRID>(grid, gw*gh, 0xFF);
    assert(!(~ig[0]));

    RandomQueue<std::array<T, 2>> list;
    std::array<T, 2> pt = { (T)bssRandReal(rect[0], rect[2]), (T)bssRandReal(rect[1], rect[3]) };

    //update containers 
    list.Push(pt);
    f(pt.data());
    grid[internal::_PDS_imageToGrid<T>(pt, cell, gw, rect)] = pt;

    T mindistsq = mindist*mindist;
    T radius, angle;
    size_t center, edge;
    //generate other points from points in queue.
    while(!list.Empty())
    {
      auto point = list.Pop();

      for(size_t i = 0; i < pointsPerIteration; i++)
      {
        radius = mindist*((T)bssRandReal(1, 2)); //random point between mindist and 2*mindist
        angle = (T)bssRandReal(0, PI_DOUBLE);
        pt[0] = point[0] + radius * cos(angle); //the new point is generated around the point (x, y)
        pt[1] = point[1] + radius * sin(angle);

        if(pt[0] > rect[0] && pt[0]<rect[2] && pt[1]>rect[1] && pt[1] < rect[3]) //Ensure point is inside recT
        {
          center = internal::_PDS_imageToGrid<T>(pt, cell, gw, rect); // If another point is in the neighborhood, abort this point.
          edge = center - gw - gw;
          assert(edge > 0);
#define POISSONSAMPLE_CHECK(edge) if((~ig[edge])!=0 && DistSqr(grid[edge][0],grid[edge][1],pt[0],pt[1])<mindistsq) continue
          POISSONSAMPLE_CHECK(edge - 1);
          POISSONSAMPLE_CHECK(edge);
          POISSONSAMPLE_CHECK(edge + 1);
          edge += gw;
          if(~(ig[edge - 1] & ig[edge] & ig[edge + 1])) continue;
          POISSONSAMPLE_CHECK(edge - 2);
          POISSONSAMPLE_CHECK(edge + 2);
          edge += gw;
          if(~(ig[edge - 1] & ig[edge] & ig[edge + 1])) continue;
          POISSONSAMPLE_CHECK(edge - 2);
          POISSONSAMPLE_CHECK(edge + 2);
          edge += gw;
          if(~(ig[edge - 1] & ig[edge] & ig[edge + 1])) continue;
          POISSONSAMPLE_CHECK(edge - 2);
          POISSONSAMPLE_CHECK(edge + 2);
          edge += gw;
          POISSONSAMPLE_CHECK(edge - 1);
          POISSONSAMPLE_CHECK(edge);
          POISSONSAMPLE_CHECK(edge + 1);
          list.Push(pt);
          f(pt.data());
          grid[center] = pt;
        }
      }
    }
  }
}

#endif