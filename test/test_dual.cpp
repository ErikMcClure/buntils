// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "test.h"
#include "buntils/Dual.h"

using namespace bun;

BUN_FORCEINLINE bool fsmallcomp(double af, double bf, int64_t maxDiff = 1)
{
  if(af == 0.0)
    return fSmall(bf);
  if(bf == 0.0)
    return fSmall(af);
  return fCompare(af, bf, maxDiff);
}

TESTDEF::RETPAIR test_DUAL()
{
  BEGINTEST;
  // d/dx( e^(x + x^2) + 2x/5 + cos(x*x) + sin(ln(x)+log(x)) ) = e^(x^2+x) (2 x+1)-2 x sin(x^2)+((1+log(10)) cos((1+1/(log(10))) log(x)))/(x log(10))+2/5

  for(int i = -10; i < 10; ++i)
  {
    Dual<double, 2> dx(i, 1); //declare our variable
    Dual<double, 2> adx(abs(i), 1);
    //auto ax = (exp(dx + (dx^2)) + (2*dx)/5 + cos(abs(x)^abs(x)) + sin(log(dx)+log10(dx)));
    double id = i;
    auto ax = (dx + (dx ^ 2)).exp() + (2.0*dx) / 5.0;
    TEST(fsmallcomp(ax(0), exp((i*i) + id) + (2.0*i / 5.0)));
    TEST(fsmallcomp(ax(1), exp((i*i) + id)*(2 * i + 1) + 2.0 / 5.0));
    TEST(fsmallcomp(ax(2), exp((i*i) + id)*(4 * i*i + 4 * i + 3)));

    if(!i) continue; //the functions below can't handle zeros.
    id = abs(i);
    ax = (adx^adx).cos();
    TEST(fsmallcomp(ax(0), cos(std::pow(id, id))));
    TEST(fsmallcomp(ax(1), sin(std::pow(id, id))*(-std::pow(id, id))*(log(id) + 1)));
    //TEST(ax(2)==(cos(std::pow(id,id))));

    ax = (adx.log() + adx.log10()).sin();
    TEST(fsmallcomp(ax(0), sin(log(id) + log10(id))));
    TEST(fsmallcomp(ax(1), ((1 + log(10.0))*cos((1 + (1 / (log(10.0))))*log(id))) / (id*log(10.0)), 5));
    //TEST(ax(2)==(-((1+log(10.0))*((1+log(10.0))*sin((1+1/(log(10.0))) log(id))+log(10.0)*cos((1+(1/(log(10.0))))*log(id))))/((id*id)*(log(10.0)*log(10.0)))));
  }

  ENDTEST;
}
