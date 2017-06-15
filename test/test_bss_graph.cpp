// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/Graph.h"
#include "test.h"
#include <iostream>

using namespace bss;

bool GRAPHACTION(uint16_t s) { return false; }
bool GRAPHISEDGE(const internal::__edge_MaxFlow<internal::__edge_LowerBound<void>>* e) { return e->capacity>0; }

template<class G> // debug
void outgraph(const G& g)
{
  std::cout << "--------" << std::endl;
  auto& n = g.GetNodes();
  for(const typename G::N_& node : n) // test range-based for loops
  {
    for(auto p = node.to; p != 0; p = p->next)
      std::cout << p->from << " -> (" << p->data.capacity << "," << p->data.flow << ") ->" << p->to << std::endl;
  }
}

TESTDEF::RETPAIR test_bss_GRAPH()
{
  BEGINTEST;
  using internal::__edge_MaxFlow;
  using internal::__vertex_Demand;
  using internal::__edge_LowerBound;
  
  {
    Graph<void, void> g(6);

    uint16_t s = 0;
    uint16_t o = 1;
    uint16_t p = 2;
    uint16_t q = 3;
    uint16_t r = 4;
    uint16_t t = 5;
    g.AddEdge(s, o, 0);
    g.AddEdge(s, p, 0);
    g.AddEdge(o, p, 0);
    g.AddEdge(o, q, 0);
    g.AddEdge(p, r, 0);
    g.AddEdge(q, r, 0);
    g.AddEdge(q, t, 0);
    g.AddEdge(r, t, 0);
    TEST(g.NumEdges() == 8);
    TEST(g.NumNodes() == 6);

    uint16_t* queue = (uint16_t*)ALLOCA(sizeof(uint16_t)*g.NumNodes());
    BreadthFirstGraph<Graph<void, void>, GRAPHACTION>(g, s, queue);
    TEST(queue[0] == p);
    TEST(queue[1] == o);
    TEST(queue[2] == r);
    TEST(queue[3] == q);
    TEST(queue[4] == t);
  }

  {
    Graph<__edge_MaxFlow<void>, void> g;

    uint16_t s = g.AddNode();
    uint16_t o = g.AddNode();
    uint16_t p = g.AddNode();
    uint16_t q = g.AddNode();
    uint16_t r = g.AddNode();
    TEST(g.Back() == r);
    uint16_t t = g.AddNode();
    TEST(g.Front() == s);
    TEST(g.Back() == t);
    { __edge_MaxFlow<void> e = { 0,3 }; g.AddEdge(s, o, &e); }
    { __edge_MaxFlow<void> e = { 0,3 }; g.AddEdge(s, p, &e); }
    { __edge_MaxFlow<void> e = { 0,2 }; g.AddEdge(o, p, &e); }
    { __edge_MaxFlow<void> e = { 0,3 }; g.AddEdge(o, q, &e); }
    { __edge_MaxFlow<void> e = { 0,2 }; g.AddEdge(p, r, &e); }
    { __edge_MaxFlow<void> e = { 0,4 }; g.AddEdge(q, r, &e); }
    { __edge_MaxFlow<void> e = { 0,2 }; g.AddEdge(q, t, &e); }
    { __edge_MaxFlow<void> e = { 0,3 }; g.AddEdge(r, t, &e); }

    MaxFlow_PushRelabel(g, s, t);
    int res[] = { g[s].to->data.flow, g[s].to->next->data.flow, g[o].to->data.flow, g[o].to->next->data.flow,
      g[p].to->data.flow, g[q].to->data.flow, g[q].to->next->data.flow, g[r].to->data.flow };
    int resref[] = { 2,3,3,0,2,2,1,3 };
    for(size_t i = 0; i < sizeof(res) / sizeof(int); ++i) TEST(res[i] == resref[i]);
  }

  {
    __vertex_Demand<void> verts[4] = { { -3 }, { -3 }, { 2 }, { 4 } };
    Graph<__edge_MaxFlow<void>, __vertex_Demand<void>> g(4, verts);
    { __edge_MaxFlow<void> e = { 0, 3 }; g.AddEdge(0, 2, &e); } // Note, this problem has two valid answers. If the edges are added in 
    { __edge_MaxFlow<void> e = { 0, 3 }; g.AddEdge(0, 1, &e); } // the opposite order, you'll get a different one.
    { __edge_MaxFlow<void> e = { 0, 2 }; g.AddEdge(1, 3, &e); }
    { __edge_MaxFlow<void> e = { 0, 2 }; g.AddEdge(1, 2, &e); }
    { __edge_MaxFlow<void> e = { 0, 2 }; g.AddEdge(2, 3, &e); }

    uint16_t s;
    uint16_t t;
    Circulation_MaxFlow(g, s, t);
    MaxFlow_PushRelabel(g, s, t);
    //outgraph(g);
    g.RemoveNode(s);
    g.RemoveNode(t);
    int res[] = { g[0].to->data.flow, g[0].to->next->data.flow, g[1].to->data.flow, g[1].to->next->data.flow, g[2].to->data.flow };
    int resref[] = { 1,2,2,2,2 };
    for(size_t i = 0; i < sizeof(res) / sizeof(int); ++i) TEST(res[i] == resref[i]);
  }

  {
    typedef __edge_MaxFlow<__edge_LowerBound<void>> EDGE;
    EDGE m[16] = { { 0,0,0 }, { 0,4,1 }, { 0,5,2 }, { 0,0,0 },
    { 0,0,0 }, { 0,0,0 }, { 0,2,0 }, { 0,2,0 },
    { 0,0,0 }, { 0,0,0 }, { 0,0,0 }, { 0,2,0 },
    { 0,0,0 }, { 0,0,0 }, { 0,0,0 }, { 0,0,0 } };
    __vertex_Demand<void> verts[4] = { { -6 }, { -2 }, { 4 }, { 4 } };
    Graph<__edge_MaxFlow<__edge_LowerBound<void>>, __vertex_Demand<void>> g;
    g.FromMatrix<GRAPHISEDGE>(4, m, verts);

    uint16_t s;
    uint16_t t;
    LowerBound_Circulation(g);
    Circulation_MaxFlow(g, s, t);
    MaxFlow_PushRelabel(g, s, t);
    g.RemoveNode(s);
    g.RemoveNode(t);
    int res[] = { g[0].to->data.flow, g[0].to->next->data.flow, g[1].to->data.flow, g[1].to->next->data.flow, g[2].to->data.flow };
    int resref[] = { 3,0,2,1,2 }; // This adds the edges in a different order so we get the other legal answer
    for(size_t i = 0; i < sizeof(res) / sizeof(int); ++i) TEST(res[i] == resref[i]);
  }

  {
    char m[16] = { 0,1,0,0,
      0,0,1,0,
      1,1,0,1,
      0,1,0,0, };
    Graph<void, void> g(4, m, 0);

    char m2[16] = { 0 };
    g.ToMatrix(m2, 0);
    TEST(!memcmp(m, m2, sizeof(m)));
  }
  ENDTEST;
}