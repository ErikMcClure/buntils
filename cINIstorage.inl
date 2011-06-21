// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef CHAR
#define CHAR char
#endif

cINIsection<CHAR> cINIstorage<CHAR>::_sectionsentinel;
cINIentry<CHAR> cINIsection<CHAR>::_entrysentinel;

template<>
cINIstorage<CHAR>::cINIstorage(const cINIstorage& copy) : _path(copy._path), _filename(copy._filename), _ini(!copy._ini?0:new cStrT<CHAR>(*copy._ini)), _logger(copy._logger)
{
  _copyhash(copy);
}

template<>
cINIstorage<CHAR>::cINIstorage(const CHAR* file, std::ostream* logger) : _ini(0), _logger(logger)
{
  FILE* f;
  FOPEN(f,file,_T("rt"));
  if(f)
  {
    _loadINI(f);
    fclose(f);
  }
  _setfilepath(file);
}
template<>
cINIstorage<CHAR>::cINIstorage(FILE* file, std::ostream* logger) : _ini(0), _logger(logger)
{
  _loadINI(file);
}

/* Destructor */
template<>
cINIstorage<CHAR>::~cINIstorage()
{
  if(_ini) delete _ini;

	khiter_t iter;
	_sections.ResetWalk();
  cArraySimple<cINIsection<CHAR>>* arr;
	while((iter=_sections.GetNext())!=_sections.End())
  {
    arr=_sections[iter];
    unsigned int svar=arr->Size();
    for(unsigned int i =0; i<svar; ++i) (*arr)[i].~cINIsection<CHAR>();
    delete arr;
  }
}
template<>
cINIsection<CHAR>* cINIstorage<CHAR>::GetSection(const CHAR* section, unsigned int instance) const
{
  khiter_t iter= _sections.GetIterator(section);
  if(iter==_sections.End()) return 0;
  cArraySimple<cINIsection<CHAR>>* arr=_sections[iter];
  return instance<arr->Size()?(*arr)+instance:0;
}
template<>
cINIentry<CHAR>* cINIstorage<CHAR>::GetEntryPtr(const CHAR *section, const CHAR* key, unsigned int keyinstance, unsigned int instance) const
{
  khiter_t iter= _sections.GetIterator(section);
  if(iter==_sections.End()) return 0;
  cArraySimple<cINIsection<CHAR>>* arr=_sections[iter];
  return instance<arr->Size()?(*arr)[instance].GetEntryPtr(key,keyinstance):0;
}
template<>
const std::vector<std::pair<cStrT<CHAR>,unsigned int>>& cINIstorage<CHAR>::BuildSectionList() const
{
  _seclist.clear();
  _sections.ResetWalk();
  khiter_t iter;
  cArraySimple<cINIsection<CHAR>>* arr;
  unsigned int i =0;
  while((iter=_sections.GetNext())!=_sections.End())
  {
    arr=_sections[iter];
    for(i=0; i<arr->Size();++i)
      _seclist.push_back(std::pair<cStrT<CHAR>,unsigned int>((*arr)[i].GetName(),i));
  }
  return _seclist;
}

template<>
cINIsection<CHAR>& BSS_FASTCALL cINIstorage<CHAR>::AddSection(const CHAR* name)
{
  if(!_ini) _openINI();
  _ini->reserve(_ini->size()+strlen(name)+4);
  CHAR c;
  size_t i;
  for(i = _ini->size(); i > 0;)
  {
    c=_ini->GetChar(--i);
    if(!(c==' ' || c=='\n' || c=='\r')) { ++i; break; }
  }
  if(i>0)
  {
    _ini->UnsafeString()[i]='\0';
    _ini->RecalcSize();
    _ini->append(_T("\n\n"));
  }
  _ini->append(_T("["));
  _ini->append(name);
  _ini->append(_T("]"));
  return *_addsection(name);
}

template<>
cINIsection<CHAR>* cINIstorage<CHAR>::_addsection(const CHAR* name)
{
  khiter_t iter=_sections.GetIterator(name);
  cArraySimple<cINIsection<CHAR>>* arr;
  if(iter==_sections.End()) //need to add in an array for this
  {
    arr= new cArraySimple<cINIsection<CHAR>>(1);
    new ((*arr)+0) cINIsection<CHAR>(name,this,0);
    _sections.Insert((*arr)[0].GetName(),arr);
    return ((*arr)+0);
  } else {
    arr=_sections[iter];
    unsigned int index=arr->Size();
    arr->SetSize(index+1);
    new ((*arr)+index) cINIsection<CHAR>(name,this,index);
    _sections.OverrideKeyPtr(iter,(*arr)[0].GetName()); //the key pointer gets corrupted when we resize the array
    return ((*arr)+index);
  }
}

