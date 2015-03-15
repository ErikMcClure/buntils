// Copyright ©2015 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_GRAPH_H__
#define __BSS_GRAPH_H__

#include "cLinkedArray.h"
#include "bss_alloc_block.h"
#include "LLBase.h"
#include <limits.h>
#ifdef BSS_COMPILER_GCC
#include <alloca.h>
#endif

namespace bss_util {
  template<typename D>
  struct BSS_COMPILER_DLLEXPORT VoidData { D data; };
  template<>
  struct BSS_COMPILER_DLLEXPORT VoidData<void> {};

  // DAG edge
  template<typename E, typename ST=unsigned short>
  struct BSS_COMPILER_DLLEXPORT Edge : LLBase<Edge<E, ST>>, VoidData<E> { ST to; ST from; LLBase<Edge<E, ST>> alt; };

  // Node for a DAG.
  template<typename E, typename V, typename ST=unsigned short>
  struct BSS_COMPILER_DLLEXPORT Node : VoidData<V> { Edge<E, ST>* to; Edge<E, ST>* from; };

  template<typename E> struct __Graph__InternalEdge {
    static BSS_FORCEINLINE void _getdata(E& target, const VoidData<E>& t) { target=t.data; }
    static BSS_FORCEINLINE void _setdataE(VoidData<E>& t, const E* d) { t.data=*d; }
  };
  template<> struct __Graph__InternalEdge<void> {
    static BSS_FORCEINLINE void _getdata(char& target, const VoidData<void>& t) { target=1; }
    static BSS_FORCEINLINE void _setdataE(VoidData<void>& t, const void* d) { }
  };

  template<typename V, typename ST>
  struct __Graph__InternalVertex {
    static BSS_FORCEINLINE void _setdataV(VoidData<V>& t, const V* d) { t.data=*d; }
    static BSS_FORCEINLINE void _setvertex(V* v, ST i, const VoidData<V>& t) { v[i] = t.data; }
    static BSS_FORCEINLINE const V* _addvertex(const V* v, ST i) { return (v+i); }
  };
  template<typename ST>
  struct __Graph__InternalVertex<void, ST> {
    static BSS_FORCEINLINE void _setdataV(VoidData<void>& t, const void* d) { }
    static BSS_FORCEINLINE void _setvertex(void* v, ST i, const VoidData<void>& t) { }
    static BSS_FORCEINLINE const void* _addvertex(const void* v, ST i) { return 0; }
  };

  // Represents a graph using an adjacency list. Converts to and from an adjacency matrix representation.
  template<typename E, typename V, typename ST=unsigned short, typename ALLOC=BlockPolicy<Edge<E, ST>>, typename NODEALLOC=StaticAllocPolicy<LINKEDNODE<Node<E, V, ST>, ST>>, ARRAY_TYPE ArrayType = CARRAY_SIMPLE>
  class Graph : public cAllocTracker<ALLOC>, protected __Graph__InternalEdge<E>, protected __Graph__InternalVertex<V, ST>
  {
    static BSS_FORCEINLINE bool _basecheck(const char* b) { return (*b)!=0; }
    static BSS_FORCEINLINE LLBase<Edge<E, ST>>& _altget(Edge<E, ST>* p) { return p->alt; }
    typedef typename std::conditional<std::is_void<E>::value, char, E>::type EDATA;
    using __Graph__InternalVertex<V, ST>::_setdataV;
    using __Graph__InternalVertex<V, ST>::_setvertex;
    using __Graph__InternalVertex<V, ST>::_addvertex;
    using __Graph__InternalEdge<E>::_setdataE;
    using __Graph__InternalEdge<E>::_getdata;

