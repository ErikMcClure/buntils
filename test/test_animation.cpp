// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "test.h"
#include "bss-util/Animation.h"
#include "bss-util/Variant.h"
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

TESTDEF::RETPAIR test_ANIMATION()
{
  BEGINTEST;
  RCounter c;
  {
    using PtrAni = AniData<RefCounter*, void, ARRAY_SAFE>;
    using RefAni = AniDataInterval<ref_ptr<RefCounter>, ARRAY_SAFE>;
    using FloatAni = AniDataSmooth<float>;

    c.Grab();
    Animation<PtrAni> a0;
    a0.Add<PtrAni>(0.0, &c);
    a0.Add<PtrAni>(1.1, &c);
    a0.Add<PtrAni>(2.0, &c);
    Animation<RefAni> a1;
    a1.Get<RefAni>().Add(0.0, &c, 1.5);
    a1.Get<RefAni>().Add(1.0, &c, 0.5);
    a1.Get<RefAni>().Add(1.5, &c, 0.5);
    a1.SetLength(); // We can't use the helper functions here so recalculate length
    Animation<FloatAni> a2;
    a2.Add<FloatAni>(0.0, 0.0f);
    a2.Add<FloatAni>(1.0, 1.0f);
    a2.Add<FloatAni>(2.0, 2.0f);
    a2.Get<FloatAni>().SetInterpolation(&FloatAni::LerpInterpolate);
    TEST(a0.GetLength() == 2.0);
    TEST(a1.GetLength() == 2.0);
    TEST(a2.GetLength() == 2.0);

    a0.SetLoop(0.0);
    //std::stringstream ss;
    //a.Serialize(ss);

    //Animation<StandardAllocator<char>> aa;
    //aa.Deserialize(ss);
    //for(size_t i = 0; i<6; ++i) c.Grab(); // compensate for the pointer we just copied over

    cAnimObj obj;
    //a.GetAttribute<3>()->AddKeyFrame(KeyFrame<3>(0.0, [&](){ c.Grab(); obj.test++; }));
    //a.GetAttribute<3>()->AddKeyFrame(KeyFrame<3>(0.6, [&](){ c.Drop(); }));
    //a.Attach(Delegate<void,AniAttribute*>::From<cAnimObj,&cAnimObj::TypeIDRegFunc>(&obj));
    AniState<cAnimObj, Animation<PtrAni>, AniStateDiscrete<cAnimObj, PtrAni, RefCounter*, &cAnimObj::donothing>> s0(&obj, &a0);
    AniState<cAnimObj, Animation<RefAni>, AniStateInterval<cAnimObj, RefAni, ref_ptr<RefCounter>, RefCounter*, &cAnimObj::retnothing, &cAnimObj::remnothing>> s1(&obj, &a1);
    AniState<cAnimObj, Animation<FloatAni>, AniStateSmooth<cAnimObj, FloatAni, float, &cAnimObj::setfloat>> s2(&obj, &a2);

    s2.SetValues(std::tuple<float>{ 15.0f });
    auto v0 = s0.GetValues();
    auto v1 = s1.GetValues();
    auto v2 = s2.GetValues();
    static_assert(std::is_same<decltype(std::make_tuple()), decltype(v0)>::value, "Wrong VALUE tuple");
    static_assert(std::is_same<decltype(std::make_tuple()), decltype(v1)>::value, "Wrong VALUE tuple");
    static_assert(std::is_same<std::tuple<float>, decltype(v2)>::value, "Wrong VALUE tuple");
    TEST(std::get<0>(v2) == 15.0f);

    static_assert(Animation<PtrAni>::STATESIZE == sizeof(s0), "STATESIZE should be sizeof(s0)");
    static_assert(Animation<RefAni>::STATESIZE == sizeof(s1), "STATESIZE should be sizeof(s1)");
    static_assert(Animation<FloatAni>::STATESIZE == sizeof(s2), "STATESIZE should be sizeof(s2)");

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
    //  Animation<StandardAllocator<char>> b(a);
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