//3 cases: removing something other then the beginning, removing the beginning, or removing the last one
template<>
bool cINIstorage<CHAR>::RemoveSection(const CHAR* name, unsigned int instance)
{
  if(!_ini) _openINI();
  cArraySimple<cINIsection<CHAR>>* arr;
  INICHUNK chunk = bss_findINIsection(*_ini,_ini->size(),name,instance);
  khiter_t iter = _sections.GetIterator(name);
  if(iter!=_sections.End() && chunk.start!=0)
  {
    arr=_sections[iter];
    if(instance>=arr->Size()) return false;
    _ini->erase((const CHAR*)chunk.start-_ini->String(),((const CHAR*)chunk.end-(const CHAR*)chunk.start)+1);
    ((*arr)+instance)->~cINIsection<CHAR>();
    if(arr->Size()==1) //this is the last one in the group
    {
      delete arr;
      _sections.RemoveIterator(iter);
    }
    else
    {
      arr->Remove(instance);
      if(!instance) _sections.OverrideKeyPtr(iter,(*arr)[0].GetName());
      unsigned int svar=arr->Size();
      for(;instance<svar;++instance) --(*arr)[instance]._index; //moves all the indices down
    }

    return true;
  }
  return false;
}

template<>
char cINIstorage<CHAR>::EditEntry(const CHAR* section, const CHAR* key, const CHAR* nvalue, unsigned int secinstance, unsigned int keyinstance)
{
  if(!_ini) _openINI();
  if(secinstance==(unsigned int)-1) AddSection(section);

  khiter_t iter = _sections.GetIterator(section);
  if(iter==_sections.End()) return -1; //if it doesn't exist at this point, fail
  cArraySimple<cINIsection<CHAR>>* arr=_sections[iter];
  if(secinstance==-1) secinstance=arr->Size()-1; //if we just added it, it will be the last entry
  if(secinstance>=arr->Size()) return -2; //if secinstance is not valid, fail
  cINIsection<CHAR>* psec = (*arr)+secinstance;
  INICHUNK chunk = bss_findINIsection(*_ini,_ini->size(),section,secinstance);
  if(!chunk.start) return -3; //if we can't find it in the INI, fail

  if(keyinstance==(unsigned int)-1) //insertion
  {
    psec->_addentry(key,nvalue);
    _ini->reserve(_ini->size()+strlen(key)+strlen(nvalue)+2);
    cStrT<CHAR> construct(_T("\n%s=%s"),key,!nvalue?_T(""):nvalue); //build keyvalue string
    const CHAR* ins=strchr((const CHAR*)chunk.start,'\n');
    if(!ins) //end of file
    {
      _ini->append(construct);
      return 0;
    }
    
    //CHAR c;
    //size_t i;
    //for(i = ins-(*_ini); i > 0;)
    //{
    //  c=_ini->GetChar(--i);
    //  if(!(c==' ' || c=='\n' || c=='\r')) { ++i; break; }
    //}

    //_ini->insert(i,construct);
    _ini->insert(ins-(*_ini),construct);
    return 0;
  } //If it wasn't an insert we need to find the entry before the other two possible cases

  iter=psec->_entries.GetIterator(key);
  if(iter==psec->_entries.End()) return -4; //if key doesn't exist at this point, fail
  cArraySimple<cINIentry<CHAR>>* entarr=psec->_entries[iter];
  if(keyinstance>=entarr->Size()) return -5; //if keyinstance is not valid, fail
  chunk=bss_findINIentry(chunk,key,keyinstance);
  if(!chunk.start) return -7; //if we can't find it

  if(!nvalue) //deletion
  {
    (*entarr+keyinstance)->~cINIentry<CHAR>();
    if(entarr->Size()==1)
    {
      delete entarr;
      psec->_entries.RemoveIterator(iter);
    }
    else 
    {
      entarr->Remove(keyinstance);
      psec->_entries.OverrideKeyPtr(iter,(*entarr)[0].GetKey());
    }
    const CHAR* end=strchr((const CHAR*)chunk.end,'\n');
    end=!end?(const CHAR*)chunk.end:end+1;
    _ini->erase((const CHAR*)chunk.start-_ini->String(),end-(const CHAR*)chunk.start);
  }
  else //edit
  {
    (*entarr+keyinstance)->SetData(nvalue);
    const CHAR* start=strchr((const CHAR*)chunk.start,'=');
    if(!start) return -6; //if this happens something is borked
    start=_trimlstr<CHAR>(++start);
    _ini->replace(start-*_ini,(_trimrstr<CHAR>(((const CHAR*)chunk.end)-1,start)-start)+1,nvalue);
  }

  return 0;
}
template<>
void cINIstorage<CHAR>::EndINIEdit(const CHAR* overridepath)
{
  if(!_ini) return;
  if(overridepath!=0) _setfilepath(overridepath);
  cStrT<CHAR> targetpath=_path;
  targetpath+=_filename;
  FILE* f;
  FOPEN(f,targetpath,_T("wb"));
  fwrite(*_ini,sizeof(CHAR),_ini->size(),f);
  fclose(f);
  DiscardINIEdit();
}
template<>
void cINIstorage<CHAR>::DiscardINIEdit()
{
  if(_ini) delete _ini;
  _ini=0;
}

