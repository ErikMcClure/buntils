// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BUN_GRAPH_H__
#define __BUN_GRAPH_H__

#include "LinkedArray.h"
#include "BlockAlloc.h"
#include "DisjointSet.h"
#include "LLBase.h"
#include <limits.h>
#ifdef BUN_COMPILER_GCC
#include <alloca.h>
#endif

namespace bun {
  namespace internal {
    template<typename D>
    struct BUN_COMPILER_DLLEXPORT VoidData { D data; };
    template<>
    struct BUN_COMPILER_DLLEXPORT VoidData<void> {};
  }

  // DAG edge
  template<typename E, typename CT = uint16_t>
  struct BUN_COMPILER_DLLEXPORT Edge : LLBase<Edge<E, CT>>, internal::VoidData<E> { CT to; CT from; LLBase<Edge<E, CT>> alt; };

  // Node for a DAG.
  template<typename E, typename V, typename CT = uint16_t>
  struct BUN_COMPILER_DLLEXPORT Node : internal::VoidData<V> { Edge<E, CT>* to; Edge<E, CT>* from; };

  namespace internal {
    template<typename E> struct __Graph__InternalEdge {
      static BUN_FORCEINLINE void _getData(E& target, const VoidData<E>& t) { target = t.data; }
      static BUN_FORCEINLINE void _setDataE(VoidData<E>& t, const E* d) { t.data = *d; }
    };
    template<> struct __Graph__InternalEdge<void> {
      static BUN_FORCEINLINE void _getData(char& target, const VoidData<void>&) { target = 1; }
      static BUN_FORCEINLINE void _setDataE(VoidData<void>&, const void*) {}
    };

    template<typename V, typename CT>
    struct __Graph__InternalVertex {
      static BUN_FORCEINLINE void _setDataV(VoidData<V>& t, const V* d) { t.data = *d; }
      static BUN_FORCEINLINE void _setVertex(V* v, CT i, const VoidData<V>& t) { v[i] = t.data; }
      static BUN_FORCEINLINE const V* _addVertex(const V* v, CT i) { return (v + i); }
    };
    template<typename CT>
    struct __Graph__InternalVertex<void, CT> {
      static BUN_FORCEINLINE void _setDataV(VoidData<void>&, const void*) {}
      static BUN_FORCEINLINE void _setVertex(void*, CT, const VoidData<void>&) {}
      static BUN_FORCEINLINE const void* _addVertex(const void*, CT) { return 0; }
    };
  }

