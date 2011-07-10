// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"
// Insert/delete implementations modified from Arjan van den Boogaard (2004)
// http://archive.gamedev.net/reference/programming/features/TStorage/TStorage.h

#pragma once
#ifndef __C_RBT_LIST_H__BSS__
#define __C_RBT_LIST_H__BSS__

#include "bss_compare.h"
#include "bss_alloc.h"

namespace bss_util {
  /* Node struct for cRBT_List */
  template<class Key, class Data>
  struct __declspec(dllexport) cRBT_Node
  {
  public:
    inline cRBT_Node(cRBT_Node&& mov) : _key(std::move(mov._key)),_data(std::move(mov._data)),_next(mov._next),_prev(mov._prev),
      _left(mov._left),_right(mov._right),_parent(mov._parent),_color(mov._color) {}
    inline cRBT_Node(Key key, Data data, cRBT_Node<Key,Data>* pNIL) : _key(key),_data(data),_next(0),_prev(0),_left(pNIL),_right(pNIL),
      _parent(0),_color(1) {}
    inline explicit cRBT_Node(cRBT_Node<Key,Data>* pNIL) : _next(0),_prev(0),_left(0),_right(0),_parent(0),_color(0) {} //NIL constructor
    inline cRBT_Node() {} //Empty constructor
    Data _data;
    Key _key;
    char _color; // 0 - black, 1 - red, -1 - duplicate
    cRBT_Node<Key,Data>* _next;
    cRBT_Node<Key,Data>* _prev;
    cRBT_Node<Key,Data>* _parent;
    union {
      struct {
        cRBT_Node<Key,Data>* _left;
        cRBT_Node<Key,Data>* _right;
      };
      struct { cRBT_Node<Key,Data>* _children[2]; };
    };

    inline cRBT_Node& operator=(cRBT_Node&& mov) 
    {
      _key=std::move(mov._key);
      _data=std::move(mov._data);
      _next=mov._next;
      _prev=mov._prev;
      _left=mov._left;
      _right=mov._right;
      _parent=mov._parent;
      _color=mov._color;
      return *this;
    }
  };

  /* Public node wrapper for cRBT_Node */
  template<class Key, class Data>
  struct __declspec(dllexport) cRBT_PNode : public cRBT_Node<Key,Data>
  {
  public:
    inline Data& GetData() { return _data; }
    inline const Key& GetKey() const { return _key; }
    inline cRBT_PNode<Key,Data>* GetNext() const { return (cRBT_PNode<Key,Data>*)_next; }
    inline cRBT_PNode<Key,Data>* GetPrev() const { return (cRBT_PNode<Key,Data>*)_prev; }
    inline char GetColor() const { return _color; }
  };

	/* This class is a combination of a red-black tree with a doubly linked list. */
	template <class K, class D, char (*C)(const K& keyleft, const K& keyright)=CompareKeys<K>, typename A=Allocator<cRBT_Node<K,D>>>
	class __declspec(dllexport) cRBT_List : protected cAllocTracker<A>
	{
	public:
		/* Constructor - takes an optional allocator */
		inline explicit cRBT_List(A* allocator=0);
    inline cRBT_List(cRBT_List&& mov);
		/* Destructor */
		inline ~cRBT_List();
    /* Retrieves a given node by key if it exists */
    inline cRBT_PNode<K,D>* Get(const K& key) const;
    /* Retrieves the node closest to the given key. */
    inline cRBT_PNode<K,D>* GetNear(const K& key, bool before=true) const;
		/* Inserts a key (key must be supplied) with the associated data pointer (can be NULL since it is never accessed by the tree itself) */
		inline cRBT_PNode<K,D>* Insert(const K& key, D data);
    /* Searches for a node with the given key and removes it if found, otherwise returns false. */
		inline bool Remove(const K& key);
    /* Removes the given node. Returns false if node is null */
		inline bool Remove(cRBT_Node<K,D>* node);
    /* Replaces the given key with newkey */
		inline bool ReplaceKey(const K& key, const K& newkey);
    /* Replaces the key of the given node to newkey */
		inline bool ReplaceKey(cRBT_Node<K,D>* node, const K& newkey);
    /* Clears the tree (called by destructor) */
		inline void Clear();
    /* Returns first element */
		inline cRBT_PNode<K,D>* GetFirst() const { return (cRBT_PNode<K,D>*)_first; }
    /* Returns last element */
		inline cRBT_PNode<K,D>* GetLast() const { return (cRBT_PNode<K,D>*)_last; }
    inline size_t Size() const { return _size; }
    
