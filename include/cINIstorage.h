// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_INISTORAGE_H__BSS__
#define __C_INISTORAGE_H__BSS__

#include "cKhash.h"
#include "cStr.h"
#include "bss_dlldef.h"
#include "cArraySimple.h"

namespace bss_util {
  template<typename T> class cINIstorage;
  template<typename T> class cINIsection;

  template<typename T=char>
  struct cINIentry
  {
    //cINIentry(const cINIentry& copy);
    BSS_DLLEXPORT cINIentry(cINIentry&& mov);
    BSS_DLLEXPORT cINIentry();
    BSS_DLLEXPORT cINIentry(const T* key, const T* svalue, long lvalue, double dvalue);
    BSS_DLLEXPORT cINIentry(const T* key, const T* data);
    BSS_DLLEXPORT ~cINIentry();
    BSS_DLLEXPORT void SetData(const T* data);
    inline const T* GetKey() const { return _key; }
    inline const T* GetString() const { return _svalue; }
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
    inline operator const T*() const { return _svalue; }

    BSS_DLLEXPORT bool operator ==(cINIentry &other) const; //these can't be inlined because the compare function is different.
    BSS_DLLEXPORT bool operator !=(cINIentry &other) const;
    BSS_DLLEXPORT cINIentry& operator=(cINIentry&& mov);

  private:
    cStrT<T> _key;
    cStrT<T> _svalue;
    long _lvalue;
    double _dvalue;
  };

  template<typename T=char>
  class cINIsection
  {
    typedef std::pair<const T*,unsigned int> KEYPAIR;
    typedef cArrayWrap<cArraySimple<cINIentry<T>>> __ARR;

  public:
    BSS_DLLEXPORT cINIsection(const cINIsection& copy);
    BSS_DLLEXPORT cINIsection(cINIsection&& mov);
    BSS_DLLEXPORT cINIsection();
    BSS_DLLEXPORT cINIsection(const T* name, cINIstorage<T>* parent,unsigned int index);
    BSS_DLLEXPORT ~cINIsection();
    BSS_DLLEXPORT cINIentry<T>& GetEntry(const T* key, unsigned int instance=0) const;
    BSS_DLLEXPORT cINIentry<T>* GetEntryPtr(const T* key, unsigned int instance=0) const;
    inline char EditEntry(const T* key, const T* data, unsigned int instance=0) { return !_parent?-1:_parent->EditEntry(_name,key,data,_index,instance); } //if data is NULL the entry is deleted. if instance is -1 the entry is inserted

    /* Builds a list of all entries */
    BSS_DLLEXPORT const std::vector<std::pair<std::pair<cStrT<T>,cStrT<T>>,unsigned int>>& BuildEntryList() const;

    inline cINIstorage<T>* GetParent() const { return _parent; }
    inline const T* GetName() const { return _name; }
    inline unsigned int GetIndex() const { return _index; }
    inline cINIentry<T>& operator[](const T* key) const { return GetEntry(key,0); }
    BSS_DLLEXPORT cINIsection& operator=(const cINIsection& right);
    BSS_DLLEXPORT cINIsection& operator=(cINIsection&& mov);

  protected:
    friend class cINIstorage<T>;
#pragma warning(push)
#pragma warning(disable: 4251)
    static BSS_DLLEXPORT cINIentry<T> _entrysentinel;
#pragma warning(pop)
    BSS_DLLEXPORT void _copyhash(const cINIsection& copy);
    BSS_DLLEXPORT void _destroyhash();
    BSS_DLLEXPORT void _addentry(const T* key, const T* data);

    cStrT<T> _name;
    unsigned int _index;
    cKhash_StringTIns<T,__ARR*,true> _entries;
    cINIstorage<T>* _parent;
  };

  template<typename T=char>
  class cINIstorage
  {
    typedef std::pair<const T*,unsigned int> KEYPAIR;
    typedef cArrayWrap<cArraySimple<cINIsection<T>>> __ARR;
    typedef cArrayWrap<cArraySimple<cINIentry<T>>> __ENTARR;

