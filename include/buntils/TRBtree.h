// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"
// Insert/delete implementations modified from Arjan van den Boogaard (2004)
// http://archive.gamedev.net/archive/reference/programming/features/TStorage/TStorage.h

#ifndef __TRB_TREE_H__BUN__
#define __TRB_TREE_H__BUN__

#include "compare.h"
#include "Alloc.h"
#include "LLBase.h"

namespace bun {
  namespace internal {
    template<class T>
    concept IsTRBNode = IntrusiveLinkedList<T> &&
                        std::same_as<std::remove_cvref_t<decltype(std::declval<T>().parent)>, T*> &&
                        std::same_as<std::remove_cvref_t<decltype(std::declval<T>().left)>, T*> &&
                        std::same_as<std::remove_cvref_t<decltype(std::declval<T>().right)>, T*> &&
                        std::same_as<std::remove_cvref_t<decltype(std::declval<T>().color)>, char>;

    // Generic Threaded Red-black tree node
    template<class T> struct BUN_COMPILER_DLLEXPORT TRB_NodeBase : LLBase<T>
    {
      using LLBase<T>::next;
      using LLBase<T>::prev;

      inline explicit TRB_NodeBase(T* pNIL, char c = 1) : left(pNIL), right(pNIL), color(c), parent(0)
      {
        next = 0;
        prev = 0;
      }
      inline TRB_NodeBase(TRB_NodeBase&& mov, T*& root, T*& first, T*& last, T* pNIL)
      {
        parent = mov.parent;

        if(parent)
        {
          if(parent->left == &mov)
            parent->left = this;
          else if(parent->right == &mov)
            parent->right = this;
          else
            assert(false);
        }
        else
          root = this;

        left = mov.left;
        if(left != pNIL)
        {
          assert(left->parent == &mov);
          left->parent = this;
        }
        right = mov.right;
        if(right != pNIL)
        {
          assert(right->parent == &mov);
          right->parent = this;
        }
        next = mov.next;
        if(next)
        {
          assert(next->prev == &mov);
          next->prev = this;
        }
        else
        {
          assert(last == &mov);
          last = this;
        }
        prev = mov.prev;
        if(prev)
        {
          assert(prev->next == &mov);
          prev->next = this;
        }
        else
        {
          assert(root == &mov);
          root = this;
        }

        color      = mov.color;
        mov.left   = pNIL;
        mov.right  = pNIL;
        mov.next   = 0;
        mov.prev   = 0;
        mov.parent = 0;
      }
      T* parent;
      union
      {
        struct
        {
          T* left;
          T* right;
        };
        T* children[2];
      };
      int8_t color; // 0 - black, 1 - red, -1 - duplicate

      // Returns false if this node was changed such that it is now out of order and needs to be re-inserted, otherwise
      // returns true.
      inline bool Validate(Comparison<const T&, const T&> auto&& f)
      {
        return (!prev || (f(*static_cast<T*>(this), *prev) >= 0)) && (!next || (f(*static_cast<T*>(this), *next) <= 0));
      }

      // Removes a node from the given abstract tree
      static void RemoveNode(T* node, T*& root, T*& first, T*& last, T* pNIL)
      {
        assert(node != pNIL);
        if(node->color == -1)
        {
          LLRemove(node, first, last);
          return;
        }
        if(node->next && node->next->color == -1)
        {
          _replaceNode(node, node->next, root);
          LLRemove(node, first, last);
          pNIL->parent = 0;
          return;
        }

        LLRemove(node, first, last);
        T* y;
        T* z;

        if(node->left != pNIL && node->right != pNIL)
          y = _findMin(node->right, pNIL);
        else
          y = node;

        z         = y->children[y->left == pNIL];
        z->parent = y->parent;

        if(y->parent != 0)
          y->parent->children[y == y->parent->right] = z;
        else
          root = z;

        bool balance = (y->color == 0);

        if(y != node)
          _replaceNode(node, y, root);
        if(balance)
          _fixDelete(z, root, pNIL);
        pNIL->parent = 0;
        assert(pNIL->color == 0);
      }

      // Inserts a node into the given abstract tree
      static void InsertNode(T* node, T*& root, T*& first, T*& last, T* pNIL, Comparison<const T&, const T&> auto&& f)
      {
        assert(node != pNIL);
        node->color = 1;
        node->next  = 0;
        node->prev  = 0;
        node->left  = pNIL;
        node->right = pNIL;
        T* cur      = root;
        T* parent   = 0;
        decltype(f(*node, *cur)) c;

        while(cur != pNIL)
        {
          parent = cur;
          c = f(*node, *cur);
          if(c < 0)
            cur = cur->left;
          else if(c > 0)
            cur = cur->right;
          else [[unlikely]] // duplicate
          {
            LLInsertAfter(node, cur, last);
            node->color = -1; // set color to duplicate
            return;           // terminate, we have nothing else to do since this node isn't actually in the tree
          }
        }

        if(parent != 0)
        {
          node->parent = parent; // set parent

          if(c < 0) // depending on if its less than or greater than the parent, set the appropriate child variable
          {
            parent->left = node;
            LLInsert(node, parent, first); // Then insert into the appropriate side of the list
          }
          else
          {
            parent->right = node;

            if(parent->next != 0 && parent->next->color == -1) // This is required to support duplicate nodes
            { // What can happen is you get here with a value greater than the parent, then try to insert after the
              // parent... but any duplicate values would then be ahead of you.
              if((parent = _treeNextSub(parent)) == 0)
                last = LLAddAfter(node, last);
              else
                LLInsert(node, parent, first);
            }
            else
              LLInsertAfter(node, parent, last); // If there aren't any duplicate values in front of you, it doesn't matter.
          }
          _fixInsert(node, root, pNIL);
        }
        else // this is the root node so re-assign
        {
          first = last = root = node; // assign to first and last
          root->color         = 0;    // root is always black (done below)
          root->next          = 0;
          root->prev          = 0;
          root->parent        = 0;
        }
      }

      // Verifies this node structure satisifies the red-black tree constraints
      inline static int DEBUGVERIFY(const TRB_NodeBase<T>* n, const TRB_NodeBase<T>* NIL)
      {
        if(n == NIL)
          return n->color == 0 ? 1 : -1;
        if(n->color == 1 && (n->left->color != 0 || n->right->color != 0))
          return -1;
        if(n->left != NIL && n->left->parent != n)
          return -1;
        if(n->right != NIL && n->right->parent != n)
          return -1;
        if(n->next && n->next->prev != n)
          return -1;
        if(n->prev && n->prev->next != n)
          return -1;
        int l = DEBUGVERIFY(n->left, NIL);
        int r = DEBUGVERIFY(n->right, NIL);
        if(l < 0 || r < 0 || r != l)
          return -1;
        return l + (n->color == 0);
      }

    protected:
      static void _leftRotate(T* node, T*& root, T* pNIL)
      {
        T* r = node->right;

        node->right = r->left;
        if(node->right != pNIL)
          node->right->parent = node;
        if(r != pNIL)
          r->parent = node->parent;

        if(node->parent)
          node->parent->children[node->parent->right == node] = r;
        else
          root = r;

        r->left = node;
        if(node != pNIL)
          node->parent = r;
      }
      static void _rightRotate(T* node, T*& root, T* pNIL)
      {
        T* r = node->left;

        node->left = r->right;
        if(node->left != pNIL)
          node->left->parent = node;
        if(r != pNIL)
          r->parent = node->parent;

        if(node->parent)
          node->parent->children[node->parent->right == node] = r;
        else
          root = r;

        r->right = node;
        if(node != pNIL)
          node->parent = r;
      }
      inline static void _fixInsert(T* node, T*& root, T* pNIL)
      {
        while(node != root && node->parent->color == 1)
        {
          if(node->parent == node->parent->parent->left)
          {
            T* y = node->parent->parent->right;

            if(y->color == 1)
            {
              node->parent->color         = 0;
              y->color                    = 0;
              node->parent->parent->color = 1;
              node                        = node->parent->parent;
            }
            else
            {
              if(node == node->parent->right)
              {
                node = node->parent;
                _leftRotate(node, root, pNIL);
              }

              node->parent->color         = 0;
              node->parent->parent->color = 1;
              _rightRotate(node->parent->parent, root, pNIL);
            }
          }
          else
          {
            T* y = node->parent->parent->left;

            if(y->color == 1)
            {
              node->parent->color         = 0;
              y->color                    = 0;
              node->parent->parent->color = 1;
              node                        = node->parent->parent;
            }
            else
            {
              if(node == node->parent->left)
              {
                node = node->parent;
                _rightRotate(node, root, pNIL);
              }

              node->parent->color         = 0;
              node->parent->parent->color = 1;
              _leftRotate(node->parent->parent, root, pNIL);
            }
          }
        }

        root->color = 0;
      }
      inline static void _fixDelete(T* node, T*& root, T* pNIL)
      {
        while(node != root && node->color == 0)
        {
          if(node == node->parent->left)
          {
            T* w = node->parent->right;
            assert(w != pNIL);
            if(w->color == 1)
            {
              w->color            = 0;
              node->parent->color = 1;
              _leftRotate(node->parent, root, pNIL);
              w = node->parent->right;
            }
            assert(w != pNIL);
            if(w->left->color == 0 && w->right->color == 0)
            {
              w->color = 1;
              node     = node->parent;
            }
            else
            {
              if(w->right->color == 0)
              {
                w->left->color = 0;
                w->color       = 1;
                _rightRotate(w, root, pNIL);
                w = node->parent->right;
              }

              w->color            = node->parent->color;
              node->parent->color = 0;
              w->right->color     = 0;
              _leftRotate(node->parent, root, pNIL);
              node = root;
            }
          }
          else
          {
            T* w = node->parent->left;
            assert(w != pNIL);
            if(w->color == 1)
            {
              w->color            = 0;
              node->parent->color = 1;
              _rightRotate(node->parent, root, pNIL);
              w = node->parent->left;
            }
            assert(w != pNIL);
            if(w->right->color == 0 && w->left->color == 0)
            {
              w->color = 1;
              node     = node->parent;
            }
            else
            {
              if(w->left->color == 0)
              {
                w->right->color = 0;
                w->color        = 1;
                _leftRotate(w, root, pNIL);
                w = node->parent->left;
              }

              w->color            = node->parent->color;
              node->parent->color = 0;
              w->left->color      = 0;
              _rightRotate(node->parent, root, pNIL);
              node = root;
            }
          }
        }
        node->color = 0;
      }
      inline static void _replaceNode(T* node, T* y, T*& root)
      {
        y->color  = node->color;
        y->left   = node->left;
        y->right  = node->right;
        y->parent = node->parent;

        if(y->parent != 0)
          y->parent->children[y->parent->right == node] = y;
        else
          root = y;

        y->left->parent  = y;
        y->right->parent = y;
      }
      BUN_FORCEINLINE static T* _findMin(T* node, T* pNIL)
      {
        while(node->left != pNIL)
          node = node->left;
        return node;
      }
      inline static T* _treeNextSub(T* node)
      {
        while(node->parent && node != node->parent->left)
          node = node->parent;
        return node->parent;
      }
    };
  }