    inline cRBT_List& operator=(cRBT_List&& mov);

	protected:
    /* Insert node before target */
    inline void _llbinsert(cRBT_Node<K,D>* node, cRBT_Node<K,D>* target);
    /* Insert node after target */
    inline void _llainsert(cRBT_Node<K,D>* node, cRBT_Node<K,D>* target);
    /* Remove node from list */
    inline void _llremove(cRBT_Node<K,D>* node);
    inline void _leftrotate(cRBT_Node<K,D>* node);
    inline void _rightrotate(cRBT_Node<K,D>* node);
    inline cRBT_Node<K,D>* _get(const K& key) const;
    inline cRBT_Node<K,D>* _getnear(const K& key, bool before) const;
		inline void _insert(cRBT_Node<K,D>* node);
		inline void _remove(cRBT_Node<K,D>* node);
    inline void _replacenode(cRBT_Node<K,D>* target, cRBT_Node<K,D>* with);

		cRBT_Node<K,D>*  _first;
		cRBT_Node<K,D>*  _last;
		cRBT_Node<K,D>*  _root;
		static cRBT_Node<K,D> NIL;
    static cRBT_Node<K,D>* pNIL;
    size_t _size;
    
		inline void FixInsert(cRBT_Node<K,D>* node);
		inline void FixDelete(cRBT_Node<K,D>* node);
		inline static cRBT_Node<K,D>* FindMin(cRBT_Node<K,D>* node);
		inline static cRBT_Node<K,D>* TreeNext(cRBT_Node<K,D>* node);
		inline static cRBT_Node<K,D>* TreeNextSub(cRBT_Node<K,D>* node);
	};
  
	template <class K, class D, char (*C)(const K& keyleft, const K& keyright),typename A>
    cRBT_Node<K,D> cRBT_List<K,D,C,A>::NIL(&cRBT_List<K,D,C,A>::NIL);
	template <class K, class D, char (*C)(const K& keyleft, const K& keyright),typename A>
    cRBT_Node<K,D>* cRBT_List<K,D,C,A>::pNIL=&cRBT_List<K,D,C,A>::NIL;
    
  template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
  inline cRBT_List<K,D,C,A>::cRBT_List(A* alloc) : cAllocTracker<A>(alloc), _size(0),_first(0),_last(0),_root(pNIL)
  {
  }
  
  template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
  inline cRBT_List<K,D,C,A>::cRBT_List(cRBT_List&& mov) : cAllocTracker<A>(std::move(mov)), _size(mov._size),_first(mov._first),
    _last(mov._last),_root(mov._root)
  {
		mov._first = NULL;
		mov._last = NULL;
		mov._root = pNIL;
		mov._size = 0;
  }
  template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
  inline cRBT_List<K,D,C,A>::~cRBT_List()
  {
    Clear();
  }

  template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
  inline cRBT_PNode<K,D>* cRBT_List<K,D,C,A>::Get(const K& key) const
  {
    return (cRBT_PNode<K,D>*)_get(key);
  }

  template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
  inline cRBT_PNode<K,D>* cRBT_List<K,D,C,A>::GetNear(const K& key, bool before=true) const
  {
    return (cRBT_PNode<K,D>*)_getnear(key,before);
  }

  template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
  inline cRBT_PNode<K,D>* cRBT_List<K,D,C,A>::Insert(const K& key, D data)
  {
    cRBT_Node<K,D>* node= _alloc->allocate(1);
    new(node) cRBT_Node<K,D>(key,data,pNIL);

    _insert(node);
    ++_size;
    return (cRBT_PNode<K,D>*)node;
  }