    Graph(const Graph&) BSS_DELETEFUNC
      Graph& operator=(const Graph&)BSS_DELETEFUNCOP
  public:
    inline Graph(Graph&& mov) : _nodes(std::move(mov._nodes)), _nedges(mov._nedges) { mov._nedges=0; }
    inline Graph() : _nodes(0), _nedges(0) {}
    // Build a matrix out of a standard adjacency matrix
    inline Graph(ST n, const char* M, const V* nodes) : _nodes(n), _nedges(0) { _construct<char, _basecheck>(n, M, nodes); }
    // Build a matrix out of only a list of nodes
    inline Graph(ST n, const V* nodes) : _nodes(n), _nedges(0) { for(ST i = 0; i < n; ++i) AddNode(nodes+i); }
    inline explicit Graph(ST n) : _nodes(n), _nedges(0) { for(ST i = 0; i < n; ++i) AddNode(); }
    inline ~Graph() { for(ST i=_nodes.Front(); i!=(ST)-1; _nodes.Next(i)) while(_nodes[i].to) RemoveEdge(_nodes[i].to); }
    inline ST NumNodes() const { return _nodes.Length(); }
    inline ST NumEdges() const { return _nedges; }
    inline ST Capacity() const { return _nodes.Capacity(); }
    inline Node<E, V, ST>* GetNode(ST index) const { return index<_nodes.Length()?&_nodes[index]:0; }
    inline const cLinkedArray<Node<E, V, ST>, ST, ArrayType, NODEALLOC>& GetNodes() const { return _nodes; }
    inline cLinkedArray<Node<E, V, ST>, ST, ArrayType, NODEALLOC>& GetNodes() { return _nodes; }
    inline ST Front() const { return _nodes.Front(); }
    inline ST Back() const { return _nodes.Back(); }
    inline typename cLinkedArray<Node<E, V, ST>, ST, ArrayType, NODEALLOC>::template cLAIter<true> begin() const { return _nodes.begin(); }
    inline typename cLinkedArray<Node<E, V, ST>, ST, ArrayType, NODEALLOC>::template cLAIter<true> end() const { return _nodes.end(); }
    inline typename cLinkedArray<Node<E, V, ST>, ST, ArrayType, NODEALLOC>::template cLAIter<false> begin() { return _nodes.begin(); }
    inline typename cLinkedArray<Node<E, V, ST>, ST, ArrayType, NODEALLOC>::template cLAIter<false> end() { return _nodes.end(); }
    inline ST AddNode() { Node<E, V, ST> aux; memset(&aux, 0, sizeof(Node<E, V, ST>)); return _nodes.Add(aux); }
    inline ST AddNode(const V* node) { if(!node) return AddNode(); Node<E, V, ST> aux; aux.to = 0; aux.from = 0; _setdataV(aux, node); return _nodes.Add(aux); }
    inline Edge<E, ST>* AddEdge(ST from, ST to)
    {
      Edge<E, ST>* r = cAllocTracker<ALLOC>::_allocate(1);
      r->to=to;
      r->from=from;
      LLAdd(r, _nodes[from].to);
      AltLLAdd<Edge<E, ST>, &Graph::_altget>(r, _nodes[to].from);
      ++_nedges;
      return r;
    }
    inline Edge<E, ST>* AddEdge(ST from, ST to, const E* edge)
    {
      Edge<E, ST>* r = AddEdge(from, to);
      _setdataE(*r, edge);
      return r;
    }
    inline void RemoveNode(ST index)
    {
      auto& n = _nodes[index];
      while(n.to) RemoveEdge(n.to);
      while(n.from) RemoveEdge(n.from);
      _nodes.Remove(index);
    }
    inline void RemoveEdge(Edge<E, ST>* edge)
    {
      LLRemove(edge, _nodes[edge->from].to);
      AltLLRemove<Edge<E, ST>, &Graph::_altget>(edge, _nodes[edge->to].from);
      cAllocTracker<ALLOC>::_deallocate(edge);
      --_nedges;
    }
    template<bool ISEDGE(const E*)>
    void FromMatrix(ST n, const E* matrix, const V* nodes) { _construct<E, ISEDGE>(n, matrix, nodes); }
    ST ToMatrix(EDATA* matrix, V* nodes) const
    {
      ST len = _nodes.Length();
      Edge<E, ST>* cur=0;
      if(!matrix) return len;
      DYNARRAY(ST, hash, _nodes.Capacity());
      ST k=0;
      for(ST i=_nodes.Front(); i!=(ST)-1; _nodes.Next(i)) hash[i]=k++; // Build up a hash mapping the IDs to monotonically increasing integers.
      for(ST i=_nodes.Front(); i!=(ST)-1; _nodes.Next(i))
      {
        _setvertex(nodes, i, _nodes[i]);
        for(cur=_nodes[i].to; cur!=0; cur=cur->next)
          _getdata(matrix[(hash[i]*len)+hash[cur->to]], *cur);
      }
      return len;
    }

