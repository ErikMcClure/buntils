// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_INISECTION_H__BSS__
#define __C_INISECTION_H__BSS__

#include "cINIentry.h"
#include "cHash.h"
#include "cArray.h"
#include "LLBase.h"
#include "bss_alloc_block_MT.h"

namespace bss_util {
  class cINIstorage;

  // Internal INI linked list node
  template<class T>
  struct BSS_COMPILER_DLLEXPORT _INInode : public LLBase<_INInode<T>>
  {
    cArray<_INInode<T>*> instances; // If this is not the only instance, points to an array of all the other instances
    T val;
  };

  // Stores an INI section, denoted by [name], as a linked list of entries, also stored as a hash for O(1) time operations.
  class BSS_DLLEXPORT cINIsection
  {
  public:
    typedef _INInode<cINIentry> _NODE;
    template<class T>
    struct INIiterator : LLIterator<_INInode<T>> {
      using LLIterator<_INInode<T>>::cur;
      inline explicit INIiterator(_INInode<T>* node) : LLIterator<_INInode<T>>(node) {}
      inline T& operator*() const { return cur->val; }
      inline T* operator->() const { return &cur->val; }
    };

// Constructors
    cINIsection(const cINIsection& copy);
    cINIsection(cINIsection&& mov);
    cINIsection();
    cINIsection(const char* name, cINIstorage* parent, size_t index);
    // Destructors
    ~cINIsection();
    // Gets the specified key with the given index. On failure returns an empty sentinel reference
    cINIentry& GetEntry(const char* key, size_t instance = 0) const;
    // Gets the specified key with the given index. Returns null on failure.
    cINIentry* GetEntryPtr(const char* key, size_t instance = 0) const;
    // Gets number of entries with the given name
    size_t GetNumEntries(const char* section) const;
    // Gets the specified key node for iteration with the given index. Returns null on failure.
    _NODE* GetEntryNode(const char* key, size_t instance = 0) const;
    // Changes the specified entry data with the given index, if data is nullptr the entry is deleted. if instance is -1 the entry is inserted.
    inline char EditEntry(const char* key, const char* data, size_t instance = 0);
    // Gets the root node of the section linked list
    inline const _NODE* Front() const { return _root; }
    // Gets the last node of the section linked list
    inline const _NODE* Back() const { return _last; }
    // Iterators for standard containers
    inline INIiterator<cINIentry> begin() { return INIiterator<cINIentry>(_root); }
    inline INIiterator<cINIentry> end() { return INIiterator<cINIentry>(0); }

    BSS_FORCEINLINE cINIstorage* GetParent() const { return _parent; }
    BSS_FORCEINLINE const char* GetName() const { return _name; }
    BSS_FORCEINLINE size_t GetIndex() const { return _index; }

    BSS_FORCEINLINE cINIentry& operator[](const char* key) const { return GetEntry(key, 0); }
    cINIsection& operator=(const cINIsection& right);
    cINIsection& operator=(cINIsection&& mov);

  protected:
    friend class cINIstorage;

    void _destroy();
    void _addentry(const char* key, const char* data);
    void _copy(const cINIsection& copy);

    static cINIentry _entrysentinel;
    static cLocklessBlockAlloc<_NODE> _alloc;

    cStr _name;
    size_t _index;
    _NODE* _root;
    _NODE* _last;
    cHash<const char*, _NODE*, true> _entries;
    cINIstorage* _parent;
  };
}

#endif
