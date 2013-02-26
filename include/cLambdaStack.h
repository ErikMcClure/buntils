// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_LAMBDA_STACK_H__BSS__ 
#define __C_LAMBDA_STACK_H__BSS__

#include "cBSS_Stack.h"
#include <functional>

namespace bss_util {
  // Implements a lambda stack used for deferred function evaluation
  template<class Fx, typename SizeType=unsigned int>
  class cLambdaStack : protected cBSS_Stack<std::function<Fx>,SizeType, cArraySafe<std::function<Fx>,SizeType>>
  {
    typedef cBSS_Stack<std::function<Fx>,SizeType, cArraySafe<std::function<Fx>,SizeType>> BASE_STACK;
    using BASE_STACK::_length;

  public:
    // Copy constructor
    inline cLambdaStack(const BASE_STACK& copy) : BASE_STACK(copy) {}
    inline cLambdaStack(BASE_STACK&& mov) : BASE_STACK(std::move(mov)) {}
    // Constructor, takes an initial size for the stack
    inline explicit cLambdaStack(int init=8) : BASE_STACK(8) {}
    // Destructor
    inline ~cLambdaStack() {}
    // Deferres a function for later execution
    template<typename F>
    inline void DeferLambda(F lambda) { BASE_STACK::Push(lambda); }
    inline void Clear() { _length=0; }
    inline SizeType Length() const { return _length; }

    inline cLambdaStack& operator=(const BASE_STACK& right) { BASE_STACK::operator=(right); return *this; }
    inline cLambdaStack& operator=(BASE_STACK&& mov) { BASE_STACK::operator=(std::move(mov)); return *this; }

  protected:
    template<typename ST>
    static inline void EvalLambdaStack(cLambdaStack<void (void), ST>& lbstack) { while(lbstack.Length()) { lbstack.Pop()(); } }
  };
}

#endif
