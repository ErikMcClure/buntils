// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_LAMBDA_STACK_H__BSS__ 
#define __C_LAMBDA_STACK_H__BSS__

#include "cBSS_Stack.h"
#include <functional>

namespace bss_util {
  /* Implements a lambda stack used for deferred function evaluation */
  template<class Fx, typename SizeType=unsigned int>
  class cLambdaStack : protected cBSS_Stack<std::function<Fx>,SizeType, cArraySafe<std::function<Fx>,SizeType>>
  {
  public:
    /* Copy constructor */
    inline cLambdaStack(const cBSS_Stack& copy) : cBSS_Stack(copy) {}
    inline cLambdaStack(cBSS_Stack&& mov) : cBSS_Stack(std::move(mov)) {}
    /* Constructor, takes an initial size for the stack */
    inline explicit cLambdaStack(int init=8) : cBSS_Stack(8) {}
    /* Destructor */
    inline ~cLambdaStack() {}
    /* Deferres a function for later execution */
    template<typename F>
    inline void DeferLambda(F lambda) { Push(lambda); }
    inline void Clear() { _length=0; }
    inline SizeType Length() const { return _length; }

    inline cLambdaStack& operator=(const cBSS_Stack& right) { cBSS_Stack::operator=(right); return *this; }
    inline cLambdaStack& operator=(cBSS_Stack&& mov) { cBSS_Stack::operator=(std::move(mov)); return *this; }

  protected:
    template<typename SizeType>
    static inline void EvalLambdaStack(cLambdaStack<void (void), SizeType>& lbstack) { while(lbstack.Length()) { lbstack.Pop()(); } }
  };
}

#endif