    inline const Node<E, V, ST>& operator[](ST index) const { return _nodes[index]; }
    inline Node<E, V, ST>& operator[](ST index) { return _nodes[index]; }
    inline Graph& operator=(Graph&& mov) { _nodes = std::move(mov._nodes); _nedges = mov._nedges; mov._nedges=0; return *this; }
    typedef ST ST_;
    typedef E E_;
    typedef V V_;
    typedef Node<E, V, ST> N_;
    typedef Edge<E, ST> EDGE_;

  protected:

    template<typename D, bool(*ISEDGE)(const D*)>
    void _construct(ST n, const D* M, const V* nodes)
    {
      DYNARRAY(ST, k, n);
      for(ST i = 0; i < n; ++i)
        k[i]=AddNode(_addvertex(nodes, i));
      for(ST i = 0; i < n; ++i)
      {
        for(ST j = 0; j < n; ++j)
          if(ISEDGE(M+(i*n)+j))
            AddEdge(k[i], k[j], M+((i*n)+j));
      }
    }

    ST _nedges;
    cLinkedArray<Node<E, V, ST>, ST, ArrayType, NODEALLOC> _nodes;
  };

  template<typename E>
  struct BSS_COMPILER_DLLEXPORT __edge_MaxFlow {
    int flow;
    int capacity;
    VoidData<E> d;
  };

  // Implementation of the FIFO push-relabel algorithm for solving a maximum-flow graph in O(V^3) time
  template<typename E, typename V, typename ST, typename ALLOC, typename NODEALLOC, ARRAY_TYPE ArrayType>
  static void MaxFlow_PushRelabel(Graph<__edge_MaxFlow<E>, V, ST, ALLOC, NODEALLOC, ArrayType>& graph, ST s, ST t)
  {
    typedef Edge<__edge_MaxFlow<E>, ST>* PEDGE;
    typedef std::pair<ST, ST> PAIR;
    ST len=graph.NumNodes(); // Setup
    ST cap=graph.Capacity();
    DYNARRAY(int, excess, cap);
    DYNARRAY(ST, height, cap);
    DYNARRAY(PEDGE, seen, cap);
    DYNARRAY(PAIR, v, (len-2)); //integer based singly-linked list.
    memset(excess, 0, cap*sizeof(int));
    memset(height, 0, cap*sizeof(ST));
    memset(seen, 0, cap*sizeof(PEDGE));
    memset(v, -1, (len-2)*sizeof(PAIR));
    PEDGE edge;
    auto& nodes = graph.GetNodes();

    height[s]=len;
    excess[s]=(INT_MAX>>1); //ensures we can't blow stuff up while sending flow backwards

    ST root;
    ST* plast=&root;
    ST k=0; // Build a list of all vertices except s and t
    for(ST i=nodes.Front(); i!=(ST)-1; nodes.Next(i))
    {
      seen[k]=nodes[i].to;
      if(i!=s && i!=t) {
        *plast=k;
        v[k].first=i;
        plast=&v[k++].second;
      }
    }
    assert(k==(len-2));

    // Push as much flow as possible from s
    for(edge=nodes[s].to; edge!=0; edge=edge->next) {
      edge->data.flow=edge->data.capacity;
      excess[edge->to]=edge->data.capacity;
    }

    int send;
    ST last=-1;
    ST lh;
    ST target;
    for(ST c=root; c!=(ST)-1;)
    {
      k=v[c].first;
      lh=height[k];
      while(excess[k]>0) // Discharge current vertex
      {
        if(!seen[k]) //If seen is null, we tried all our neighbors so relabel
        {
          send=INT_MAX;
          for(edge=nodes[k].to; edge!=0; edge=edge->next) { // First we try to send it forward
            if(edge->data.capacity - edge->data.flow > 0) {
              if(height[edge->to]<send) send=height[edge->to];
              height[k] = send+1; // we have to do this in here because if there is no valid relabel, the height can't change.
            }
          }
          if(send==INT_MAX) // If this is true, our forward relabel failed, so that means we need to try to push things backwards
          {
            for(edge=nodes[k].from; edge!=0; edge=edge->alt.next) {
              if(edge->data.flow > 0) { // Negate the flow and set capacity to 0 because we're going backwards
                if(height[edge->from]<send) send=height[edge->from];
                height[k] = send+1;
              }
            }
            seen[k]=nodes[k].from;
          } else
            seen[k]=nodes[k].to;
          continue;
        }
        if(excess[k]>0) {// If possible, do a push. We must check whether our current edge is backwards or not
          send = (seen[k]->from==k)?seen[k]->data.capacity-seen[k]->data.flow:seen[k]->data.flow;
          target = (seen[k]->from==k)?seen[k]->to:seen[k]->from;
          if(height[k]>height[target] && send>0) { // If possible, do a push
            if(excess[k]<send) send=excess[k]; // send an amount equal to min(excess, capacity - flow)
            seen[k]->data.flow+=(seen[k]->from==k)?send:-send; // Negate if the edge is backwards
            excess[target]+=send;
            excess[k]-=send;
          }
        }
        seen[k]=(seen[k]->from==k)?seen[k]->next:seen[k]->alt.next;
      }
      if(lh!=height[k]) // If the height changed...
      {
        if(last!=(ST)-1)  // Move vertex to start of the list, if it's not there already
        {
          v[last].second=v[c].second;
          v[c].second=root;
          root=c;
        }
        c=root; // restart traversal at start of list
        continue;
      }
      last=c;
      c=v[c].second;
    }
  };

