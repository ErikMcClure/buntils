// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "profiler.h"
#include "cStr.h"
#include "cArraySort.h"
#include "bss_alloc_greedy.h"
#include <fstream>

using namespace bss_util;

namespace bss_util {
  // Static implementation of the standard allocation policy, used for cArrayBase
  struct PROF_HEATNODE
  {
    struct BSS_COMPILER_DLLEXPORT HeatAllocPolicy {
      static cGreedyAlloc _alloc;
      inline static PROF_HEATNODE* allocate(size_t cnt, const PROF_HEATNODE* p = 0)
      {
        auto n = _alloc.allocT<PROF_HEATNODE>(cnt + 1);
        *((size_t*)n++) = cnt;
        if(p)
        {
          size_t l = *((size_t*)(p - 1));
          memcpy(n, p, l * sizeof(PROF_HEATNODE));
          deallocate(const_cast<PROF_HEATNODE*>(p));
        }

        return n;
      } static void deallocate(PROF_HEATNODE* p, size_t = 0) { _alloc.dealloc(p - 1); }
    };
    static inline char COMP(const PROF_HEATNODE& l, const PROF_HEATNODE& r) { return SGNCOMPARE(r.avg, l.avg); }
    inline bool DEBUGCHECK()
    {
      uint8_t buf[sizeof(PROF_HEATNODE)];
      memset(buf, 0xfd, sizeof(PROF_HEATNODE));
      for(PROF_HEATNODE* p = _children.begin(); p != _children.end(); ++p)
        if(!memcmp(p, buf, sizeof(PROF_HEATNODE)) || !p->DEBUGCHECK())
        {
          assert(false);
          return false;
        }
      return true;
    }
    inline PROF_HEATNODE() : avg(0.0), id(0) {}
    inline PROF_HEATNODE(PROFILER_INT ID, double Avg) : avg(Avg), id(ID) {}
    inline PROF_HEATNODE(const PROF_HEATNODE& copy) : _children(copy._children), avg(copy.avg), id(copy.id) {}
    inline PROF_HEATNODE(PROF_HEATNODE&& mov) : _children(std::move(mov._children)), avg(mov.avg), id(mov.id) {}
    cArraySort<PROF_HEATNODE, COMP, uint32_t, CARRAY_CONSTRUCT, HeatAllocPolicy> _children;
    double avg;
    PROFILER_INT id;
    PROF_HEATNODE& operator=(PROF_HEATNODE&& mov) { _children = std::move(mov._children); avg = mov.avg; id = mov.id; return *this; }
    PROF_HEATNODE& operator=(const PROF_HEATNODE& copy) { _children = copy._children; avg = copy.avg; id = copy.id; return *this; }
  };
  struct PROF_FLATOUT
  {
    double avg;
    double var;
    uint64_t total;
  };
}

Profiler Profiler::profiler;
PROFILER_INT Profiler::total = 0;
cGreedyAlloc PROF_HEATNODE::HeatAllocPolicy::_alloc(128 * sizeof(PROF_HEATNODE));

Profiler::Profiler() : _alloc(32), _data(1), _totalnodes(0) { _trie = _cur = _allocnode(); _data[0] = 0; }