  // Threaded Red-black tree node with a value
  template<class T> struct BUN_COMPILER_DLLEXPORT TRB_Node : internal::TRB_NodeBase<TRB_Node<T>>
  {
    inline explicit TRB_Node(TRB_Node* pNIL) : internal::TRB_NodeBase<TRB_Node<T>>(pNIL, 0) {}
    inline TRB_Node(T v, TRB_Node* pNIL) : value(v), internal::TRB_NodeBase<TRB_Node<T>>(pNIL) {}
#pragma warning(push)
#pragma warning(disable : 4251)
    T value;
#pragma warning(pop)
  };

  namespace internal {
    template<typename T, Comparison<T, T> C> struct TRB_Node_ThreeWay : protected C
    {
      TRB_Node_ThreeWay() = default;
      TRB_Node_ThreeWay(const C& c) : C(c) {}
      TRB_Node_ThreeWay(C&& c) : C(std::move(c)) {}
      TRB_Node_ThreeWay(const TRB_Node_ThreeWay&) = default;
      TRB_Node_ThreeWay(TRB_Node_ThreeWay&&)      = default;

      [[nodiscard]] constexpr BUN_FORCEINLINE auto operator()(const TRB_Node<T>& l, const TRB_Node<T>& r) const
      {
        return _getf()(l.value, r.value);
      }

      [[nodiscard]] constexpr BUN_FORCEINLINE const C& _getf() const noexcept { return *this; }

      using is_transparent = int;
    };
  }

