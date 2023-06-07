// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __INISTORAGE_H__BUN__
#define __INISTORAGE_H__BUN__

#include "INIsection.h"

namespace bun {
  // Stores an INI file as a linked list of sections, also stored via hash for O(1) time operations.
  class BUN_DLLEXPORT INIstorage
  {
  public:
    using _NODE = INIsection::_INInode<INIsection>;

    // Constructors
    INIstorage(const INIstorage& copy);
    INIstorage(INIstorage&& mov);
    INIstorage(const char* file = 0, std::ostream* logger = 0);
    INIstorage(FILE* file, std::ostream* logger = 0);
    // Destructor
    ~INIstorage();
    // Gets a section based on name and instance
    INIsection* GetSection(const char* section, size_t instance = 0) const;
    // Gets number of sections with the given name
    size_t GetNumSections(const char* section) const;
    // Gets a section node based on name and instance
    _NODE* GetSectionNode(const char* section, size_t instance = 0) const;
    // Gets a convertable INI entry
    INIentry* GetEntryPtr(const char *section, const char* key, size_t keyinstance = 0, size_t secinstance = 0) const;
    // Gets an INI entry node for iteration
    INIsection::_NODE* GetEntryNode(const char *section, const char* key, size_t keyinstance = 0, size_t secinstance = 0) const;
    // Gets the root node of the section linked list
    inline const _NODE* Front() const { return _root; }
    // Gets the last node of the section linked list
    inline const _NODE* Back() const { return _last; }
    // iterators for standard containers
    inline INIsection::INIiterator<INIsection> begin() { return INIsection::INIiterator<INIsection>(_root); }
    inline INIsection::INIiterator<INIsection> end() { return INIsection::INIiterator<INIsection>(0); }

    INIsection& AddSection(const char* name);
    bool RemoveSection(const char* name, size_t instance = 0);
    char EditEntry(const char* section, const char* key, const char* nvalue = 0, size_t keyinstance = 0, size_t secinstance = 0); //if nvalue is 0 the entry is deleted. if either instance is -1 it triggers an insert
    inline char EditAddEntry(const char* section, const char* key, const char* nvalue = 0, size_t keyinstance = 0, size_t secinstance = 0) { return EditEntry(section, key, nvalue, GetEntryPtr(section, key, keyinstance, secinstance) == 0 ? -1 : keyinstance, GetSection(section, secinstance) == 0 ? -1 : secinstance); }
    void EndINIEdit(const char* overridepath = 0); //Saves changes to file (INI files are automatically opened when an edit operation is done)
    void DiscardINIEdit();

    BUN_FORCEINLINE INIsection& operator [](const char *section) const { INIsection* ret = GetSection(section, 0); return !ret ? _sectionsentinel : *ret; }
    INIstorage& operator=(const INIstorage& right);
    INIstorage& operator=(INIstorage&& mov);
    inline const char* GetPath() const { return _path; } //gets path to folder this INI was in
    inline INIentry& GetEntry(const char *section, const char* key, size_t keyinstance = 0, size_t secinstance = 0) const { INIentry* ret = GetEntryPtr(section, key, keyinstance, secinstance); return !ret ? INIsection::_entrysentinel : *ret; }
    void DEBUGTEST()
    {
      for(auto[k,v] : _sections)
      {
        auto& p = v->instances;
        assert(!p.Capacity() == !p.begin());
      }
    }

  protected:
    friend class INIsection;

    void _loadINI(FILE* file);
    void _openINI();
    void _setFilePath(const char* path);
    INIsection* _addSection(const char* name);
    void _copy(const INIstorage& copy);
    void _destroy();

    static INIsection _sectionsentinel;
    static LocklessBlockPolicy<_NODE> _alloc;

    HashIns<const char*, _NODE*> _sections;
    Str _path; //holds path to INI
    Str _filename; //holds INI filename;
    Str* _ini; //holds entire INI file. Made a pointer so we can distinguish between an empty INI file and an unopened file.
    std::ostream* _logger; //Holds a pointer to a logger object
    _NODE* _root;
    _NODE* _last;
  };
}

#endif