void Profiler::AddData(PROFILER_INT id, ProfilerData* p)
{
  if(_data.Capacity() <= id)
    _data.SetCapacity(id + 1);
  _data[id] = p;
}
void Profiler::WriteToFile(const char* s, uint8_t output)
{
  std::ofstream stream(BSSPOSIX_WCHAR(s), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
  WriteToStream(stream, output);
}
void Profiler::WriteToStream(std::ostream& stream, uint8_t output)
{
#ifdef BSS_DEBUG
  if(!__DEBUG_VERIFY(_trie))
  {
    stream << "ERROR: Infinite value detected in BSS Profiler! Skipping output to avoid infinite loop." << std::endl;
    return;
  }
#endif

  if(output | OUTPUT_TREE)
  {
    stream << "BSS Profiler Tree Output: " << std::endl;
    _treeout(stream, _trie, 0, -1, 0);
    stream << std::endl << std::endl;
  }
  if(output | OUTPUT_FLAT)
  {
    stream << "BSS Profiler Flat Output: " << std::endl;
    PROF_FLATOUT* avg = (PROF_FLATOUT*)calloc(_data.Capacity(), sizeof(PROF_FLATOUT));
    if(!avg) return;
    _flatout(avg, _trie, 0, 0);
    std::sort(avg + 1, avg + _data.Capacity(), [](const PROF_FLATOUT& l, const PROF_FLATOUT& r) -> bool { return l.avg > r.avg; });
    for(PROFILER_INT i = 1; i < _data.Capacity(); ++i)
    {
      stream << '[' << _trimpath(_data[i]->file) << ':' << _data[i]->line << "] " << _data[i]->name << ": ";
      _timeformat(stream, avg[i].avg, avg[i].var, avg[i].total);
      stream << std::endl;
    }
    free(avg);
    stream << std::endl << std::endl;
  }
  PROF_HEATNODE::HeatAllocPolicy::_alloc.Clear();
  if(output | OUTPUT_HEATMAP)
  {
    PROF_HEATNODE root;
    _heatout(root, _trie, 0, 0);
    stream << "BSS Profiler Heat Output: " << std::endl;
    _heatwrite(stream, root, -1, _heatfindmax(root));
  }
}
void Profiler::_treeout(std::ostream& stream, PROF_TRIENODE* node, PROFILER_INT id, uint32_t level, PROFILER_INT idlevel)
{
  if(!node) return;
  if(node->total != (uint64_t)-1)
  {
    for(uint32_t i = 0; i < level * 2; ++i) stream.put(' ');
    stream << '[' << _trimpath(_data[id]->file) << ':' << _data[id]->line << "] " << _data[id]->name << ": ";
    _timeformat(stream, node->avg, 0.0, node->total);
    stream << std::endl;
    id = 0;
    idlevel = 0;
  }
  for(PROFILER_INT i = 0; i < 16; ++i)
    _treeout(stream, node->_children[i], id | (i << (4 * idlevel)), level + !id, idlevel + 1);
}
void Profiler::_heatout(PROF_HEATNODE& heat, PROF_TRIENODE* node, PROFILER_INT id, PROFILER_INT idlevel)
{
  if(!node) return;
  if(node->total != (uint64_t)-1)
  {
    auto k = heat._children.Insert(PROF_HEATNODE(id, node->avg));
    PROF_HEATNODE& nheat = heat._children[k];
    id = 0;
    idlevel = 0;

    for(PROFILER_INT i = 0; i < 16; ++i)
      _heatout(nheat, node->_children[i], id | (i << (4 * idlevel)), 0);

    double total = 0.0; // Create the [code] node
    for(PROFILER_INT i = 0; i < nheat._children.Length(); ++i)
      total += nheat._children[i].avg;

    if(nheat._children.Length() > 0)
      nheat._children.Insert(PROF_HEATNODE(0, node->codeavg));
    assert(heat.DEBUGCHECK());
  }
  else
    for(PROFILER_INT i = 0; i < 16; ++i)
      _heatout(heat, node->_children[i], id | (i << (4 * idlevel)), idlevel + 1);
}
double Profiler::_heatfindmax(PROF_HEATNODE& heat)
{
  double max = heat.avg;
  for(PROFILER_INT i = 0; i < heat._children.Length(); ++i)
    max = std::max(_heatfindmax(heat._children[i]), max);
  return max;
}
void Profiler::_heatwrite(std::ostream& stream, PROF_HEATNODE& node, uint32_t level, double max)
{
  static const int BARLENGTH = 10;

  if(level != (uint32_t)-1)
  {
    for(uint32_t i = 0; i < level * 2; ++i) stream.put(' ');
    if(!node.id)
      stream << "[code]: ";
    else
      stream << '[' << _trimpath(_data[node.id]->file) << ':' << _data[node.id]->line << "] " << _data[node.id]->name << ": ";
    _timeformat(stream, node.avg, 0.0, 0);
    //double percent = (node.avg/max);
    double mag1 = pow(max, 1.0 / 4.0);
    double mag2 = pow(max, 2.0 / 4.0);
    double mag3 = pow(max, 3.0 / 4.0);
    uint32_t num = fFastTruncate(std::min(node.avg / mag1, 10.0));
    num += fFastTruncate(std::min(node.avg / mag2, 10.0));
    num += fFastTruncate(std::min(node.avg / mag3, 10.0));
    char bar[BARLENGTH + 1] = {};
    memset(bar, ' ', BARLENGTH);
    for(uint32_t i = 0; i < num; ++i)
    {
      switch(i / BARLENGTH)
      {
      case 0: bar[i%BARLENGTH] = '.'; break;
      case 1: bar[i%BARLENGTH] = '-'; break;
      case 2: bar[i%BARLENGTH] = '#'; break;
      }
    }
    stream << "   [" << bar << std::endl;
  }
  if(node._children.Length() == 1 && !node._children[0].id)
    return;
  for(PROFILER_INT i = 0; i < node._children.Length(); ++i)
    _heatwrite(stream, node._children[i], level + 1, max);
}
void Profiler::_flatout(PROF_FLATOUT* avg, PROF_TRIENODE* node, PROFILER_INT id, PROFILER_INT idlevel)
{
  if(!node) return;
  if(node->total != (uint64_t)-1)
  {
    if(node->total > 0) // if node->total is zero the equation will detonate, and this can easily happen for any node that doesn't get run.
    { // Equation: new_avg = a1 * (N1/(N1+N2)) + a2 * (N2/(N1+N2))
      double navg = avg[id].avg * (avg[id].total / ((double)(avg[id].total + node->total))) + node->avg * (node->total / ((double)(avg[id].total + node->total)));
      //var[id] = avg[id].var + node->variance + avg[id].total*(avg[id].avg - navg)*(avg[id].avg - navg) + node->total*(node->avg - navg)*(node->avg - navg);
      avg[id].avg = navg;
      avg[id].total += node->total;
    }
    id = 0;
    idlevel = 0;
  }
  for(PROFILER_INT i = 0; i < 16; ++i)
    _flatout(avg, node->_children[i], id | (i << (4 * idlevel)), idlevel + 1);

}
const char* Profiler::_trimpath(const char* path)
{
  const char* r = strrchr(path, '/');
  const char* r2 = strrchr(path, '\\');
  r = bssmax(r, r2);
  return (!r) ? path : (r + 1);
}
void Profiler::_timeformat(std::ostream& stream, double avg, double variance, uint64_t num)
{
  //double sd = bss_util::FastSqrt(variance/(double)(num-1));
  if(avg >= 1000000000000.0)
    stream << (avg / 1000000000.0) << " s";
  else if(avg >= 1000000000.0)
    stream << (avg / 1000000.0) << " ms";
  else if(avg >= 1000000.0)
    stream << (avg / 1000.0) << L" \x039Cs";
  else
    stream << avg << " ns";
}
PROF_TRIENODE* Profiler::_allocnode()
{
  PROF_TRIENODE* r = _alloc.alloc(1);
  memset(r, 0, sizeof(PROF_TRIENODE));
  r->total = (uint64_t)-1;
  ++_totalnodes;
  return r;
}