template<>
void cINIstorage<CHAR>::_loadINI(FILE* f)
{
  INIParser parse;
  bss_initINI(&parse,f);
  cINIsection<CHAR>* cursec=0;
  while(bss_parseLine(&parse)!=0)
  {
    if(parse.newsection)
      cursec=_addsection(parse.cursection);
    else
    {
      if(!cursec) cursec=_addsection(_T(""));
      cursec->_addentry(parse.curkey, parse.curvalue);
    }
  }

  //if(parse.newsection) //We don't check for empty sections
  //  cursec=_addsection(parse.cursection); 

  bss_destroyINI(&parse);
}
template<>
void cINIstorage<CHAR>::_setfilepath(const CHAR* file)
{
  const CHAR* hold=strrchr(file,'/');
  hold=!hold?file :hold+1;
  const CHAR* hold2=strrchr(hold,'\\');
  hold=!hold2?hold:hold2+1;
  _filename=hold;
  size_t length = hold-file;
  _path.resize(++length);
  memcpy(_path.UnsafeString(),file,length);
  _path.UnsafeString()[length-1]='\0';
  _path.RecalcSize();
}
template<>
void cINIstorage<CHAR>::_copyhash(const cINIstorage& copy)
{
  khiter_t iter;
  unsigned int i=0;
  cArraySimple<cINIsection<CHAR>>* old;
  cArraySimple<cINIsection<CHAR>>* arr;
	copy._sections.ResetWalk();
	while((iter=copy._sections.GetNext())!=copy._sections.End())
  {
    old=copy._sections[iter];
    arr=new cArraySimple<cINIsection<CHAR>>(*old);
    unsigned int svar=arr->Size();
    for(unsigned int i =0; i < svar; ++i)
      new ((*arr)+i) cINIsection<CHAR>((*old)[i]);
    _sections.Insert((*arr)[0].GetName(),arr);
  }
}

template<>
cINIstorage<CHAR>& cINIstorage<CHAR>::operator=(const cINIstorage<CHAR>& right)
 { 
   _path=right._path;
   _filename=right._filename;
   _ini=!right._ini?0:new cStrT<CHAR>(*right._ini);
   _logger=right._logger;
   _sections.Clear();
   _copyhash(right);
   return *this;
}




template<>
cINIsection<CHAR>::cINIsection(const cINIsection& copy) : _name(copy._name),_parent(copy._parent),_index(copy._index)
{
  _copyhash(copy);
}
template<>
cINIsection<CHAR>::cINIsection() : _parent(0)
{
}
template<>
cINIsection<CHAR>::cINIsection(const CHAR* name, cINIstorage<CHAR>* parent, unsigned int index) : _name(name), _parent(parent), _index(index)
{
}
template<>
cINIsection<CHAR>::~cINIsection()
{
	khiter_t iter;
	_entries.ResetWalk();
  
  cArraySimple<cINIentry<CHAR>>* arr;
	while((iter=_entries.GetNext())!=_entries.End())
  {
    arr=_entries[iter];
    unsigned int svar=arr->Size();
    for(unsigned int i =0; i<svar; ++i) (*arr)[i].~cINIentry<CHAR>();
    delete arr;
  }
}
template<>
void cINIsection<CHAR>::_copyhash(const cINIsection& copy)
{
  khiter_t iter;
  unsigned int i=0;
  cArraySimple<cINIentry<CHAR>>* old;
  cArraySimple<cINIentry<CHAR>>* arr;
	copy._entries.ResetWalk();
	while((iter=copy._entries.GetNext())!=copy._entries.End())
  {
    old=copy._entries[iter];
    arr=new cArraySimple<cINIentry<CHAR>>(*old);
    unsigned int svar=arr->Size();
    for(unsigned int i =0; i < svar; ++i)
      new ((*arr)+i) cINIentry<CHAR>((*old)[i]);
    _entries.Insert((*arr)[0].GetKey(),arr);
  }
}
template<>
void cINIsection<CHAR>::_addentry(const CHAR* key, const CHAR* data)
{
  khiter_t iter=_entries.GetIterator(key);
  cArraySimple<cINIentry<CHAR>>* arr;
  if(iter==_entries.End()) //need to add in an array for this
  {
    arr= new cArraySimple<cINIentry<CHAR>>(1);
    new ((*arr)+0) cINIentry<CHAR>(key,data);
    _entries.Insert((*arr)[0].GetKey(),arr);
  } else {
    arr=_entries[iter];
    unsigned int index=arr->Size();
    arr->SetSize(index+1);
    new ((*arr)+index) cINIentry<CHAR>(key,data);
    _entries.OverrideKeyPtr(iter,(*arr)[0].GetKey());
  }
}