  template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
  inline bool cRBT_List<K,D,C,A>::Remove(const K& key)
  {
    /*cRBT_Node<K,D>* node=_get(key);
    if(!node) return false;
    Remove(node);
    return true;*/
    return Remove(_get(key));
  }

  template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
  inline bool cRBT_List<K,D,C,A>::Remove(cRBT_Node<K,D>* node)
  {
    if(!node) return false;
    //D retval=node->_data;
    _remove(node);
		_alloc->deallocate(node, 1);
    --_size;
    //return retval;
    return true;
  }

	template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
	inline cRBT_Node<K,D>* cRBT_List<K,D,C,A>::_get(const K &key) const
	{
    cRBT_Node<K,D> *  y = _root;
    int c;

    while (y != pNIL)
    {
	    c = C(key, y->_key);
	    if (c < 0)  y = y->_left;
	    else if(c > 0) y = y->_right;
	    else return y;
    }

    return NULL;
	}

	template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
	inline cRBT_Node<K,D>* cRBT_List<K,D,C,A>::_getnear(const K &key, bool before) const
	{
    cRBT_Node<K,D>* cur=_root;
    cRBT_Node<K,D>* parent;
    char res;

		while (cur != pNIL)
		{
      parent=cur;
			res = C(key, cur->_key);
			if (res < 0)  cur = cur->_left;
			else if(res > 0) cur = cur->_right;
			else return cur;
		}

    if(before)
      return (res<0 || !parent->_next)?parent:parent->_next;
    else
      return (res>0 || parent->_prev)?parent:parent->_prev;
	}

  template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
  inline bool cRBT_List<K,D,C,A>::ReplaceKey(const K& key, const K& newkey)
  {
    return ReplaceKey(_get(key), newkey);
  }

  template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
  inline bool cRBT_List<K,D,C,A>::ReplaceKey(cRBT_Node<K,D>* node, const K& newkey)
  {
    if(!node) return false;
    _remove(node);
    new(node) cRBT_Node(newkey,node->_data); //This is an amusing abuse of placeholder new to re-initialize the node. This is safe because data is a value type.
    _insert(node,node->_data);
    return true;
  }

	template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
	inline void cRBT_List<K,D,C,A>::Clear()
	{
			cRBT_Node<K,D> *  node = _first;
			cRBT_Node<K,D> *  hold;

			while(node)
			{
					hold = node;
          node = node->_next;
					_alloc->deallocate(hold, 1);
			}

			_first = NULL;
			_last = NULL;
			_root = pNIL;
			_size = 0;
	}

	template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
  inline cRBT_List<K,D,C,A>& cRBT_List<K,D,C,A>::operator=(cRBT_List<K,D,C,A>&& mov)
  {
    Clear();
    
		_first = mov._first;
		_last = mov._last;
		_root = mov._root;
		_size = mov._size;
		mov._first = NULL;
		mov._last = NULL;
		mov._root = pNIL;
		mov._size = 0;
  }

	template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
	inline void cRBT_List<K,D,C,A>::_rightrotate(cRBT_Node<K,D> * node)
	{
    cRBT_Node<K,D>* rot=node->_left;

    node->_left=rot->_right;
    if(node->_left!=pNIL)
      node->_left->_parent=node;

	  if(rot != pNIL)
      rot->_parent=node->_parent;

    if(node->_parent)
      node->_parent->_children[node->_parent->_right == node]=rot;
    else
      _root=rot;

		rot->_right = node;
		if (node != pNIL) node->_parent = rot;
	}

	template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
	inline void            cRBT_List<K,D,C,A>::_leftrotate(cRBT_Node<K,D> * node)
	{
    cRBT_Node<K,D>* rot=node->_right;

    node->_right=rot->_left;
    if(node->_right!=pNIL)
      node->_right->_parent=node;

	  if(rot != pNIL)
      rot->_parent=node->_parent;

    if(node->_parent)
      node->_parent->_children[node->_parent->_right == node]=rot;
    else
      _root=rot;

		rot->_left = node;
		if (node != pNIL) node->_parent = rot;
	}

