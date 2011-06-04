// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_LAMBDA_STACK_H__ 
#define __C_LAMBDA_STACK_H__

#include "cBSS_Stack.h"
#include <functional>

namespace bss_util {
  /* Implements a lambda stack used for deferred function evaluation */
  template<class Fx, typename SizeType=unsigned int>
  class cLambdaStack : protected cBSS_Stack<std::function<Fx>,ValueTraits<std::function<Fx>>,SizeType, cArrayConstruct<std::function<Fx>,SizeType>>
  {
  public:
    /* Copy constructor */
    inline cLambdaStack(const cBSS_Stack& copy) : cBSS_Stack(copy) {}
    /* Constructor, takes an initial size for the stack */
    inline explicit cLambdaStack(int init=8) : cBSS_Stack(8) {}
    /* Destructor */
    inline ~cLambdaStack() {}
    /* Deferres a lambda function for later execution */
    inline void DeferLambda(const std::function<Fx>& lambda) { Push(lambda); }

  protected:
    template<typename SizeType>
    static inline void EvalLambdaStack(cLambdaStack<void (void), SizeType>& lbstack) { while(lbstack.Length()) { lbstack.Pop()(); } }
  };
}

#endif