  // Represents a graph using an adjacency list. Converts to and from an adjacency matrix representation.
  template<typename E, typename V, typename CT = uint16_t, typename Alloc = StandardAllocator<Edge<E, CT>>, typename NODEALLOC = StandardAllocator<internal::LINKEDNODE<Node<E, V, CT>, CT>>>
  class Graph : public Alloc, protected internal::__Graph__InternalEdge<E>, protected internal::__Graph__InternalVertex<V, CT>
  {
    static BUN_FORCEINLINE bool _baseCheck(const char* b) { return (*b) != 0; }
    static BUN_FORCEINLINE LLBase<Edge<E, CT>>& _altGet(Edge<E, CT>* p) { return p->alt; }
    using EDATA = typename std::conditional<std::is_void<E>::value, char, E>::type;
    using internal::__Graph__InternalVertex<V, CT>::_setDataV;
    using internal::__Graph__InternalVertex<V, CT>::_setVertex;
    using internal::__Graph__InternalVertex<V, CT>::_addVertex;
    using internal::__Graph__InternalEdge<E>::_setDataE;
    using internal::__Graph__InternalEdge<E>::_getData;

    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;
  public:
    inline Graph(Graph&& mov) : Alloc(std::move(mov)), _nodes(std::move(mov._nodes)), _nedges(mov._nedges) { mov._nedges = 0; }
    inline explicit Graph(const Alloc& alloc) : Alloc(alloc), _nodes(0), _nedges(0) {}
    inline Graph() requires std::is_default_constructible_v<Alloc> : _nodes(0), _nedges(0) {}
    // Build a matrix out of a standard adjacency matrix
    inline Graph(CT n, const char* M, const V* nodes) requires std::is_default_constructible_v<Alloc> : _nodes(n), _nedges(0) { _construct<char, _baseCheck>(n, M, nodes); }
    // Build a matrix out of only a list of nodes
    inline Graph(CT n, const V* nodes)  requires std::is_default_constructible_v<Alloc> : _nodes(n), _nedges(0) { for (CT i = 0; i < n; ++i) AddNode(nodes + i); }
    inline explicit Graph(CT n) requires std::is_default_constructible_v<Alloc> : _nodes(n), _nedges(0) { for (CT i = 0; i < n; ++i) AddNode(); }
    inline ~Graph() { for (CT i = _nodes.Front(); i != (CT)-1; _nodes.Next(i)) while (_nodes[i].to) RemoveEdge(_nodes[i].to); }
    inline CT NumNodes() const { return _nodes.Length(); }
    inline CT NumEdges() const { return _nedges; }
    inline CT Capacity() const { return _nodes.Capacity(); }
    inline Node<E, V, CT>* GetNode(CT index) const { return index < _nodes.Length() ? &_nodes[index] : 0; }
    inline const LinkedArray<Node<E, V, CT>, CT, NODEALLOC>& GetNodes() const { return _nodes; }
    inline LinkedArray<Node<E, V, CT>, CT, NODEALLOC>& GetNodes() { return _nodes; }
    inline CT Front() const { return _nodes.Front(); }
    inline CT Back() const { return _nodes.Back(); }
    inline typename LinkedArray<Node<E, V, CT>, CT, NODEALLOC>::template cLAIter<true> begin() const { return _nodes.begin(); }
    inline typename LinkedArray<Node<E, V, CT>, CT, NODEALLOC>::template cLAIter<true> end() const { return _nodes.end(); }
    inline typename LinkedArray<Node<E, V, CT>, CT, NODEALLOC>::template cLAIter<false> begin() { return _nodes.begin(); }
    inline typename LinkedArray<Node<E, V, CT>, CT, NODEALLOC>::template cLAIter<false> end() { return _nodes.end(); }
    inline CT AddNode() { Node<E, V, CT> aux; bun_Fill(aux, 0); return _nodes.Add(aux); }
    inline CT AddNode(const V* node) { if (!node) return AddNode(); Node<E, V, CT> aux; aux.to = 0; aux.from = 0; _setDataV(aux, node); return _nodes.Add(aux); }
    inline Edge<E, CT>* AddEdge(CT from, CT to)
    {
      Edge<E, CT>* r = std::allocator_traits<Alloc>::allocate(*this, 1);
      r->to = to;
      r->from = from;
      LLAdd(r, _nodes[from].to);
      AltLLAdd<Edge<E, CT>, &Graph::_altGet>(r, _nodes[to].from);
      ++_nedges;
      return r;
    }
    inline Edge<E, CT>* AddEdge(CT from, CT to, const E* edge)
    {
      Edge<E, CT>* r = AddEdge(from, to);
      _setDataE(*r, edge);
      return r;
    }
    inline void RemoveNode(CT index)
    {
      auto& n = _nodes[index];
      while (n.to) RemoveEdge(n.to);
      while (n.from) RemoveEdge(n.from);
      _nodes.Remove(index);
    }
    inline void RemoveEdge(Edge<E, CT>* edge)
    {
      LLRemove(edge, _nodes[edge->from].to);
      AltLLRemove<Edge<E, CT>, &Graph::_altGet>(edge, _nodes[edge->to].from);
      std::allocator_traits<Alloc>::deallocate(*this, edge, 1);
      --_nedges;
    }
    template<bool ISEDGE(const E*)>
    void FromMatrix(CT n, const E* matrix, const V* nodes) { _construct<E, ISEDGE>(n, matrix, nodes); }
    CT ToMatrix(EDATA* matrix, V* nodes) const
    {
      CT len = _nodes.Length();
      Edge<E, CT>* cur = 0;

      if (!matrix)
        return len;

      VARARRAY(CT, hash, _nodes.Capacity());
      CT k = 0;

      for (CT i = _nodes.Front(); i != (CT)-1; _nodes.Next(i))
        hash[i] = k++; // Build up a hash mapping the IDs to monotonically increasing integers.

      for (CT i = _nodes.Front(); i != (CT)-1; _nodes.Next(i))
      {
        _setVertex(nodes, i, _nodes[i]);

        for (cur = _nodes[i].to; cur != 0; cur = cur->next)
          _getData(matrix[(hash[i] * len) + hash[cur->to]], *cur);
      }
      return len;
    }

    inline const Node<E, V, CT>& operator[](CT index) const { return _nodes[index]; }
    inline Node<E, V, CT>& operator[](CT index) { return _nodes[index]; }
    inline Graph& operator=(Graph&& mov)
    {
      Alloc::operator=(std::move(mov));
      _nodes = std::move(mov._nodes);
      _nedges = mov._nedges;
      mov._nedges = 0;
      return *this;
    }
    using CT_ = CT;
    using E_ = E;
    using V_ = V;
    using N_ = Node<E, V, CT>;
    using EDGE_ = Edge<E, CT>;

