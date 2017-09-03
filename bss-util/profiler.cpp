// Copyright �2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/Profiler.h"
#include "bss-util/Str.h"
#include "bss-util/ArraySort.h"
#include "bss-util/GreedyAlloc.h"
#include <fstream>

namespace bss {
  namespace internal {
  // Static implementation of the standard allocation policy, used for ArrayBase
    struct PROF_HEATNODE
    {
      struct BSS_COMPILER_DLLEXPORT HeatAllocPolicy {
        static GreedyAlloc _alloc;
        inline static PROF_HEATNODE* allocate(size_t cnt, const PROF_HEATNODE* p = 0)
        {
          auto n = _alloc.AllocT<PROF_HEATNODE>(cnt + 1);
          *((size_t*)n++) = cnt;
          if(p)
          {
            size_t l = *((size_t*)(p - 1));
            memcpy(n, p, l * sizeof(PROF_HEATNODE));
            deallocate(const_cast<PROF_HEATNODE*>(p));
          }

          return n;
        } static void deallocate(PROF_HEATNODE* p, size_t = 0) { _alloc.Dealloc(p - 1); }
      };
      static inline char COMP(const PROF_HEATNODE& l, const PROF_HEATNODE& r) { return SGNCOMPARE(r.avg, l.avg); }
      inline bool DEBUGCHECK()
      {
        uint8_t buf[sizeof(PROF_HEATNODE)];
        bssFill(buf, 0xfd);
        for(PROF_HEATNODE* p = _children.begin(); p != _children.end(); ++p)
          if(!memcmp(p, buf, sizeof(PROF_HEATNODE)) || !p->DEBUGCHECK())
          {
            assert(false);
            return false;
          }
        return true;
      }
      inline PROF_HEATNODE() : avg(0.0), id(0) {}
      inline PROF_HEATNODE(Profiler::ProfilerInt ID, double Avg) : avg(Avg), id(ID) {}
      inline PROF_HEATNODE(const PROF_HEATNODE& copy) : _children(copy._children), avg(copy.avg), id(copy.id) {}
      inline PROF_HEATNODE(PROF_HEATNODE&& mov) : _children(std::move(mov._children)), avg(mov.avg), id(mov.id) {}
      ArraySort<PROF_HEATNODE, COMP, size_t, ARRAY_CONSTRUCT, HeatAllocPolicy> _children;
      double avg;
      Profiler::ProfilerInt id;
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
}

using namespace bss;
using namespace bss::internal;

Profiler Profiler::profiler;
Profiler::ProfilerInt Profiler::total = 0;
GreedyAlloc PROF_HEATNODE::HeatAllocPolicy::_alloc(128 * sizeof(PROF_HEATNODE));

Profiler::Profiler() : _alloc(32), _data(1), _totalnodes(0) { _trie = _cur = _allocNode(); _data[0] = 0; }

void Profiler::AddData(ProfilerInt id, ProfilerData* p)
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
    _treeOut(stream, _trie, 0, -1, 0);
    stream << std::endl << std::endl;
  }
  if(output | OUTPUT_FLAT)
  {
    stream << "BSS Profiler Flat Output: " << std::endl;
    PROF_FLATOUT* avg = (PROF_FLATOUT*)calloc(_data.Capacity(), sizeof(PROF_FLATOUT));
    if(!avg) return;
    _flatOut(avg, _trie, 0, 0);
    std::sort(avg + 1, avg + _data.Capacity(), [](const PROF_FLATOUT& l, const PROF_FLATOUT& r) -> bool { return l.avg > r.avg; });
    for(ProfilerInt i = 1; i < _data.Capacity(); ++i)
    {
      stream << '[' << _trimPath(_data[i]->file) << ':' << _data[i]->line << "] " << _data[i]->name << ": ";
      _timeFormat(stream, avg[i].avg, avg[i].var, avg[i].total);
      stream << std::endl;
    }
    free(avg);
    stream << std::endl << std::endl;
  }
  PROF_HEATNODE::HeatAllocPolicy::_alloc.Clear();
  if(output | OUTPUT_HEATMAP)
  {
    PROF_HEATNODE root;
    _heatOut(root, _trie, 0, 0);
    stream << "BSS Profiler Heat Output: " << std::endl;
    _heatWrite(stream, root, -1, _heatFindMax(root));
  }
}
void Profiler::_treeOut(std::ostream& stream, PROF_TRIENODE* node, ProfilerInt id, size_t level, ProfilerInt idlevel)
{
  if(!node) return;
  if(node->total != (uint64_t)-1)
  {
    for(size_t i = 0; i < level * 2; ++i) stream.put(' ');
    stream << '[' << _trimPath(_data[id]->file) << ':' << _data[id]->line << "] " << _data[id]->name << ": ";
    _timeFormat(stream, node->avg, 0.0, node->total);
    stream << std::endl;
    id = 0;
    idlevel = 0;
  }
  for(ProfilerInt i = 0; i < 16; ++i)
    _treeOut(stream, node->_children[i], id | (i << (4 * idlevel)), level + !id, idlevel + 1);
}
void Profiler::_heatOut(PROF_HEATNODE& heat, PROF_TRIENODE* node, ProfilerInt id, ProfilerInt idlevel)
{
  if(!node) return;
  if(node->total != (uint64_t)-1)
  {
    auto k = heat._children.Insert(PROF_HEATNODE(id, node->avg));
    PROF_HEATNODE& nheat = heat._children[k];
    id = 0;
    idlevel = 0;

    for(ProfilerInt i = 0; i < 16; ++i)
      _heatOut(nheat, node->_children[i], id | (i << (4 * idlevel)), 0);

    double total = 0.0; // Create the [code] node
    for(ProfilerInt i = 0; i < nheat._children.Length(); ++i)
      total += nheat._children[i].avg;

    if(nheat._children.Length() > 0)
      nheat._children.Insert(PROF_HEATNODE(0, node->codeavg));
    assert(heat.DEBUGCHECK());
  }
  else
    for(ProfilerInt i = 0; i < 16; ++i)
      _heatOut(heat, node->_children[i], id | (i << (4 * idlevel)), idlevel + 1);
}
double Profiler::_heatFindMax(PROF_HEATNODE& heat)
{
  double max = heat.avg;
  for(ProfilerInt i = 0; i < heat._children.Length(); ++i)
    max = std::max(_heatFindMax(heat._children[i]), max);
  return max;
}
void Profiler::_heatWrite(std::ostream& stream, PROF_HEATNODE& node, size_t level, double max)
{
  static const int BARLENGTH = 10;

  if(level != (size_t)-1)
  {
    for(size_t i = 0; i < level * 2; ++i) stream.put(' ');
    if(!node.id)
      stream << "[code]: ";
    else
      stream << '[' << _trimPath(_data[node.id]->file) << ':' << _data[node.id]->line << "] " << _data[node.id]->name << ": ";
    _timeFormat(stream, node.avg, 0.0, 0);
    //double percent = (node.avg/max);
    double mag1 = pow(max, 1.0 / 4.0);
    double mag2 = pow(max, 2.0 / 4.0);
    double mag3 = pow(max, 3.0 / 4.0);
    size_t num = fFastTruncate(std::min(node.avg / mag1, 10.0));
    num += fFastTruncate(std::min(node.avg / mag2, 10.0));
    num += fFastTruncate(std::min(node.avg / mag3, 10.0));
    char bar[BARLENGTH + 1] = {};
    bssFillN<char>(bar, BARLENGTH, ' ');
    for(size_t i = 0; i < num; ++i)
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
  for(ProfilerInt i = 0; i < node._children.Length(); ++i)
    _heatWrite(stream, node._children[i], level + 1, max);
}
void Profiler::_flatOut(PROF_FLATOUT* avg, PROF_TRIENODE* node, ProfilerInt id, ProfilerInt idlevel)
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
  for(ProfilerInt i = 0; i < 16; ++i)
    _flatOut(avg, node->_children[i], id | (i << (4 * idlevel)), idlevel + 1);

}
const char* Profiler::_trimPath(const char* path)
{
  const char* r = strrchr(path, '/');
  const char* r2 = strrchr(path, '\\');
  r = std::max(r, r2);
  return (!r) ? path : (r + 1);
}
void Profiler::_timeFormat(std::ostream& stream, double avg, double variance, uint64_t num)
{
  //double sd = bss::FastSqrt(variance/(double)(num-1));
  if(avg >= 1000000000000.0)
    stream << (avg / 1000000000.0) << " s";
  else if(avg >= 1000000000.0)
    stream << (avg / 1000000.0) << " ms";
  else if(avg >= 1000000.0)
    stream << (avg / 1000.0) << L" \x039Cs";
  else
    stream << avg << " ns";
}
Profiler::PROF_TRIENODE* Profiler::_allocNode()
{
  PROF_TRIENODE* r = _alloc.Alloc(1);
  bssFill(*r, 0);
  r->total = (uint64_t)-1;
  ++_totalnodes;
  return r;
}