  // Threaded Red-black tree implementation
  template<typename T, Comparison<T, T> Comp = std::compare_three_way, typename Alloc = StandardAllocator<TRB_Node<T>>>
  class BUN_COMPILER_DLLEXPORT BUN_EMPTY_BASES TRBtree :
    protected Alloc,
    protected internal::TRB_Node_ThreeWay<T, Comp>
  {
    [[nodiscard]] constexpr BUN_FORCEINLINE const Comp& _getcomp() const noexcept { return *this; }
    [[nodiscard]] constexpr BUN_FORCEINLINE const internal::TRB_Node_ThreeWay<T, Comp>& _getnodecomp() const noexcept
    {
      return *this;
    }

  public:
    TRBtree(const TRBtree&) = delete;
    inline TRBtree(TRBtree&& mov) :
      Alloc(std::move(mov)),
      internal::TRB_Node_ThreeWay<T, Comp>(std::move(mov)),
      _first(mov._first),
      _last(mov._last),
      _root(mov._root),
      NIL(mov.NIL)
    {
      mov._first = 0;
      mov._last  = 0;
      mov.NIL    = std::allocator_traits<Alloc>::allocate(*this, 1);
      new(NIL) TRB_Node<T>(0);
      mov.NIL->left  = mov.NIL;
      mov.NIL->right = mov.NIL;
      mov._root      = mov.NIL;
    }
    inline explicit TRBtree(const Alloc& alloc, const Comp& comp) :
      Alloc(alloc), internal::TRB_Node_ThreeWay<T, Comp>(comp), _first(0), _last(0), NIL(0), _root(0)
    {
      NIL = std::allocator_traits<Alloc>::allocate(*this, 1);
      new(NIL) TRB_Node<T>(0);
      NIL->left  = NIL;
      NIL->right = NIL;
      _root      = NIL;
    }
    inline TRBtree(const Alloc& alloc)
      requires std::is_default_constructible_v<Comp>
      : TRBtree(alloc, Comp())
    {}
    inline TRBtree(const Comp& comp)
      requires std::is_default_constructible_v<Alloc>
      : TRBtree(Alloc(), comp)
    {}
    inline TRBtree()
      requires std::is_default_constructible_v<Alloc> && std::is_default_constructible_v<Comp>
      : TRBtree(Alloc(), Comp())
    {}
    // Destructor
    inline ~TRBtree()
    {
      Clear();
      NIL->~TRB_Node();
      std::allocator_traits<Alloc>::deallocate(*this, NIL, 1);
    }
    // Clears the tree
    inline void Clear()
    {
      for(auto i = begin(); i.IsValid();) // Walk through the tree using the linked list and deallocate everything
      {
        (*i)->~TRB_Node();
        std::allocator_traits<Alloc>::deallocate(*this, *(i++), 1);
      }

      _first = 0;
      _last  = 0;
      _root  = NIL;
    }
    // Retrieves a given node by key if it exists
    BUN_FORCEINLINE TRB_Node<T>* Get(const T& value) const { return GetNode(value, _root, NIL, _getcomp()); }
    // Retrieves the node closest to the given key.
    BUN_FORCEINLINE TRB_Node<T>* GetNear(const T& value, bool before = true) const
    {
      return GetNodeNear(value, before, _root, NIL, _getcomp());
    }
    // Gets the root node
    BUN_FORCEINLINE const TRB_Node<T>* GetRoot() const { return _root; }
    // Inserts a key with the associated data
    BUN_FORCEINLINE TRB_Node<T>* Insert(const T& value)
    {
      TRB_Node<T>* node = std::allocator_traits<Alloc>::allocate(*this, 1);
      new(node) TRB_Node<T>(value, NIL);
      TRB_Node<T>::InsertNode(node, _root, _first, _last, NIL, _getnodecomp());
      return node;
    }
    // Searches for a node with the given key and removes it if found, otherwise returns false.
    BUN_FORCEINLINE bool Remove(const T& value) { return Remove(GetNode(value, _root, NIL, _getcomp())); }
    // Removes the given node. Returns false if node is null
    BUN_FORCEINLINE bool Remove(TRB_Node<T>* node)
    {
      if(!node)
        return false;

      TRB_Node<T>::RemoveNode(node, _root, _first, _last, NIL);
      node->~TRB_Node();
      std::allocator_traits<Alloc>::deallocate(*this, node, 1);
      return true;
    }
    // Returns first element
    BUN_FORCEINLINE TRB_Node<T>* Front() const { return _first; }
    // Returns last element
    BUN_FORCEINLINE TRB_Node<T>* Back() const { return _last; }
    // Iteration functions
    inline LLIterator<const TRB_Node<T>> begin() const { return LLIterator<const TRB_Node<T>>(_first); }
    inline LLIterator<const TRB_Node<T>> end() const { return LLIterator<const TRB_Node<T>>(0); }
    inline LLIterator<TRB_Node<T>> begin() { return LLIterator<TRB_Node<T>>(_first); }
    inline LLIterator<TRB_Node<T>> end() { return LLIterator<TRB_Node<T>>(0); }
    inline static bool Validate(TRB_Node<T>* node, const Comp& comp)
    {
      return node->Validate<internal::TRB_Node_ThreeWay<T, Comp>>(internal::TRB_Node_ThreeWay<T, Comp>(comp));
    }

    inline TRBtree& operator=(TRBtree&& mov)
    {
      Clear();
      TRB_Node<T>* nil = NIL;
      _first           = mov._first;
      _last            = mov._last;
      _root            = mov._root;
      NIL              = mov.NIL;
      mov._first       = 0;
      mov._last        = 0;
      mov.NIL          = nil;
      mov._root        = mov.NIL;
      return *this;
    }

    static TRB_Node<T>* GetNode(const T& x, TRB_Node<T>* const& root, TRB_Node<T>* pNIL, const Comp& comp)
    {
      TRB_Node<T>* cur = root;

      while(cur != pNIL)
      {
        auto c = comp(x, cur->value);
        if(c < 0)
          cur = cur->left;
        else if(c > 0)
          cur = cur->right;
        else [[unlikely]]
          return cur;
      }

      return 0;
    }
    static TRB_Node<T>* GetNodeNear(const T& x, bool before, TRB_Node<T>* const& root, TRB_Node<T>* pNIL, const Comp& comp)
    {
      TRB_Node<T>* cur    = root;
      TRB_Node<T>* parent = pNIL;
      decltype(comp(x, cur->value)) res;

      while(cur != pNIL)
      {
        parent = cur;
        res    = comp(x, cur->value);
        if(res < 0)
          cur = cur->left;
        else if(res > 0)
          cur = cur->right;
        else [[unlikely]]
          return cur;
      }

      if(before)
        return (res < 0 && parent->prev) ? parent->prev : parent;
      else
        return (res > 0 && parent->next) ? parent->next : parent;
    }

    inline int DEBUGVERIFY() { return TRB_Node<T>::DEBUGVERIFY(_root, NIL); }

  protected:
    TRB_Node<T>* _first;
    TRB_Node<T>* _last;
    TRB_Node<T>* _root;
    TRB_Node<T>* NIL;
  };
}

#endif