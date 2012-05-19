// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_INISTORAGE_H__BSS__
#define __C_INISTORAGE_H__BSS__

#include "cKhash.h"
#include "cStr.h"
#include "bss_dlldef.h"
#include "cArraySimple.h"

namespace bss_util {
  class cINIstorage;
  class cINIsection;

  struct cINIentry
  {
    //cINIentry(const cINIentry& copy);
    BSS_DLLEXPORT cINIentry(cINIentry&& mov);
    BSS_DLLEXPORT cINIentry();
    BSS_DLLEXPORT cINIentry(const char* key, const char* svalue, long lvalue, double dvalue);
    BSS_DLLEXPORT cINIentry(const char* key, const char* data);
    BSS_DLLEXPORT ~cINIentry();
    BSS_DLLEXPORT void SetData(const char* data);
    inline const char* GetKey() const { return _key; }
    inline const char* GetString() const { return _svalue; }
    inline long GetLong() const { return _lvalue; }
    inline double GetDouble() const { return _dvalue; }
    inline bool IsValid() const { return !_key.empty(); }

    inline operator bool() const { return _lvalue!=0; }
    inline operator char() const { return (char)_lvalue; }
    inline operator short() const { return (short)_lvalue; }
    inline operator int() const { return (int)_lvalue; }
    inline operator long() const { return (long)_lvalue; }
    inline operator unsigned char() const { return (unsigned char)_lvalue; }
    inline operator unsigned short() const { return (unsigned short)_lvalue; }
    inline operator unsigned int() const { return (unsigned int)_lvalue; }
    inline operator unsigned long() const { return (unsigned long)_lvalue; }
    inline operator float() const { return (float)_dvalue; }
    inline operator double() const { return _dvalue; }
    inline operator const char*() const { return _svalue; }

    BSS_DLLEXPORT bool operator ==(cINIentry &other) const; //these can't be inlined because the compare function is different.
    BSS_DLLEXPORT bool operator !=(cINIentry &other) const;
    BSS_DLLEXPORT cINIentry& operator=(cINIentry&& mov);

  private:
    cStr _key;
    cStr _svalue;
    long _lvalue;
    double _dvalue;
  };

  class cINIsection
  {
    typedef std::pair<const char*,unsigned int> KEYPAIR;
    typedef cArrayWrap<cArraySimple<cINIentry>> __ARR;

  public:
    BSS_DLLEXPORT cINIsection(const cINIsection& copy);
    BSS_DLLEXPORT cINIsection(cINIsection&& mov);
    BSS_DLLEXPORT cINIsection();
    BSS_DLLEXPORT cINIsection(const char* name, cINIstorage* parent,unsigned int index);
    BSS_DLLEXPORT ~cINIsection();
    BSS_DLLEXPORT cINIentry& GetEntry(const char* key, unsigned int instance=0) const;
    BSS_DLLEXPORT cINIentry* GetEntryPtr(const char* key, unsigned int instance=0) const;
    inline char EditEntry(const char* key, const char* data, unsigned int instance=0);

    /* Builds a list of all entries */
    BSS_DLLEXPORT const std::vector<std::pair<std::pair<cStr,cStr>,unsigned int>>& BuildEntryList() const;

    inline cINIstorage* GetParent() const { return _parent; }
    inline const char* GetName() const { return _name; }
    inline unsigned int GetIndex() const { return _index; }
    inline cINIentry& operator[](const char* key) const { return GetEntry(key,0); }
    BSS_DLLEXPORT cINIsection& operator=(const cINIsection& right);
    BSS_DLLEXPORT cINIsection& operator=(cINIsection&& mov);

  protected:
    friend class cINIstorage;
#pragma warning(push)
#pragma warning(disable: 4251)
    static BSS_DLLEXPORT cINIentry _entrysentinel;
#pragma warning(pop)
    BSS_DLLEXPORT void _copyhash(const cINIsection& copy);
    BSS_DLLEXPORT void _destroyhash();
    BSS_DLLEXPORT void _addentry(const char* key, const char* data);

    cStr _name;
    unsigned int _index;
    cKhash_StringTIns<char,__ARR*,true> _entries;
    cINIstorage* _parent;
  };

  class cINIstorage
  {
    typedef std::pair<const char*,unsigned int> KEYPAIR;
    typedef cArrayWrap<cArraySimple<cINIsection>> __ARR;
    typedef cArrayWrap<cArraySimple<cINIentry>> __ENTARR;

