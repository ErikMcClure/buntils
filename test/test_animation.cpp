// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "cFXAni.h"
#include "variant.h"
#include <memory>

using namespace bss_util;

struct cAnimObj
{
  cAnimObj() : test(0), test2(0) {}
  int test;
  int test2;
  float fl;
  void BSS_FASTCALL donothing(cRefCounter*) { ++test; }
  cRefCounter* BSS_FASTCALL retnothing(ref_ptr<cRefCounter> p) {
    ++test2;
    p->Grab();
    return (cRefCounter*)p;
  }
  void BSS_FASTCALL remnothing(cRefCounter* p) { p->Drop(); }
  void BSS_FASTCALL setfloat(float a) { fl = a; }
  const float& BSS_FASTCALL getfloat() { return fl; }
};

typedef variant<double> FX_DEF;
typedef variant<DEBUG_CDT<true>, DEBUG_CDT<false>> FX_OBJ;
typedef variant<cAnimation<float>> FX_ANI;
typedef variant<cAniStateDiscrete<float>> FX_STATE;

double anidefcreate(const FX_DEF& def, FX_OBJ& obj)
{
  double d = def.get<double>();
  if(d >= 0.0)
    obj = DEBUG_CDT<true>();
  else
    obj = DEBUG_CDT<false>();
  return abs(d);
}

