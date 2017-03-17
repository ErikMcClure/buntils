// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __PROFILER_H__BSS__
#define __PROFILER_H__BSS__

#include "cHighPrecisionTimer.h"
#include "cArray.h"
#include "bss_alloc_block.h"
#include <cmath>

#ifndef BSS_ENABLE_PROFILER
#define PROFILE_BEGIN(name)
#define PROFILE_END(name)
#define PROFILE_BLOCK(name)
#define PROFILE_FUNC()
#define PROFILE_OUTPUT(file,output)
#else
#define __PROFILE_STATBLOCK(name,str) static bss_util::Profiler::ProfilerData PROFDATA_##name(str,__FILE__,__LINE__)
#define __PROFILE_ZONE(name) bss_util::ProfilerBlock BLOCK_##name(PROFDATA_##name .id, bss_util::Profiler::profiler.GetCur())
#define PROFILE_BEGIN(name) __PROFILE_STATBLOCK(name, MAKESTRING(name)); PROF_TRIENODE* PROFCACHE_##name = bss_util::Profiler::profiler.GetCur(); uint64_t PROFTIME_##name = bss_util::Profiler::profiler.StartProfile(PROFDATA_##name .id)
#define PROFILE_END(name) bss_util::Profiler::profiler.EndProfile(PROFTIME_##name, PROFCACHE_##name)
#define PROFILE_BLOCK(name) __PROFILE_STATBLOCK(name, MAKESTRING(name)); __PROFILE_ZONE(name);
#define PROFILE_FUNC() __PROFILE_STATBLOCK(func, __FUNCTION__); __PROFILE_ZONE(func)
#define PROFILE_OUTPUT(file,output) bss_util::Profiler::profiler.WriteToFile(file,output)
#endif

typedef uint16_t PROFILER_INT;

namespace bss_util {
  struct PROF_HEATNODE;
  struct PROF_FLATOUT;

  struct PROF_TRIENODE
  {
    PROF_TRIENODE* _children[16];
    double avg;
    double codeavg;
    //double variance; 
    uint64_t total; // If total is -1 this isn't a terminating node
    uint64_t inner;
  };
  struct BSS_DLLEXPORT Profiler
  {
    struct ProfilerData
    {
      const char* name;
      const char* file;
      uint32_t line;
      PROFILER_INT id;

      inline ProfilerData(const char* Name, const char* File, uint32_t Line) : name(Name), file(File), line(Line), id(++total) { Profiler::profiler.AddData(id, this); }
    };

    BSS_FORCEINLINE uint64_t StartProfile(PROFILER_INT id)
    {
      PROF_TRIENODE** r=&_cur;
      while(id>0)
      {
        r = &(*r)->_children[id%16];
        id = (id>>4);
        if(!*r)
          *r = _allocnode();
      }
      _cur=*r;
      if(_cur->total == (uint64_t)-1)
        _cur->total = 0;
      _cur->inner=0;
      return cHighPrecisionTimer::OpenProfiler();
    }
    BSS_FORCEINLINE void EndProfile(uint64_t time, PROF_TRIENODE* old)
    {
      time = cHighPrecisionTimer::CloseProfiler(time);
      //bssvariance<double, uint64_t>(_cur->variance, _cur->avg, (double)time, ++_cur->total);
      _cur->avg = bssavg<double, uint64_t>(_cur->avg, (double)time, ++_cur->total);
      _cur->codeavg = bssavg<double, uint64_t>(_cur->codeavg, (double)(time - _cur->inner), _cur->total);
      _cur=old;
      _cur->inner += time;
    }
    BSS_FORCEINLINE PROF_TRIENODE* GetRoot() { return _trie; }
    BSS_FORCEINLINE PROF_TRIENODE* GetCur() { return _cur; }
    void AddData(PROFILER_INT id, ProfilerData* p);
    enum OUTPUT_DATA : uint8_t { OUTPUT_FLAT=1, OUTPUT_TREE=2, OUTPUT_HEATMAP=4, OUTPUT_ALL=1|2|4 };
    void WriteToFile(const char* s, uint8_t output);
    void WriteToStream(std::ostream& stream, uint8_t output);

    static Profiler profiler;
    static const PROFILER_INT BUFSIZE=4096;

  private:
    Profiler();
    PROF_TRIENODE* _allocnode();
    void _treeout(std::ostream& stream, PROF_TRIENODE* node, PROFILER_INT id, uint32_t level, PROFILER_INT idlevel);
    void _heatout(PROF_HEATNODE& heat, PROF_TRIENODE* node, PROFILER_INT id, PROFILER_INT idlevel);
    void _heatwrite(std::ostream& stream, PROF_HEATNODE& node, uint32_t level, double max);
    double _heatfindmax(PROF_HEATNODE& heat);
    static void _flatout(PROF_FLATOUT* avg, PROF_TRIENODE* node, PROFILER_INT id, PROFILER_INT idlevel);
    static const char* _trimpath(const char* path);
    static void _timeformat(std::ostream& stream, double avg, double variance, uint64_t num);
    static PROFILER_INT total;

    cArray<ProfilerData*, PROFILER_INT> _data;
    PROF_TRIENODE* _trie;
    PROF_TRIENODE* _cur;
    cBlockAlloc<PROF_TRIENODE> _alloc;
    uint32_t _totalnodes;
  };

  inline static bool __DEBUG_VERIFY(PROF_TRIENODE* node)
  {
    if(!node) return true;
    if(!std::isfinite(node->avg) || !std::isfinite(node->codeavg)) {
      return false;
    }
    for(int i = 0; i < 16; ++i)
      if(!__DEBUG_VERIFY(node->_children[i]))
        return false;
    return true;
  }

  struct ProfilerBlock
  {
    BSS_FORCEINLINE ProfilerBlock(PROFILER_INT id, PROF_TRIENODE* cache) : _cache(cache), _time(Profiler::profiler.StartProfile(id)) { }
    BSS_FORCEINLINE ~ProfilerBlock() { Profiler::profiler.EndProfile(_time, _cache); }
    PROF_TRIENODE* _cache;
    uint64_t _time;
  };
}

#endif