  public:
    /* Constructor - takes a filepath */
    BSS_DLLEXPORT cINIstorage(const cINIstorage& copy);
    BSS_DLLEXPORT cINIstorage(cINIstorage&& mov);
    BSS_DLLEXPORT cINIstorage(const char* file=0, std::ostream* logger=0);
    BSS_DLLEXPORT cINIstorage(FILE* file, std::ostream* logger=0);
    /* Destructor */
    BSS_DLLEXPORT ~cINIstorage();
    /* Gets a section based on name and instance */
    BSS_DLLEXPORT cINIsection* GetSection(const char* section, unsigned int instance=0) const;
    /* Gets a convertable INI entry */
    BSS_DLLEXPORT cINIentry* GetEntryPtr(const char *section, const char* key, unsigned int keyinstance=0, unsigned int instance=0) const;
    /* Builds a list of all sections */
    BSS_DLLEXPORT const std::vector<std::pair<cStr,unsigned int>>& BuildSectionList() const;
    
    BSS_DLLEXPORT cINIsection& BSS_FASTCALL AddSection(const char* name);
    BSS_DLLEXPORT bool RemoveSection(const char* name, unsigned int instance=0);
    BSS_DLLEXPORT char EditEntry(const char* section, const char* key, const char* nvalue=0, unsigned int secinstance=0, unsigned int keyinstance=0); //if nvalue is 0 the entry is deleted. if either instance is -1 it triggers an insert
    inline char EditAddEntry(const char* section, const char* key, const char* nvalue=0, unsigned int secinstance=0, unsigned int keyinstance=0) { return EditEntry(section,key,nvalue,GetSection(section,secinstance)==0?-1:secinstance,GetEntryPtr(section,key,keyinstance,secinstance)==0?-1:keyinstance); }
    BSS_DLLEXPORT void EndINIEdit(const char* overridepath=0); //Saves changes to file (INI files are automatically opened when an edit operation is done)
    BSS_DLLEXPORT void DiscardINIEdit();

    inline cINIsection& operator [](const char *section) const { cINIsection* ret=GetSection(section,0); return !ret?_sectionsentinel:*ret; }
    BSS_DLLEXPORT cINIstorage& operator=(const cINIstorage& right);
    BSS_DLLEXPORT cINIstorage& operator=(cINIstorage&& mov);
    inline const char* GetPath() const { return _path; } //gets path to folder this INI was in
    inline cINIentry& GetEntry(const char *section, const char* key, unsigned int keyinstance=0, unsigned int instance=0) const { cINIentry* ret=GetEntryPtr(section,key,keyinstance,instance); return !ret?cINIsection::_entrysentinel:*ret; }

    //cINIentry& EditEntry(const char* section, const char* data, const char* key, unsigned int instance=0);
    //cINIentry& EditEntry(const char* section, unsigned int sectioninstance, const char* data, const char* key, unsigned int instance=0);
    //bool RemoveEntry(const char *section, const char* key, unsigned int keyinstance=0, unsigned int instance=0);
    //static cINIstorage* CreateINIstorage(const char* file);
    //const std::vector<std::pair<const char*,unsigned short>>& GetSectionList() const;
    //cINIentry& operator ()(const char *section,  const char* key, unsigned int keyinstance=0, unsigned int instance=0) const;
   
  protected:
    friend class cINIsection;

    BSS_DLLEXPORT void _loadINI(FILE* file);
    BSS_DLLEXPORT void _openINI();
    BSS_DLLEXPORT void _setfilepath(const char* path);
    BSS_DLLEXPORT cINIsection* _addsection(const char* name);
    BSS_DLLEXPORT void _copyhash(const cINIstorage& copy);
    BSS_DLLEXPORT void _destroyhash();

    cKhash_StringTIns<char,__ARR*,true> _sections;
    cStr _path; //holds path to INI
    cStr _filename; //holds INI filename;
    cStr* _ini; //holds entire INI file
    std::ostream* _logger; //Holds a pointer to a logger object
#pragma warning(push)
#pragma warning(disable: 4251)
    static BSS_DLLEXPORT cINIsection _sectionsentinel;
    mutable std::vector<std::pair<cStr,unsigned int>> _seclist; //holder array for sectionlist building
    mutable std::vector<std::pair<std::pair<cStr,cStr>,unsigned int>> _entlist; //holder array for entrylist building
#pragma warning(pop)
  };

  inline char cINIsection::EditEntry(const char* key, const char* data, unsigned int instance) { return !_parent?-1:_parent->EditEntry(_name,key,data,_index,instance); } //if data is NULL the entry is deleted. if instance is -1 the entry is inserted
}

#endif