  protected:
    template<typename D, bool(*ISEDGE)(const D*)>
    void _construct(CT n, const D* M, const V* nodes)
    {
      VARARRAY(CT, k, n);

      for (CT i = 0; i < n; ++i)
        k[i] = AddNode(_addVertex(nodes, i));

      for (CT i = 0; i < n; ++i)
      {
        for (CT j = 0; j < n; ++j)
          if (ISEDGE(M + (i * n) + j))
            AddEdge(k[i], k[j], M + ((i * n) + j));
      }
    }

    LinkedArray<Node<E, V, CT>, CT, NODEALLOC> _nodes;
    CT _nedges;
  };

  namespace internal {
    template<typename E>
    struct BUN_COMPILER_DLLEXPORT __edge_MaxFlow {
      int flow;
      int capacity;
      VoidData<E> d;
    };
  }

  // Implementation of the FIFO push-relabel algorithm for solving a maximum-flow graph in O(V^3) time
  template<typename E, typename V, typename CT, typename ALLOC, typename NODEALLOC>
  inline void MaxFlow_PushRelabel(Graph<internal::__edge_MaxFlow<E>, V, CT, ALLOC, NODEALLOC>& graph, CT s, CT t)
  {
    using PEDGE = Edge<internal::__edge_MaxFlow<E>, CT>*;
    using PAIR = std::pair<CT, CT>;
    CT len = graph.NumNodes(); // Setup
    CT cap = graph.Capacity();
    VARARRAY(int, excess, cap);
    VARARRAY(CT, height, cap);
    VARARRAY(PEDGE, seen, cap);
    VARARRAY(PAIR, v, (len - 2)); //integer based singly-linked list.
    bun_FillN<int>(excess, cap);
    bun_FillN<CT>(height, cap);
    bun_FillN<PEDGE>(seen, cap);
    bun_FillN<PAIR>(v, (len - 2), -1);
    PEDGE edge;
    auto& nodes = graph.GetNodes();

    height[s] = len;
    excess[s] = (INT_MAX >> 1); //ensures we can't blow stuff up while sending flow backwards

    CT root;
    CT* plast = &root;
    CT k = 0; // Build a list of all vertices except s and t

    for (CT i = nodes.Front(); i != (CT)-1; nodes.Next(i))
    {
      seen[k] = nodes[i].to;

      if (i != s && i != t)
      {
        *plast = k;
        v[k].first = i;
        plast = &v[k++].second;
      }
    }

    assert(k == (len - 2));

    // Push as much flow as possible from s
    for (edge = nodes[s].to; edge != 0; edge = edge->next)
    {
      edge->data.flow = edge->data.capacity;
      excess[edge->to] = edge->data.capacity;
    }

    int send;
    CT last = -1;
    CT lh;
    CT target;

    for (CT c = root; c != (CT)-1;)
    {
      k = v[c].first;
      lh = height[k];

      while (excess[k] > 0) // Discharge current vertex
      {
        if (!seen[k]) //If seen is null, we tried all our neighbors so relabel
        {
          send = INT_MAX;

          for (edge = nodes[k].to; edge != 0; edge = edge->next)
          { // First we try to send it forward
            if (edge->data.capacity - edge->data.flow > 0)
            {
              if (height[edge->to] < send) send = height[edge->to];
              height[k] = send + 1; // we have to do this in here because if there is no valid relabel, the height can't change.
            }
          }

          if (send == INT_MAX) // If this is true, our forward relabel failed, so that means we need to try to push things backwards
          {
            for (edge = nodes[k].from; edge != 0; edge = edge->alt.next)
            {
              if (edge->data.flow > 0)
              { // Negate the flow and set capacity to 0 because we're going backwards
                if (height[edge->from] < send)
                  send = height[edge->from];

                height[k] = send + 1;
              }
            }
            seen[k] = nodes[k].from;
          }
          else
            seen[k] = nodes[k].to;

          continue;
        }
        if (excess[k] > 0)
        {// If possible, do a push. We must check whether our current edge is backwards or not
          send = (seen[k]->from == k) ? seen[k]->data.capacity - seen[k]->data.flow : seen[k]->data.flow;
          target = (seen[k]->from == k) ? seen[k]->to : seen[k]->from;

          if (height[k] > height[target] && send > 0)
          { // If possible, do a push
            if (excess[k] < send)
              send = excess[k]; // send an amount equal to min(excess, capacity - flow)

            seen[k]->data.flow += (seen[k]->from == k) ? send : -send; // Negate if the edge is backwards
            excess[target] += send;
            excess[k] -= send;
          }
        }
        seen[k] = (seen[k]->from == k) ? seen[k]->next : seen[k]->alt.next;
      }

      if (lh != height[k]) // If the height changed...
      {
        if (last != (CT)-1)  // Move vertex to start of the list, if it's not there already
        {
          v[last].second = v[c].second;
          v[c].second = root;
          root = c;
        }

        c = root; // restart traversal at start of list
        continue;
      }

      last = c;
      c = v[c].second;
    }
  };