template<>
const std::vector<std::pair<std::pair<cStrT<CHAR>,cStrT<CHAR>>,unsigned int>>& cINIsection<CHAR>::BuildEntryList() const
{
  assert(_parent!=0);
  _parent->_entlist.clear();
  _entries.ResetWalk();
  khiter_t iter;
  cArraySimple<cINIentry<CHAR>>* arr;
  unsigned int i =0;
  while((iter=_entries.GetNext())!=_entries.End())
  {
    arr=_entries[iter];
    for(i=0; i<arr->Size();++i)
      _parent->_entlist.push_back(std::pair<std::pair<cStrT<CHAR>,cStrT<CHAR>>,unsigned int>(std::pair<cStrT<CHAR>,cStrT<CHAR>>((*arr)[i].GetKey(),(*arr)[i].GetString()),i));
  }
  return _parent->_entlist;
}

template<>
cINIsection<CHAR>& cINIsection<CHAR>::operator=(const cINIsection<CHAR>& right)
{ 
  _name=right._name;
  _index=right._index;
  _parent=right._parent;
  _entries.Clear();
  _copyhash(right);
  return *this;
}

template<>
cINIentry<CHAR>* cINIsection<CHAR>::GetEntryPtr(const CHAR* key, unsigned int instance) const
{ 
  khiter_t iter= _entries.GetIterator(key);
  if(iter==_entries.End()) return 0;
  cArraySimple<cINIentry<CHAR>>* arr=_entries[iter];
  return instance<arr->Size()?(*arr)+instance:0;
}

template<>
cINIentry<CHAR>& cINIsection<CHAR>::GetEntry(const CHAR* key, unsigned int instance) const
{ 
  cINIentry<CHAR>* entry=GetEntryPtr(key,instance);
  return !entry?_entrysentinel:*entry;
}





template<>
cINIentry<CHAR>::cINIentry() : _lvalue(0),_dvalue(0.0)//,_index(0)
{
}
template<>
cINIentry<CHAR>::cINIentry(const CHAR *key, const CHAR *svalue, long lvalue, double dvalue) : _key(key),_svalue(svalue),
  _lvalue(lvalue),_dvalue(dvalue)//,_index(index)
{
}
template<>
cINIentry<CHAR>::cINIentry(const CHAR* key, const CHAR* data) : _key(key)//,_index(index)
{
  SetData(data);
}

template<>
cINIentry<CHAR>::~cINIentry()
{
}
//
//template<> const CHAR* cINIentry<CHAR>::GetKey() const { return _key; }
//template<> const CHAR* cINIentry<CHAR>::GetString() const { return _svalue; }
//template<> long cINIentry<CHAR>::GetLong() const { return _lvalue; }
//template<> double cINIentry<CHAR>::GetDouble() const { return _dvalue; }
//
//template<> cINIentry<CHAR>::operator bool() const { return _lvalue!=0; }
//template<> cINIentry<CHAR>::operator short() const { return (short)_lvalue; }
//template<> cINIentry<CHAR>::operator int() const { return (int)_lvalue; }
//template<> cINIentry<CHAR>::operator long() const { return (long)_lvalue; }
//template<> cINIentry<CHAR>::operator unsigned short() const { return (unsigned short)_lvalue; }
//template<> cINIentry<CHAR>::operator unsigned int() const { return (unsigned int)_lvalue; }
//template<> cINIentry<CHAR>::operator unsigned long() const { return (unsigned long)_lvalue; }
//template<> cINIentry<CHAR>::operator float() const { return (float)_dvalue; }
//template<> cINIentry<CHAR>::operator double() const { return _dvalue; }
//template<> cINIentry<CHAR>::operator const CHAR*() const { return _svalue; }