  public:
    /* Constructor - takes a filepath */
    BSS_DLLEXPORT cINIstorage(const cINIstorage& copy);
    BSS_DLLEXPORT cINIstorage(cINIstorage&& mov);
    BSS_DLLEXPORT cINIstorage(const T* file=0, std::ostream* logger=0);
    BSS_DLLEXPORT cINIstorage(FILE* file, std::ostream* logger=0);
    /* Destructor */
    BSS_DLLEXPORT ~cINIstorage();
    /* Gets a section based on name and instance */
    BSS_DLLEXPORT cINIsection<T>* GetSection(const T* section, unsigned int instance=0) const;
    /* Gets a convertable INI entry */
    BSS_DLLEXPORT cINIentry<T>* GetEntryPtr(const T *section, const T* key, unsigned int keyinstance=0, unsigned int instance=0) const;
    /* Builds a list of all sections */
    BSS_DLLEXPORT const std::vector<std::pair<cStrT<T>,unsigned int>>& BuildSectionList() const;
    
    BSS_DLLEXPORT cINIsection<T>& BSS_FASTCALL AddSection(const T* name);
    BSS_DLLEXPORT bool RemoveSection(const T* name, unsigned int instance=0);
    BSS_DLLEXPORT char EditEntry(const T* section, const T* key, const T* nvalue=0, unsigned int secinstance=0, unsigned int keyinstance=0); //if nvalue is 0 the entry is deleted. if either instance is -1 it triggers an insert
    inline char EditAddEntry(const T* section, const T* key, const T* nvalue=0, unsigned int secinstance=0, unsigned int keyinstance=0) { return EditEntry(section,key,nvalue,GetSection(section,secinstance)==0?-1:secinstance,GetEntryPtr(section,key,keyinstance,secinstance)==0?-1:keyinstance); }
    BSS_DLLEXPORT void EndINIEdit(const T* overridepath=0); //Saves changes to file (INI files are automatically opened when an edit operation is done)
    BSS_DLLEXPORT void DiscardINIEdit();

    inline cINIsection<T>& operator [](const T *section) const { cINIsection<T>* ret=GetSection(section,0); return !ret?_sectionsentinel:*ret; }
    BSS_DLLEXPORT cINIstorage& operator=(const cINIstorage& right);
    BSS_DLLEXPORT cINIstorage& operator=(cINIstorage&& mov);
    inline const T* GetPath() const { return _path; } //gets path to folder this INI was in
    inline cINIentry<T>& GetEntry(const T *section, const T* key, unsigned int keyinstance=0, unsigned int instance=0) const { cINIentry<T>* ret=GetEntryPtr(section,key,keyinstance,instance); return !ret?cINIsection<T>::_entrysentinel:*ret; }

    //cINIentry<T>& EditEntry(const T* section, const T* data, const T* key, unsigned int instance=0);
    //cINIentry<T>& EditEntry(const T* section, unsigned int sectioninstance, const T* data, const T* key, unsigned int instance=0);
    //bool RemoveEntry(const T *section, const T* key, unsigned int keyinstance=0, unsigned int instance=0);
    //static cINIstorage* CreateINIstorage(const T* file);
    //const std::vector<std::pair<const T*,unsigned short>>& GetSectionList() const;
    //cINIentry<T>& operator ()(const T *section,  const T* key, unsigned int keyinstance=0, unsigned int instance=0) const;
   
  protected:
    friend class cINIsection<T>;

    BSS_DLLEXPORT void _loadINI(FILE* file);
    BSS_DLLEXPORT void _openINI();
    BSS_DLLEXPORT void _setfilepath(const T* path);
    BSS_DLLEXPORT cINIsection<T>* _addsection(const T* name);
    BSS_DLLEXPORT void _copyhash(const cINIstorage& copy);
    BSS_DLLEXPORT void _destroyhash();

    cKhash_StringTIns<T,__ARR*,true> _sections;
    cStrT<T> _path; //holds path to INI
    cStrT<T> _filename; //holds INI filename;
    cStrT<T>* _ini; //holds entire INI file
    std::ostream* _logger; //Holds a pointer to a logger object
#pragma warning(push)
#pragma warning(disable: 4251)
    static BSS_DLLEXPORT cINIsection<T> _sectionsentinel;
    mutable std::vector<std::pair<cStrT<T>,unsigned int>> _seclist; //holder array for sectionlist building
    mutable std::vector<std::pair<std::pair<cStrT<T>,cStrT<T>>,unsigned int>> _entlist; //holder array for entrylist building
#pragma warning(pop)
  };
}

#endif