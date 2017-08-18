// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __DELEGATE_H__BSS__
#define __DELEGATE_H__BSS__

#include "bss_util.h"
#include <functional>

namespace bss {
  // Delegate class using variadic templates
  template<typename R, typename... Args>
  class BSS_COMPILER_DLLEXPORT Delegate
  {
    inline Delegate(std::function<R(Args...)>&& src) = delete; // Don't do Delegate([&](){ return; }) or it'll go out of scope.
      typedef R RTYPE;
    typedef R(*FUNCTYPE)(void*, Args...);
    typedef RTYPE(*COREFUNC)(Args...);
  public:
    inline Delegate(const Delegate& copy) noexcept : _src(copy._src), _stub(copy._stub) {}
    inline Delegate(void* src, R(*stub)(void*, Args...)) noexcept : _src(src), _stub(stub) {}
    inline Delegate(std::function<R(Args...)>& src) noexcept : _src(&src), _stub(&stublambda) {}
    inline Delegate() noexcept : _src(0), _stub(0) {}
    inline R operator()(Args ... args) const { return (*_stub)(_src, args...); }
    inline Delegate& operator=(const Delegate& right) noexcept { _src = right._src; _stub = right._stub; return *this; }
    inline bool IsEmpty() const noexcept { return _src == 0 || _stub == 0; }
    inline void* RawSource() const { return _src; }
    inline FUNCTYPE RawFunc() const { return _stub; }

    template<class T, RTYPE(T::*F)(Args...)>
    inline static Delegate From(T* src) noexcept { return Delegate(src, &stub<T, F>); }
    template<class T, RTYPE(T::*F)(Args...) const>
    inline static Delegate From(const T* src) noexcept { return Delegate(const_cast<T*>(src), &stubconst<T, F>); }
    template<RTYPE(*F)(Args...)>
    inline static Delegate FromC() noexcept { return Delegate(0, &stubstateless<F>); }
    template<class T, RTYPE(*F)(T*, Args...)>
    inline static Delegate FromC(T* src) noexcept { return Delegate((void*)src, (FUNCTYPE)&stubC<T, F>); }
    template<class T, RTYPE(*F)(const T*, Args...)>
    inline static Delegate FromC(const T* src) noexcept { return Delegate(const_cast<T*>(src), &stubconstC<T, F>); }

    template <class T, RTYPE(T::*F)(Args...)>
    static R stub(void* src, Args... args) { return (static_cast<T*>(src)->*F)(args...); }
    template <class T, RTYPE(T::*F)(Args...) const>
    static R stubconst(void* src, Args... args) { return (static_cast<const T*>(src)->*F)(args...); }
    static R stublambda(void* src, Args... args) { return (*static_cast<std::function<R(Args...)>*>(src))(args...); }
    template <RTYPE(*F)(Args...)>
    static R stubstateless(void* src, Args... args) { return (*F)(args...); }
    template <class T, RTYPE(*F)(T*, Args...)>
    static R stubC(void* src, Args... args) { return (*F)(static_cast<T*>(src), args...); }
    template <class T, RTYPE(*F)(const T*, Args...)>
    static R stubconstC(void* src, Args... args) { return (*F)(static_cast<const T*>(src), args...); }
    static R stubembed(void* src, Args... args) { return ((COREFUNC)src)(args...); }

  protected:
    void* _src;
    R(*_stub)(void*, Args...);
  };

  template<typename R, typename... Args>
  struct StoreFunction : std::tuple<Args...>
  {
    inline StoreFunction(R(*f)(Args...), Args&&... args) : std::tuple<Args...>(std::forward<Args>(args)...), _f(f) {}
    BSS_FORCEINLINE R Call() const { return _unpack(std::index_sequence_for<Args...>{}); }
    BSS_FORCEINLINE R operator()() const { return Call(); }

    R(*_f)(Args...);

  private:
    template<size_t ...S> BSS_FORCEINLINE R _unpack(std::index_sequence<S...>) const { return _f(std::get<S>(*this) ...); }
  };

  template<typename R, typename... Args>
  struct StoreDelegate : std::tuple<Args...>
  {
    inline StoreDelegate(Delegate<R, Args...> fn, Args&&... args) : std::tuple<Args...>(std::forward<Args>(args)...), _fn(fn) {}
    BSS_FORCEINLINE R Call() const { return _unpack(std::index_sequence_for<Args...>{}); }
    BSS_FORCEINLINE R operator()() const { return Call(); }

    Delegate<R, Args...> _fn;

  private:
    template<size_t ...S> BSS_FORCEINLINE R _unpack(std::index_sequence<S...>) const { return _fn(std::get<S>(*this) ...); }
  };

  template<typename R, typename... Args>
  struct DeferFunction : StoreFunction<R, Args...>
  {
    inline DeferFunction(R(*f)(Args...), Args&&... args) : StoreFunction<R, Args...>(f, std::forward<Args>(args)...) {}
    inline ~DeferFunction() { this->Call(); }
  };

  template<typename R, typename... Args>
  struct DeferDelegate : StoreDelegate<R, Args...>
  {
    inline DeferDelegate(Delegate<R, Args...> fn, Args&&... args) : StoreDelegate<R, Args...>(fn, std::forward<Args>(args)...) {}
    inline ~DeferDelegate() { this->Call(); }
  };

  template<typename R, class... Args>
  BSS_FORCEINLINE DeferDelegate<R, Args...> defer(Delegate<R, Args...> fn, Args&&... args) { return DeferDelegate<R, Args...>(fn, std::forward<Args>(args)...); }
  template<typename R, class... Args>
  BSS_FORCEINLINE DeferFunction<R, Args...> defer(R(*stub)(Args...), Args&&... args) { return DeferFunction<R, Args...>(stub, std::forward<Args>(args)...); }
}

#endif