  namespace internal {
    template<typename V>
    struct BUN_COMPILER_DLLEXPORT __vertex_Demand {
      int demand;
      VoidData<V> d;
    };
  }

  // Reduces a circulation problem to a maximum-flow graph in O(V) time. Returns nonzero if infeasible.
  template<typename E, typename V, typename CT, typename ALLOC, typename NODEALLOC>
  inline int Circulation_MaxFlow(Graph<internal::__edge_MaxFlow<E>, internal::__vertex_Demand<V>, CT, ALLOC, NODEALLOC>& g, CT& s, CT& t)
  {
    int total = 0; // The total demand must be 0

    int d;
    int ss = g.AddNode();
    int st = g.AddNode();
    auto& n = g.GetNodes();
    internal::__edge_MaxFlow<E> edge = { 0 };

    for (CT i = n.Front(); i != (CT)-1; n.Next(i))
    {
      if (i != ss && i != st)
      {
        d = n[i].data.demand;
        total += d;

        if (d < 0)
        {// This is a source
          edge.capacity = -d;
          g.AddEdge(ss, i, &edge);
        }
        else if (d > 0)
        {// this is a sink
          edge.capacity = d;
          g.AddEdge(i, st, &edge);
        }
      }
    }

    s = ss;
    t = st;
    return total;
  }

  namespace internal {
    template<typename E>
    struct BUN_COMPILER_DLLEXPORT __edge_LowerBound {
      int lowerbound;
      VoidData<E> d;
    };
  }

  // Reduces a lower-bound circulation problem to a basic circulation problem in O(E) time.
  template<typename E, typename V, typename CT, typename ALLOC, typename NODEALLOC>
  inline void LowerBound_Circulation(Graph<internal::__edge_MaxFlow<internal::__edge_LowerBound<E>>, internal::__vertex_Demand<V>, CT, ALLOC, NODEALLOC>& g)
  {
    Edge<internal::__edge_MaxFlow<internal::__edge_LowerBound<E>>, CT>* e;
    auto& n = g.GetNodes();
    int l;
    for (CT i = n.Front(); i != (CT)-1; n.Next(i))
    {
      for (e = n[i].to; e != 0; e = e->next)
      {
        l = e->data.d.data.lowerbound;
        e->data.capacity -= l;
        n[i].data.demand += l;
        n[e->to].data.demand -= l;
        e->data.d.data.lowerbound = 0;
      }
    }
  }


  // Breadth-first search for any directed graph. If FACTION returns true, terminates.
  template<typename G, bool(*FACTION)(typename G::CT_)>
  inline void BreadthFirstGraph(G& graph, typename G::CT_ root)
  {
    VARARRAY(typename G::CT_, queue, graph.NumNodes());
    BreadthFirstGraph<G, FACTION>(graph, root, queue);
  }

  // Breadth-first search for any directed graph. If FACTION returns true, terminates. queue must point to an array at least GetNodes() long.
  template<typename G, bool(*FACTION)(typename G::CT_)>
  inline void BreadthFirstGraph(G& graph, typename G::CT_ root, typename G::CT_* queue)
  {
    using CT = typename G::CT_;
    using SST = typename std::make_signed<CT>::type;
    using E = Edge<typename G::E_, CT>;
    auto& n = graph.GetNodes();

    if ((*FACTION)(root))
      return;

    VARARRAY(CT, aset, graph.Capacity());
    DisjointSet<CT, NullAllocator<SST>> set(reinterpret_cast<SST*>((CT*)aset), graph.Capacity());

    // Queue up everything next to the root, checking only for edges that connect the root to itself
    size_t l = 0;
    for (E* edge = n[root].to; edge != 0; edge = edge->next)
    {
      if (edge->to != root)
        queue[l++] = edge->to;
    }

    for (size_t i = 0; i != l; ++i) // Go through the queue
    {
      if (FACTION(queue[i]))
        return;

      set.Union(root, queue[i]);

      for (E* edge = n[queue[i]].to; edge != 0; edge = edge->next)
      {
        if (set.Find(edge->to) != root) //Enqueue the children if they aren't already in the set.
          queue[l++] = edge->to; // Doesn't need to be circular because we can only enqueue n-1 anyway.
      }
    }
  }
}

#endif