  template<typename V>
  struct BSS_COMPILER_DLLEXPORT __vertex_Demand {
    int demand;
    VoidData<V> d;
  };

  // Reduces a circulation problem to a maximum-flow graph in O(V) time. Returns nonzero if infeasible.
  template<typename E, typename V, typename ST, typename ALLOC, typename NODEALLOC, ARRAY_TYPE ArrayType>
  static int Circulation_MaxFlow(Graph<__edge_MaxFlow<E>, __vertex_Demand<V>, ST, ALLOC, NODEALLOC, ArrayType>& g, ST& s, ST& t)
  {
    int total=0; // The total demand must be 0

    int d;
    int ss=g.AddNode();
    int st=g.AddNode();
    auto& n = g.GetNodes();
    __edge_MaxFlow<E> edge ={ 0 };

    for(ST i=n.Front(); i!=(ST)-1; n.Next(i)) {
      if(i!=ss && i!=st) {
        d=n[i].data.demand;
        total+=d;
        if(d<0) {// This is a source
          edge.capacity=-d;
          g.AddEdge(ss, i, &edge);
        } else if(d>0) {// this is a sink
          edge.capacity=d;
          g.AddEdge(i, st, &edge);
        }
      }
    }
    s=ss;
    t=st;
    return total;
  }

  template<typename E>
  struct BSS_COMPILER_DLLEXPORT __edge_LowerBound {
    int lowerbound;
    VoidData<E> d;
  };

  // Reduces a lower-bound circulation problem to a basic circulation problem in O(E) time.
  template<typename E, typename V, typename ST, typename ALLOC, typename NODEALLOC, ARRAY_TYPE ArrayType>
  static void LowerBound_Circulation(Graph<__edge_MaxFlow<__edge_LowerBound<E>>, __vertex_Demand<V>, ST, ALLOC, NODEALLOC, ArrayType>& g)
  {
    Edge<__edge_MaxFlow<__edge_LowerBound<E>>, ST>* e;
    auto& n = g.GetNodes();
    int l;
    for(ST i=n.Front(); i!=(ST)-1; n.Next(i)) {
      for(e=n[i].to; e!=0; e=e->next) {
        l = e->data.d.data.lowerbound;
        e->data.capacity-=l;
        n[i].data.demand+=l;
        n[e->to].data.demand-=l;
        e->data.d.data.lowerbound=0;
      }
    }
  }
}

#endif