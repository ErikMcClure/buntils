// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/FXAni.h"
#include "bss-util/Variant.h"
#include "test.h"
#include <memory>

using namespace bss;

struct cAnimObj
{
  cAnimObj() : test(0), test2(0) {}
  int test;
  int test2;
  float fl;
  void donothing(RefCounter*) { ++test; }
  RefCounter* retnothing(ref_ptr<RefCounter> p) {
    ++test2;
    p->Grab();
    return (RefCounter*)p;
  }
  void remnothing(RefCounter* p) { p->Drop(); }
  void setfloat(float a) { fl = a; }
  const float& getfloat() { return fl; }
};

typedef Variant<double> FX_DEF;
typedef Variant<DEBUG_CDT<true>, DEBUG_CDT<false>> FX_OBJ;
typedef Variant<Animation<float>> FX_ANI;
typedef Variant<AniStateDiscrete<float>> FX_STATE;

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
  if(ani.is<Animation<float>>())
  {
    Delegate<void, float> d(0, 0);
    if(obj.is<DEBUG_CDT<true>>())
      d = Delegate<void, float>::From<DEBUG_CDT<true>, &DEBUG_CDT<true>::donothing>(&obj.get<DEBUG_CDT<true>>());
    else
      d = Delegate<void, float>::From<DEBUG_CDT<false>, &DEBUG_CDT<false>::donothing>(&obj.get<DEBUG_CDT<false>>());
    state = AniStateDiscrete<float>(&ani.get<Animation<float>>(), d);
  }
}
TESTDEF::RETPAIR test_ANIMATION()
{
  BEGINTEST;
  RCounter c;
  {
    c.Grab();
    Animation<RefCounter*> a0;
    a0.Add(0.0, &c);
    a0.Add(1.1, &c);
    a0.Add(2.0, &c);
    AnimationInterval<ref_ptr<RefCounter>> a1;
    a1.Add(0.0, &c, 1.5);
    a1.Add(1.0, &c, 0.5);
    a1.Add(1.5, &c, 0.5);
    Animation<float> a2(&AniStateSmooth<float>::LerpInterpolate);
    a2.Add(0.0, 0.0f);
    a2.Add(1.0, 1.0f);
    a2.Add(2.0, 2.0f);

    TEST(a0.GetLength() == 2.0);
    TEST(a1.GetLength() == 2.0);
    TEST(a2.GetLength() == 2.0);

    a0.SetLoop(0.0);
    //std::stringstream ss;
    //a.Serialize(ss);

    //Animation<StaticAllocPolicy<char>> aa;
    //aa.Deserialize(ss);
    //for(size_t i = 0; i<6; ++i) c.Grab(); // compensate for the pointer we just copied over

    cAnimObj obj;
    //a.GetAttribute<3>()->AddKeyFrame(KeyFrame<3>(0.0, [&](){ c.Grab(); obj.test++; }));
    //a.GetAttribute<3>()->AddKeyFrame(KeyFrame<3>(0.6, [&](){ c.Drop(); }));
    //a.Attach(Delegate<void,AniAttribute*>::From<cAnimObj,&cAnimObj::TypeIDRegFunc>(&obj));
    AniStateDiscrete<RefCounter*> s0(&a0, Delegate<void, RefCounter*>::From<cAnimObj, &cAnimObj::donothing>(&obj));
    AniStateInterval<ref_ptr<RefCounter>, RefCounter*> s1(&a1, Delegate<RefCounter*, ref_ptr<RefCounter>>::From<cAnimObj, &cAnimObj::retnothing>(&obj), Delegate<void, RefCounter*>::From<cAnimObj, &cAnimObj::remnothing>(&obj));
    AniStateSmooth<float> s2(&a2, Delegate<void, float>::From<cAnimObj, &cAnimObj::setfloat>(&obj));

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
    //  Animation<StaticAllocPolicy<char>> b(a);
    //  obj.test=0;
    //  b.Attach(Delegate<void, AniAttribute*>::From<cAnimObj, &cAnimObj::TypeIDRegFunc>(&obj));
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
    typedef FXAni<FX_DEF, FX_OBJ, anidefcreate, FX_ANI, FX_STATE, anidefmap> FXANI;
    AniFrame<float, void> frames[2] = { { -1.0f,0.0 }, { 0.0f,1.0 } };
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
    FXAniState<FXANI> fxstate(&_test);
    fxstate.Interpolate(1);
    fxstate.Interpolate(5);
    fxstate.Interpolate(1);
  }
  ENDTEST;
}