void anidefmap(FX_ANI& ani, FX_STATE& state, FX_OBJ& obj)
{
  if(ani.is<cAnimation<float>>())
  {
    delegate<void, float> d(0, 0);
    if(obj.is<DEBUG_CDT<true>>())
      d = delegate<void, float>::From<DEBUG_CDT<true>, &DEBUG_CDT<true>::donothing>(&obj.get<DEBUG_CDT<true>>());
    else
      d = delegate<void, float>::From<DEBUG_CDT<false>, &DEBUG_CDT<false>::donothing>(&obj.get<DEBUG_CDT<false>>());
    state = cAniStateDiscrete<float>(&ani.get<cAnimation<float>>(), d);
  }
}
TESTDEF::RETPAIR test_ANIMATION()
{
  BEGINTEST;
  RCounter c;
  {
    c.Grab();
    cAnimation<cRefCounter*> a0;
    a0.Add(0.0, &c);
    a0.Add(1.1, &c);
    a0.Add(2.0, &c);
    cAnimationInterval<ref_ptr<cRefCounter>> a1;
    a1.Add(0.0, &c, 1.5);
    a1.Add(1.0, &c, 0.5);
    a1.Add(1.5, &c, 0.5);
    cAnimation<float> a2(&cAniStateSmooth<float>::LerpInterpolate);
    a2.Add(0.0, 0.0f);
    a2.Add(1.0, 1.0f);
    a2.Add(2.0, 2.0f);

    TEST(a0.GetLength() == 2.0);
    TEST(a1.GetLength() == 2.0);
    TEST(a2.GetLength() == 2.0);

    a0.SetLoop(0.0);
    //std::stringstream ss;
    //a.Serialize(ss);

    //cAnimation<StaticAllocPolicy<char>> aa;
    //aa.Deserialize(ss);
    //for(int i = 0; i<6; ++i) c.Grab(); // compensate for the pointer we just copied over

    cAnimObj obj;
    //a.GetAttribute<3>()->AddKeyFrame(KeyFrame<3>(0.0, [&](){ c.Grab(); obj.test++; }));
    //a.GetAttribute<3>()->AddKeyFrame(KeyFrame<3>(0.6, [&](){ c.Drop(); }));
    //a.Attach(delegate<void,AniAttribute*>::From<cAnimObj,&cAnimObj::TypeIDRegFunc>(&obj));
    cAniStateDiscrete<cRefCounter*> s0(&a0, delegate<void, cRefCounter*>::From<cAnimObj, &cAnimObj::donothing>(&obj));
    cAniStateInterval<ref_ptr<cRefCounter>, cRefCounter*> s1(&a1, delegate<cRefCounter*, ref_ptr<cRefCounter>>::From<cAnimObj, &cAnimObj::retnothing>(&obj), delegate<void, cRefCounter*>::From<cAnimObj, &cAnimObj::remnothing>(&obj));
    cAniStateSmooth<float> s2(&a2, delegate<void, float>::From<cAnimObj, &cAnimObj::setfloat>(&obj));

    auto interall = [&](double t) {
      s0.Interpolate(t);
      s1.Interpolate(t);
      s2.Interpolate(t);
    };
    TEST(c.Grab() == 5);
    c.Drop();
    TEST(obj.test == 0);
    interall(0.0);
    TEST(obj.test == 1);
    TEST(obj.test2 == 1);
    interall(0.5);
    TEST(obj.test == 1);
    TEST(obj.test2 == 1);
    interall(0.5);
    TEST(obj.test == 1);
    TEST(obj.test2 == 2);
    interall(0.5);
    TEST(obj.test == 2);
    TEST(obj.test2 == 3);
    TEST(s0.GetTime() == 1.5);
    TEST(s1.GetTime() == 1.5);
    TEST(s2.GetTime() == 1.5);
    interall(0.5);
    TEST(s0.GetTime() == 0.0);
    TEST(s1.GetTime() == 2.0);
    TEST(s2.GetTime() == 2.0);
    TEST(obj.test == 4);
    interall(0.5);
    TEST(obj.test == 4);
    obj.test = 0;
    s0.Reset();
    s1.Reset();
    s2.Reset();
    TEST(s0.GetTime() == 0.0);
    TEST(s1.GetTime() == 0.0);
    TEST(s2.GetTime() == 0.0);
    interall(0.0);
    TEST(obj.test == 1);
    interall(0.6);
    obj.test = 0;
    s0.Reset();
    s1.Reset();
    s2.Reset();
    TEST(c.Grab() == 5);
    c.Drop();
    TEST(a0.GetLoop() == 0.0);
    TEST(a1.GetLoop() < 0.0);
    TEST(a2.GetLoop() < 0.0);
    a0.SetLoop(1.0);
    a1.SetLoop(1.0);
    a2.SetLoop(1.0);
    s0.SetTime(1.5);
    s1.SetTime(1.5);
    s2.SetTime(1.5);
    interall(0.0);
    TEST(a0.GetLoop() == 1.0);
    TEST(obj.test == 2);
    interall(3.0);
    s0.Reset();
    s1.Reset();
    s2.Reset();
    TEST(c.Grab() == 5);
    c.Drop();

    //{
    //  cAnimation<StaticAllocPolicy<char>> b(a);
    //  obj.test=0;
    //  b.Attach(delegate<void, AniAttribute*>::From<cAnimObj, &cAnimObj::TypeIDRegFunc>(&obj));
    //  TEST(obj.test==0);
    //  b.Start(0);
    //  TEST(obj.test==2);
    //  TEST(c.Grab()==21);
    //  c.Drop();
    //  b.Interpolate(10.0);
    //  TEST(c.Grab()==22);
    //  c.Drop();
    //}
  }
  TEST(c.Grab() == 2);

  ENDTEST;
}

TESTDEF::RETPAIR test_FX_ANI()
{
  BEGINTEST;
  DEBUG_CDT<true>::count = 0;

  {
    typedef cFXAni<FX_DEF, FX_OBJ, anidefcreate, FX_ANI, FX_STATE, anidefmap> FXANI;
    cAniFrame<float, void> frames[2] = { { -1.0f,0.0 }, { 0.0f,1.0 } };
    FXANI::FXMANAGER manager;
    FXANI _test(&manager);
    _test.AddDef(FX_DEF(1.0), 0.0, true);
    _test.AddDef(FX_DEF(-1.0), 3.0, true);
    _test.AddDef(FX_DEF(0.0), 0.0, false);
    _test.AddDef(FX_DEF(-2.0), 1.0, false);
    _test.AddAnimation<float, void>(frames, 2, 0);
    _test.AddMapping(0, 0, 0.0);
    _test.AddMapping(1, 0, 2.0);
    _test.AddMapping(2, 0, 1.0);
    _test.AddMapping(3, 0, 0.0);
    cFXAniState<FXANI> fxstate(&_test);
    fxstate.Interpolate(1);
    fxstate.Interpolate(5);
    fxstate.Interpolate(1);
  }
  ENDTEST;
}