	template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
	inline void            cRBT_List<K,D,C,A>::FixInsert(cRBT_Node<K,D> * node)
	{
			while (node != _root && node->_parent->_color == 1)
			{
					if (node->_parent == node->_parent->_parent->_left)
					{
							cRBT_Node<K,D> *  y = node->_parent->_parent->_right;
							if (y->_color == 1)
							{
									node->_parent->_color = 0;
									y->_color = 0;
									node->_parent->_parent->_color = 1;
									node = node->_parent->_parent;
							}
							else
							{
									if (node == node->_parent->_right)
									{
											node = node->_parent;
											_leftrotate(node);
									}

									node->_parent->_color = 0;
									node->_parent->_parent->_color = 1;
									_rightrotate(node->_parent->_parent);
							}
					}
					else
					{
							cRBT_Node<K,D> *  y = node->_parent->_parent->_left;
							if (y->_color == 1)
							{
									node->_parent->_color = 0;
									y->_color = 0;
									node->_parent->_parent->_color = 1;
									node = node->_parent->_parent;
							}
							else
							{
									if (node == node->_parent->_left)
									{
											node = node->_parent;
											_rightrotate(node);
									}
									node->_parent->_color = 0;
									node->_parent->_parent->_color = 1;
									_leftrotate(node->_parent->_parent);
							}
					}
			}

			_root->_color = 0;
	}

	template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
	inline void            cRBT_List<K,D,C,A>::_insert(cRBT_Node<K,D> * node)
	{
		cRBT_Node<K,D> *  cur=_root;
		cRBT_Node<K,D> *  parent=0;
		int c;
     
    while(cur!=pNIL)
    {
      parent=cur;
			c = C(node->_key, cur->_key);
      if(c<0)
        cur=cur->_left;
      else if(c>0)
        cur=cur->_right;
      else //duplicate
      {
        //_llainsert(node,cur); //insert after current node
        //D data=node->_data;
        //node->_data=cur->_data; //Then switch data with it so this acts as through we inserted before it, but don't have to reassign anything.
        //cur->_data=data;
        if(cur->_next!=0 && cur->_next->_color==-1)
        {
          if((cur=TreeNext(cur))!=0) _llbinsert(node,cur);
          else _llainsert(node,_last);
        }
        else
          _llainsert(node,cur);

        node->_color=-1; //set color to duplicate
        return; //terminate, we have nothing else to do since this node isn't actually in the tree
      }
    }
    
    if(parent!=0)
    {
      node->_parent=parent; //set parent

      if(c<0) //depending on if its less than or greater than the parent, set the appropriate child variable
      {
        parent->_left=node;
        _llbinsert(node,parent); //Then insert into the appropriate side of the list
      }
      else
      {
        parent->_right=node;
        if(parent->_next!=0 && parent->_next->_color==-1)
        {
          if((parent=TreeNextSub(parent))==0) _llainsert(node, _last);
          else _llbinsert(node, parent);
        }
        else
          _llainsert(node, parent);
      }
			FixInsert(node);
    }
    else //this is the root node so re-assign
    {
      _first=_last=_root=node; //assign to _first and _last
      _root->_color=0; //root is always black (done below)
    }
	}

	template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
	inline void            cRBT_List<K,D,C,A>::FixDelete(cRBT_Node<K,D> * node)
	{
			while (node != _root && node->_color == 0)
			{
					if (node == node->_parent->_left)
					{
							cRBT_Node<K,D> * w = node->_parent->_right;
							if (w->_color == 1)
							{
									w->_color = 0;
									node->_parent->_color = 1;
									_leftrotate (node->_parent);
									w = node->_parent->_right;
							}
							if (w->_left->_color == 0 && w->_right->_color == 0)
							{
									w->_color = 1;
									node = node->_parent;
							}
							else
							{
									if (w->_right->_color == 0)
									{
											w->_left->_color = 0;
											w->_color = 1;
											_rightrotate(w);
											w = node->_parent->_right;
									}
									w->_color = node->_parent->_color;
									node->_parent->_color = 0;
									w->_right->_color = 0;
									_leftrotate (node->_parent);
									node = _root;
							}
					}
					else
					{
							cRBT_Node<K,D> * w = node->_parent->_left;
							if (w->_color == 1) {
									w->_color = 0;
									node->_parent->_color = 1;
									_rightrotate (node->_parent);
									w = node->_parent->_left;
							}
							if (w->_right->_color == 0 && w->_left->_color == 0)
							{
									w->_color = 1;
									node = node->_parent;
							}
							else
							{
									if (w->_left->_color == 0)
									{
											w->_right->_color = 0;
											w->_color = 1;
											_leftrotate (w);
											w = node->_parent->_left;
									}
									w->_color = node->_parent->_color;
									node->_parent->_color = 0;
									w->_left->_color = 0;
									_rightrotate (node->_parent);
									node = _root;
							}
					}
			}
			node->_color = 0;
	}

