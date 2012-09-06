// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_INISECTION_H__BSS__
#define __C_INISECTION_H__BSS__

#include "cINIentry.h"
#include "cKhash.h"
#include "cArraySimple.h"
#include "LLBase.h"
#include "bss_alloc_fixed.h"

namespace bss_util {
  class cINIstorage;

  // Internal INI linked list node
  template<class T>
  struct BSS_COMPILER_DLLEXPORT _INInode : public LLBase<_INInode<T>>
  {
    typename DArray<_INInode<T>*>::t instances; // If this is not the only instance, points to an array of all the other instances
    T val;
  };

  // Stores an INI section, denoted by [name], as a linked list of entries, also stored as a hash for O(1) time operations.
  class BSS_DLLEXPORT cINIsection
  {
    typedef _INInode<cINIentry> _NODE;

  public:
    cINIsection(const cINIsection& copy);
    cINIsection(cINIsection&& mov);
    cINIsection();
    cINIsection(const char* name, cINIstorage* parent,unsigned int index);
    ~cINIsection();
    cINIentry& GetEntry(const char* key, unsigned int instance=0) const;
    cINIentry* GetEntryPtr(const char* key, unsigned int instance=0) const;
    inline char EditEntry(const char* key, const char* data, unsigned int instance=0); //if data is NULL the entry is deleted. if instance is -1 the entry is inserted
    inline const _NODE* GetEntryList() const { return _root; }

    inline cINIstorage* GetParent() const { return _parent; }
    inline const char* GetName() const { return _name; }
    inline unsigned int GetIndex() const { return _index; }
    inline BSS_FORCEINLINE cINIentry& operator[](const char* key) const { return GetEntry(key,0); }
    cINIsection& operator=(const cINIsection& right);
    cINIsection& operator=(cINIsection&& mov);

  protected:
    friend class cINIstorage;

    void _destroy();
    void _addentry(const char* key, const char* data);
    void _copy(const cINIsection& copy);

    static cINIentry _entrysentinel;
    static cFixedAlloc<_NODE,4> _alloc;

    cStr _name;
    unsigned int _index;
    _NODE* _root;
    _NODE* _last;
    cKhash_StringTIns<char,_NODE*,true> _entries;
    cINIstorage* _parent;
  };
}

#endif