// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __PROFILER_H__BUN__
#define __PROFILER_H__BUN__

#include "Array.h"
#include "BlockAlloc.h"
#include "HighPrecisionTimer.h"
#include <cmath>

#ifndef BUN_ENABLE_PROFILER
  #define PROFILE_BEGIN(name)
  #define PROFILE_END(name)
  #define PROFILE_BLOCK(name)
  #define PROFILE_FUNC()
  #define PROFILE_OUTPUT(file, output)
#else
  #define __PROFILE_STATBLOCK(name, str) static bun::Profiler::ProfilerData PROFDATA_##name(str, __FILE__, __LINE__)
  #define __PROFILE_ZONE(name)           bun::ProfilerBlock BLOCK_##name(PROFDATA_##name.id, bun::Profiler::profiler.GetCur())
  #define PROFILE_BEGIN(name)                                                          \
    __PROFILE_STATBLOCK(name, TXT(name));                                              \
    bun::Profiler::PROF_TRIENODE* PROFCACHE_##name = bun::Profiler::profiler.GetCur(); \
    uint64_t PROFTIME_##name                       = bun::Profiler::profiler.StartProfile(PROFDATA_##name.id)
  #define PROFILE_END(name) bun::Profiler::profiler.EndProfile(PROFTIME_##name, PROFCACHE_##name)
  #define PROFILE_BLOCK(name)             \
    __PROFILE_STATBLOCK(name, TXT(name)); \
    __PROFILE_ZONE(name);
  #define PROFILE_FUNC()                     \
    __PROFILE_STATBLOCK(func, __FUNCTION__); \
    __PROFILE_ZONE(func)
  #define PROFILE_OUTPUT(file, output) bun::Profiler::profiler.WriteToFile(file, output)
#endif

namespace bun {
  namespace internal {
    struct PROF_HEATNODE;
    struct PROF_FLATOUT;
  }

  struct BUN_DLLEXPORT Profiler
  {
    typedef uint16_t ProfilerInt;

    struct PROF_TRIENODE
    {
      PROF_TRIENODE* _children[16];
      double avg;
      double codeavg;
      // double variance;
      uint64_t total; // If total is -1 this isn't a terminating node
      uint64_t inner;
    };

    struct ProfilerData
    {
      const char* name;
      const char* file;
      size_t line;
      ProfilerInt id;

      inline ProfilerData(const char* Name, const char* File, size_t Line) : name(Name), file(File), line(Line), id(++total)
      {
        Profiler::profiler.AddData(id, this);
      }
    };

    BUN_FORCEINLINE uint64_t StartProfile(ProfilerInt id)
    {
      PROF_TRIENODE** r = &_cur;

      while(id > 0)
      {
        r  = &(*r)->_children[id % 16];
        id = (id >> 4);
        if(!*r)
          *r = _allocNode();
      }

      _cur = *r;
      if(_cur->total == (uint64_t)~0)
        _cur->total = 0;
      _cur->inner = 0;
      return HighPrecisionTimer::OpenProfiler();
    }
    BUN_FORCEINLINE void EndProfile(uint64_t time, PROF_TRIENODE* old)
    {
      time          = HighPrecisionTimer::CloseProfiler(time);
      _cur->avg     = bun_Avg<double, uint64_t>(_cur->avg, (double)time, ++_cur->total);
      _cur->codeavg = bun_Avg<double, uint64_t>(_cur->codeavg, (double)(time - _cur->inner), _cur->total);
      _cur          = old;
      _cur->inner += time;
    }
    BUN_FORCEINLINE PROF_TRIENODE* GetRoot() { return _trie; }
    BUN_FORCEINLINE PROF_TRIENODE* GetCur() { return _cur; }
    void AddData(ProfilerInt id, ProfilerData* p);
    enum OUTPUT_DATA : uint8_t
    {
      OUTPUT_FLAT    = 1,
      OUTPUT_TREE    = 2,
      OUTPUT_HEATMAP = 4,
      OUTPUT_ALL     = 1 | 2 | 4
    };
    void WriteToFile(const char* s, uint8_t output);
    void WriteToStream(std::ostream& stream, uint8_t output);

    static Profiler profiler;
    static const ProfilerInt BUFSIZE = 4096;

  private:
    Profiler();
    PROF_TRIENODE* _allocNode();
    void _treeOut(std::ostream& stream, PROF_TRIENODE* node, ProfilerInt id, size_t level, ProfilerInt idlevel);
    void _heatOut(internal::PROF_HEATNODE& heat, PROF_TRIENODE* node, ProfilerInt id, ProfilerInt idlevel);
    void _heatWrite(std::ostream& stream, const internal::PROF_HEATNODE& node, size_t level, double max);
    double _heatFindMax(const internal::PROF_HEATNODE& heat);
    static void _flatOut(internal::PROF_FLATOUT* avg, PROF_TRIENODE* node, ProfilerInt id, ProfilerInt idlevel);
    static const char* _trimPath(const char* path);
    static void _timeFormat(std::ostream& stream, double avg, double variance, uint64_t num);
    static ProfilerInt total;

    Array<ProfilerData*, ProfilerInt> _data;
    PROF_TRIENODE* _trie;
    PROF_TRIENODE* _cur;
    BlockPolicy<PROF_TRIENODE> _alloc;
    size_t _totalnodes;
  };

  inline static bool __DEBUG_VERIFY(Profiler::PROF_TRIENODE* node)
  {
    if(!node)
      return true;

    if(!std::isfinite(node->avg) || !std::isfinite(node->codeavg))
      return false;

    for(int i = 0; i < 16; ++i)
      if(!__DEBUG_VERIFY(node->_children[i]))
        return false;

    return true;
  }

  struct ProfilerBlock
  {
    BUN_FORCEINLINE ProfilerBlock(Profiler::ProfilerInt id, Profiler::PROF_TRIENODE* cache) :
      _cache(cache), _time(Profiler::profiler.StartProfile(id))
    {}
    BUN_FORCEINLINE ~ProfilerBlock() { Profiler::profiler.EndProfile(_time, _cache); }
    Profiler::PROF_TRIENODE* _cache;
    uint64_t _time;
  };
}

#endif
