// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_INISTORAGE_H__BSS__
#define __C_INISTORAGE_H__BSS__

#include "cINIsection.h"

namespace bss_util {
  // Stores an INI file as a linked list of sections, also stored via hash for O(1) time operations.
  class BSS_DLLEXPORT cINIstorage
  {
  public:
    typedef _INInode<cINIsection> _NODE;

    // Constructors
    cINIstorage(const cINIstorage& copy);
    cINIstorage(cINIstorage&& mov);
    cINIstorage(const char* file=0, std::ostream* logger=0);
    cINIstorage(FILE* file, std::ostream* logger=0);
    // Destructor
    ~cINIstorage();
    // Gets a section based on name and instance
    cINIsection* GetSection(const char* section, unsigned int instance=0) const;
    // Gets a section node based on name and instance
    _NODE* GetSectionNode(const char* section, unsigned int instance=0) const;
    // Gets a convertable INI entry
    cINIentry* GetEntryPtr(const char *section, const char* key, unsigned int keyinstance=0, unsigned int secinstance=0) const;
    // Gets an INI entry node for iteration
    cINIsection::_NODE* GetEntryNode(const char *section, const char* key, unsigned int keyinstance=0, unsigned int secinstance=0) const;
    // Gets the root node of the section linked list
    inline const _NODE* Front() const { return _root; }
    // Gets the last node of the section linked list
    inline const _NODE* Back() const { return _last; }
    
    cINIsection& BSS_FASTCALL AddSection(const char* name);
    bool RemoveSection(const char* name, unsigned int instance=0);
    char EditEntry(const char* section, const char* key, const char* nvalue=0, unsigned int keyinstance=0,unsigned int secinstance=0); //if nvalue is 0 the entry is deleted. if either instance is -1 it triggers an insert
    inline char EditAddEntry(const char* section, const char* key, const char* nvalue=0, unsigned int keyinstance=0,unsigned int secinstance=0) { return EditEntry(section,key,nvalue,GetEntryPtr(section,key,keyinstance,secinstance)==0?-1:keyinstance,GetSection(section,secinstance)==0?-1:secinstance); }
    void EndINIEdit(const char* overridepath=0); //Saves changes to file (INI files are automatically opened when an edit operation is done)
    void DiscardINIEdit();

    inline BSS_FORCEINLINE cINIsection& operator [](const char *section) const { cINIsection* ret=GetSection(section,0); return !ret?_sectionsentinel:*ret; }
    cINIstorage& operator=(const cINIstorage& right);
    cINIstorage& operator=(cINIstorage&& mov);
    inline const char* GetPath() const { return _path; } //gets path to folder this INI was in
    inline cINIentry& BSS_FASTCALL GetEntry(const char *section, const char* key, unsigned int keyinstance=0, unsigned int secinstance=0) const { cINIentry* ret=GetEntryPtr(section,key,keyinstance,secinstance); return !ret?cINIsection::_entrysentinel:*ret; }
    
  protected:
    friend class cINIsection;

    void _loadINI(FILE* file);
    void _openINI();
    void _setfilepath(const char* path);
    cINIsection* _addsection(const char* name);
    void _copy(const cINIstorage& copy);
    void _destroy();
    void _BuildSectionList(std::vector<std::pair<cStr,unsigned int>>& list) const;

    static cINIsection _sectionsentinel;
    static cFixedAlloc<_NODE,4> _alloc;

    cKhash_StringTIns<char,_NODE*,true> _sections;
    cStr _path; //holds path to INI
    cStr _filename; //holds INI filename;
    cStr* _ini; //holds entire INI file. Made a pointer so we can distinguish between an empty INI file and an unopened file.
    std::ostream* _logger; //Holds a pointer to a logger object
    _NODE* _root;
    _NODE* _last;
  };
}

#endif