	//---------------------------------------------------------------------------
	template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
	inline void            cRBT_List<K,D,C,A>::_remove(cRBT_Node<K,D> * node)
	{
			if (node->_color == -1)
			{
					_llremove(node);
					return;
			}

			if (node->_next && node->_next->_color == -1)
			{
					_replacenode(node, node->_next);
					_llremove(node);
					return;
			}

			_llremove(node);

			cRBT_Node<K,D> *  y;
			cRBT_Node<K,D> *  z;
			bool            balance;

      if(node->_left != pNIL && node->_right != pNIL) //{
        y=FindMin(node->_right);
        //y=node->_right;
			  //while (y->_left != pNIL) y = y->_left;
      //}
      else
         y = node;
			
      z = y->_children[y->_left == pNIL];
			z->_parent = y->_parent;

			if(y->_parent!=0)
        y->_parent->_children[y == y->_parent->_right] = z;
			else
			  _root = z;

			balance = (y->_color == 0);

			if(y != node)
        _replacenode(node, y);

			if(balance)
        FixDelete(z);
	}

	//---------------------------------------------------------------------------
	template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
	inline void            cRBT_List<K,D,C,A>::_llremove(cRBT_Node<K,D> * node)
	{
			if(node->_prev != 0) node->_prev->_next = node->_next;
			else _first = node->_next;
			if(node->_next != 0) node->_next->_prev = node->_prev;
			else _last = node->_prev;
	}

	//---------------------------------------------------------------------------
	template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
	inline void            cRBT_List<K,D,C,A>::_llbinsert(cRBT_Node<K,D> * y, cRBT_Node<K,D> * node)
	{
			y->_prev = node->_prev;
			y->_next = node;
			if(node->_prev != 0) y->_prev->_next = y;
			else _first = y;
			node->_prev = y;
	}

	//---------------------------------------------------------------------------
	template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
	inline void            cRBT_List<K,D,C,A>::_llainsert(cRBT_Node<K,D> * y, cRBT_Node<K,D> * node)
	{
			y->_next = node->_next;
			y->_prev = node;
			if (node->_next != 0) y->_next->_prev = y;
			else _last = y;
			node->_next = y;
	}

	template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
	inline void cRBT_List<K,D,C,A>::_replacenode(cRBT_Node<K,D> * node, cRBT_Node<K,D> * y)
	{
    y->_color = node->_color;
    y->_left = node->_left;
    y->_right = node->_right;
    y->_parent = node->_parent;

    if (y->_parent!=0)
      y->_parent->_children[y->_parent->_right == node] = y;
    else
	    _root = y;

    y->_left->_parent = y;
    y->_right->_parent = y;
	}

	template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
  inline cRBT_Node<K,D>* cRBT_List<K,D,C,A>::TreeNext(cRBT_Node<K,D>* node)
	{
		if (node->_right != pNIL) return FindMin(node->_right);

    return TreeNextSub(node);
	}

	template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
  inline cRBT_Node<K,D>* cRBT_List<K,D,C,A>::TreeNextSub(cRBT_Node<K,D>* node)
	{
    while (node->_parent && node != node->_parent->_left)
			node = node->_parent;

		return node->_parent;
	}

	template<class K, class D, char (*C)(const K& keyleft, const K& keyright), typename A>
  inline cRBT_Node<K,D>* cRBT_List<K,D,C,A>::FindMin(cRBT_Node<K,D>* node)
	{
		while (node->_left != pNIL) node = node->_left;
		return node;
	}
}

#endif
