// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "cINIstorage.h"
#include <limits> //for NaN
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <string.h>
#include "bss_deprecated.h"
#include "bss_util_c.h"
#include "cStr.h"
#include "INIparse.h"

//#define STRINGCOPYLENGTH(from,to,length) size_t length = strlen(from); to = new char[++length]; STRCPY(to, length, from);
//#define STRINGCOPY(from,to) STRINGCOPYLENGTH(from,to,length)

using namespace bss_util;

template<typename T>
const T* BSS_FASTCALL _trimlstr(const T* str)
{
  for(;*str>0 && *str<33;++str);
  return str;
}

template<typename T>
const T* BSS_FASTCALL _trimrstr(const T* end,const T* begin)
{
  for(;end>begin && *end<33;--end);
  return end;
}

template<>
void cINIstorage<char>::_openINI()
{
  cStr targetpath=_path;
  targetpath+=_filename;
  FILE* f;
  FOPEN(f,targetpath,"a+b"); //this will create the file if it doesn't already exist
  if(!f) return;
  fseek(f,0,SEEK_END);
  unsigned long size=ftell(f);
  fseek(f,0,SEEK_SET);
  _ini = new cStr(size+1);
  size=fread(_ini->UnsafeString(),sizeof(char),size,f); //reads in the entire file
  _ini->UnsafeString()[size]='\0';
  _ini->RecalcSize();
  fclose(f);
}

template<>
void cINIstorage<wchar_t>::_openINI()
{
  cStrW targetpath=_path;
  targetpath+=_filename;
  FILE* f;
  WFOPEN(f,targetpath,L"a+b"); //this will create the file if it doesn't already exist
  if(!f) return;
  fseek(f,0,SEEK_END);
  unsigned long size=ftell(f);
  fseek(f,0,SEEK_SET);
  cStr buf(size+1);
  size=fread(buf.UnsafeString(),sizeof(char),size,f); //reads in the entire file
  buf.UnsafeString()[size]='\0';
  size=UTF8Decode2BytesUnicode(buf,0);
  _ini = new cStrW(size+1);
  UTF8Decode2BytesUnicode(buf,_ini->UnsafeString());
  _ini->UnsafeString()[size]='\0';
  _ini->RecalcSize();
  fclose(f);
}

void _futfwrite(const wchar_t* source, char discard, int size, FILE* f)
{
  unsigned char* buf = (unsigned char*)malloc(size*sizeof(wchar_t));
  size=UTF8Encode2BytesUnicode(source,buf);
  fwrite(buf,sizeof(unsigned char),size,f);
  free(buf);
}


#define _T(s) (s)

//#define STR_RT "rt"
//#define STR_NONE ""
//#define STR_SL "["
//
//#define STR_NSES "\n%s=%s"
//#define STR_WT "wt"
//#define STR_APT "a+b"
//#define STR_LB "]"

#define CHAR char
#include "cINIstorage.inl"
#undef CHAR
#define CHAR wchar_t
#undef FOPEN
#define FOPEN WFOPEN
#define strlen wcslen
#define strchr wcschr
#define strrchr wcsrchr
#define fputs fputws
#define INIParser INIParserW
#define bss_findINIentry bss_wfindINIentry
#define bss_findINIsection bss_wfindINIsection
#define bss_initINI bss_winitINI
#define bss_destroyINI bss_wdestroyINI
#define bss_parseLine bss_wparseLine
#define fwrite _futfwrite

#undef _T
#define _T(s) WIDEN(s)
//#undef STR_RT
//#undef STR_WT
//#undef STR_NONE
//#undef STR_NNSL
//#undef STR_NSES
//#undef STR_APT 
//#undef STR_LB
//#define STR_RT L"rb" //So apparently text mode is assumed to be ASCII... even if you use a wchar_t function. You have to open a unicode file in binary and the parser magically works
//#define STR_WT L"wb"
//#define STR_NONE L""
//#define STR_NNSL L"\n\n["
//#define STR_NSES L"\n%s=%s"
//#define STR_APT L"a+b"
//#define STR_LB L"]"

#include "cINIstorage.inl"
#undef CHAR
#undef FOPEN
#undef strchr

template<> bool cINIentry<char>::operator ==(cINIentry &other) const { return STRICMP(_key,other._key)==0 && STRICMP(_svalue,other._svalue)==0; }
template<> bool cINIentry<char>::operator !=(cINIentry &other) const { return STRICMP(_key,other._key)!=0 || STRICMP(_svalue,other._svalue)!=0; }

template<> bool cINIentry<wchar_t>::operator ==(cINIentry &other) const { return WCSICMP(_key,other._key)==0 && WCSICMP(_svalue,other._svalue)==0; }
template<> bool cINIentry<wchar_t>::operator !=(cINIentry &other) const { return WCSICMP(_key,other._key)!=0 || WCSICMP(_svalue,other._svalue)!=0; }

template<>
void cINIentry<char>::SetData(const char* data)
{  
  if(!data) return;
  _svalue=data;

  if(_svalue[0] == '0' && (_svalue[1] == 'x' || _svalue[1] == 'X')) //If this is true its a hex number
  {
    std::string s(_svalue);
    unsigned long v;
    std::istringstream iss(s);
    iss >> std::setbase(0) >> v;
    std::setbase(10); //reset base to 10
    _lvalue = (long)v;
    _dvalue = (double)v;
  }
  else if(!strchr(_svalue, '.')) //If there isn't a period in the sequence, its either a string (in which case we don't care) or an integer, so set _dvalue to invalid.
  {
    _lvalue = atol(_svalue);
    _dvalue = (double)_lvalue;
  }
  else //Ok its got a . in there so its either a double or a string, so we just round _lvalue
  {
    _dvalue = atof(_svalue);
    _lvalue = (long)_dvalue;
  }
}


template<>
void cINIentry<wchar_t>::SetData(const wchar_t* data)
{  
  if(!data) return;
  _svalue=data;

  if(_svalue[0] == '0' && (_svalue[1] == 'x' || _svalue[1] == 'X')) //If this is true its a hex number
  {
    std::wstring s(_svalue);
    unsigned long v;
    std::wistringstream iss(s);
    iss >> std::setbase(0) >> v;
    std::setbase(10); //reset base to 10
    _lvalue = (long)v;
    _dvalue = (double)v;
  }
  else if(!wcschr(_svalue, '.')) //If there isn't a period in the sequence, its either a string (in which case we don't care) or an integer, so set _dvalue to invalid.
  {
    _lvalue = atol(cStr(_svalue)); //all numbers are ascii, so to convert we just convert to ASCII from unicode and then do the conversion (there are no wide versions of these functions)
    _dvalue = (double)_lvalue;
  }
  else //Ok its got a . in there so its either a double or a string, so we just round _lvalue
  {
    _dvalue = atof(cStr(_svalue));
    _lvalue = (long)_dvalue;
  }
}

//cINIcomment::cINIcomment(const char* comment) : _comment(comment)
//{
//}
//cINIcomment::~cINIcomment()
//{
//}
//void cINIcomment::Write(FILE* f)
//{
//  if(_comment.size())
//  {
//    fwrite(";", 1,1,f);
//    fwrite(_comment, 1, strlen(_comment), f);
//  }
//}
//
//cINIstorage::cINIstorage(const cINIstorage& copy)// : cINIparse((cINIparse)copy)
//{
//  memcpy(this,&copy,sizeof(cINIstorage));
//
//  _path = copy.GetPath();
//}
//
//cINIstorage::cINIstorage(FILE* file, std::ostream* logger) //: cINIparse()
//{
//  //Open(file);
//  _logger = logger;
//  _loadINI();
//}
//
//cINIstorage::cINIstorage() //: cINIparse()
//{
//  _logger = 0;
//}
//
//cINIstorage::cINIstorage(const char* file, std::ostream* logger) //: cINIparse()
//{
//  //Open(file);
//  _logger = logger;
//  _loadINI();
//
//  if(file)
//  {
//    const char* pstr = strrchr(file, '\\');
//    if(!pstr) pstr = strrchr(file, '/');
//    if(pstr)
//    {
//      size_t length=pstr-file;
//      _path.reserve((++length)+1);
//      STRNCPY(_path.UnsafeString(),length+1, file, length);
//      _path.UnsafeString()[length]='\0';
//    }
//  }
//}
//
//cINIstorage::~cINIstorage()
//{
//  INI_MultiValue<cINIsection>* cur;
//  cINIsection* hold;
//	unsigned int j;
//	khiter_t iter;
//	_sections.ResetWalk();
//	while((iter=_sections.GetNext())!=_sections.End())
//	{
//		cur=_sections.Get(iter);
//		j=-1;
//		while(hold = cur->Get(++j))
//			delete hold;
//		delete cur;
//    //_sections.Remove(iter);
//	}
//}
//
//void cINIstorage::_loadINI()
//{
//  cINIsection* cursec=_addsection("",0);
//  while(getNextLine() != -1)
//  {
//    if(IsNewSection())
//      cursec=AddSection(getCurSection(),getCurComment());
//    
//    cursec->AddEntry(getCurID(), getCurString(), getCurComment());
//  }
//
//  if(IsNewSection()) //It's possible the last line of the INI file is an empty section so we have to check for that
//      cursec=AddSection(getCurSection(),getCurComment());
//  if(f_ini) fclose(f_ini);
//}
//cINIsection* cINIstorage::GetSection(const char *section, unsigned int instance) const
//{
//	INI_MultiValue<cINIsection>* result = _sections.GetKey(section);
//	if(!result) return 0;
//	return result->Get(instance);
//}
////void cINIstorage::ResetWalk()
////{
////  _sections.ResetWalk();
////  _curwalk=_sections.End();
////  _curwalkinst=-1;
////}
////cINIsection* cINIstorage::GetNext() const
////{
////  if(_curwalk==_sections.End())
////  {
////    _curwalkinst=-1;
////    _curwalk = _sections.GetNext();
////    if(_curwalk==_sections.End())
////      return 0;
////  }
////
////  INI_MultiValue<cINIsection>* val = _sections.Get(_curwalk);
////  cINIsection* retval = val->Get(++_curwalkinst);
////  if(!retval) _curwalk=_sections.End();
////  else return retval;
////  return GetNext();
////}
//const std::vector<std::pair<const char*,unsigned short>>& cINIstorage::GetSectionList() const
//{
//  return _sectionlist;
//}
//
//cINIsection* cINIstorage::operator [](const char *section) const
//{
//  return GetSection(section);
//}
//
//cINIentry* cINIstorage::GetEntryPtr(const char *section, const char* key, unsigned int keyinstance, unsigned int instance) const
//{
//  cINIsection* temp = GetSection(section, instance);
//
//  if(!temp)
//    return 0; //We don't need to throw an error because getSection throws its own error
//
//  return temp->GetEntryPtr(key, keyinstance);
//}
//
//cINIentry& cINIstorage::GetEntry(const char *section, const char* key, unsigned int keyinstance, unsigned int instance) const
//{
//  return *GetEntryPtr(section,key,instance,keyinstance);
//}
//cINIentry& cINIstorage::operator ()(const char *section,  const char* key, unsigned int keyinstance, unsigned int instance) const
//{
//  return GetEntry(section, key, instance, keyinstance);
//}
//
//cINIsection* BSS_FASTCALL cINIstorage::AddSection(const char* name, const char* comment)
//{
//  if(!name || name[0] == '\0') return 0; //can't have a null name becuase that section already exists and is reserved for entries with no section
//
//  return _addsection(name,comment);
//}
//
//cINIsection* BSS_FASTCALL cINIstorage::_addsection(const char* name, const char* comment)
//{
//  cINIsection* retval = new cINIsection(name, this, _logger, comment);
//  INI_MultiValue<cINIsection>* result = _sections.GetKey(name);
//  if(!result)
//	{
//		result = new INI_MultiValue<cINIsection>(retval, name); //Our hasher only takes POINTERS to strings, so we must store the name here in the collection, otherwise it'll pop in and out of existence
//		_sections.Insert(result->GetName(), result);
//    _sectionlist.push_back(std::pair<const char*,unsigned short>(retval->GetName(),0));
//	}
//  else
//  {
//		result->Add(retval);
//    _sectionlist.push_back(std::pair<const char*,unsigned short>(retval->GetName(),result->_multi->size()-1));
//  }
//  return retval;
//}
//
//bool cINIstorage::RemoveSection(const char* name, unsigned int instance)
//{
//  if(!name || name[0] == '\0') return false; 
//  
//	khiter_t iterator = _sections.GetIterator(name);
//  INI_MultiValue<cINIsection>* result = _sections.Get(iterator);
//	if(!result) return false;
//
//  cINIsection* hold=result->Remove(instance);
//  if(!hold) return false;
//  delete hold;
//  if(!result->_single)
//  {
//    delete result;
//		_sections.RemoveIterator(iterator);
//  }
//
//  return true;
//}
//
//cINIentry& cINIstorage::EditEntry(const char* section, const char* data, const char* key, unsigned int instance)
//{
//  return EditEntry(section, (unsigned int)0, data, key, instance);
//}
//
//cINIentry& cINIstorage::EditEntry(const char* section, unsigned int sectioninstance, const char* data, const char* key, unsigned int instance)
//{
//  if(!section || section[0] == '\0') return *((cINIentry*)0);
//  cINIsection* sec = GetSection(section, sectioninstance);
//  if(!sec) sec = AddSection(section);
//  return sec->EditEntry(data, key, instance);
//}
//bool cINIstorage::RemoveEntry(const char *section, const char* key,unsigned int keyinstance, unsigned int instance)
//{
//  if(!section || section[0] == '\0') return false;
//  cINIsection* sec = GetSection(section, instance);
//  if(!sec) return false;
//  return sec->RemoveEntry(key, keyinstance);
//}
//
//bool cINIstorage::SaveINI(const char *file)
//{
//  FILE* f;
//  FOPEN(f, file, "wb");
//  if(!f) return false;
//  _saveINI(f);
//  fclose(f);
//  return true;
//}
//
//void cINIstorage::SaveINI(FILE* file)
//{
//  _saveINI(file);
//}
//
//void cINIstorage::_saveINI(FILE* file)
//{
//  INI_MultiValue<cINIsection>* cur;
//  cINIsection* hold;
//	unsigned int j;
//	unsigned int i=-1;
//	khiter_t iter;
//	_sections.ResetWalk();
//	while((iter=_sections.GetNext())!=_sections.End())
//	{
//		cur=_sections.Get(iter);
//		j=-1;
//		while(hold = cur->Get(++j))
//			hold->Write(file);
//		if(++i != 0 && i+1 != _sections.Length()) fwrite("\r\n\r\n",1,4,file);
//	}
//}
//
//const char* cINIstorage::GetPath() const
//{
//  return _path;
//}
//
////------------
//
//cINIsection::cINIsection(const cINIsection& copy) : cINIcomment(copy)
//{
//  memcpy(this,&copy,sizeof(cINIsection));
//
//  _name=copy.GetName();
//}
//
//cINIsection::cINIsection() : cINIcomment(0)
//{
//  _parent = 0;
//}
//
//cINIsection::cINIsection(const char* name, cINIstorage* parent, std::ostream* logger, const char* comment) : cINIcomment(comment)
//{
//  _logger = logger;
//  _name=name;
//
//  _parent = parent;
//}
//
//cINIsection::~cINIsection()
//{
//  INI_MultiValue<cINIentry>* cur;
//  cINIentry* hold;
//	unsigned int j;
//	khiter_t iter;
//	_entries.ResetWalk();
//	while((iter=_entries.GetNext())!=_entries.End())
//	{
//		cur=_entries.Get(iter);
//		j=-1;
//		while(hold = cur->Get(++j))
//			delete hold;
//		delete cur;
//    //_entries.Remove(iter);
//	}
//}
//
//cINIentry& cINIsection::GetEntry(const char *key, unsigned int instance) const
//{
//  return *GetEntryPtr(key,instance);
//}
//
//cINIentry* cINIsection::GetEntryPtr(const char *key, unsigned int instance) const
//{
//	INI_MultiValue<cINIentry>* result = _entries.GetKey(key);
//	if(!result) return 0;
//	return result->Get(instance);
//}
//
//cINIentry& cINIsection::EditEntry(const char *data, const char *key, unsigned int instance)
//{
//  if(!data || data[0] == '\0')
//  {
//    RemoveEntry(key, instance);
//    return *((cINIentry*)0);
//  }
//
//  cINIentry* get = GetEntryPtr(key, instance);
//  if(get)
//    get->SetData(data);
//  else
//    get = &AddEntry(key, data);
//  return *get;
//}
//
//cINIentry& cINIsection::AddEntry(const char* key, const char* data, const char* comment)
//{
//  if(!key || key[0] == '\0') return *((cINIentry*)0); //can't have a null name becuase that section already exists and is reserved for entries with no section
//
//  cINIentry* retval = new cINIentry(key, data, comment);
//  INI_MultiValue<cINIentry>* result = _entries.GetKey(key);
//  if(!result)
//	{
//		result = new INI_MultiValue<cINIentry>(retval, key); //Our hasher only takes POINTERS to strings, so we must store the name here in the collection, otherwise it'll pop in and out of existence
//		_entries.Insert(result->GetName(), result);
//	}
//  else
//		result->Add(retval);
//  return *retval;
//}
//
//bool cINIsection::RemoveEntry(const char* key, unsigned int instance)
//{
//  if(!key || key[0] == '\0') return false; 
//  
//	khiter_t iterator = _entries.GetIterator(key);
//  INI_MultiValue<cINIentry>* result = _entries.Get(iterator);
//	if(!result) return false;
//
//  cINIentry* hold=result->Remove(instance);
//  if(!hold) return false;
//  delete hold;
//  if(!result->_single)
//  {
//    delete result;
//		_entries.RemoveIterator(iterator);
//  }
//
//  return true;
//}
//
//const char* cINIsection::GetName() const
//{
//  return _name;
//}
//
//cINIentry& cINIsection::operator()(const char* key, unsigned int instance) const
//{
//  return *GetEntryPtr(key,instance);
//}
//
//cINIentry& cINIsection::operator[](const char* key) const
//{
//  return *GetEntryPtr(key,0);
//}
//
//cINIstorage* cINIsection::GetParent() const
//{
//  return _parent;
//}
//
//void cINIsection::Write(FILE* f)
//{
//  if(_name && _name[0] != '\0') //If this has no name, its the top inclusive section and we don't want to write a blank set of brackets.
//  {
//    fwrite("[",1,1,f);
//    fwrite(_name, 1, strlen(_name),f);
//    fwrite("]",1,1,f);
//    if(_comment && _comment[0] != '\0') //You can't append a comment to something that doesn't exist, which is why this included in the if scope
//    {
//      fwrite(" ;", 1, 2, f);
//      fwrite(_comment, 1, strlen(_comment), f);
//    }
//  }
//
//    INI_MultiValue<cINIentry>* cur;
//  cINIentry* hold;
//	unsigned int j;
//	khiter_t iter;
//	_entries.ResetWalk();
//	while((iter=_entries.GetNext())!=_entries.End())
//	{
//		cur=_entries.Get(iter);
//		j=-1;
//		while(hold = cur->Get(++j))
//    {
//      fwrite("\r\n",1,2,f);
//      hold->Write(f);
//    }
//	}
//}
//
//long cINIentry::GetLong() const
//{
//  return _lvalue;
//}
//
//const char* cINIentry::GetKey() const
//{
//  return _key;
//}
//
//const char* cINIentry::GetString() const
//{
//  return _svalue;
//}
//
//double cINIentry::GetDouble() const
//{
//  return _dvalue;
//}
//cINIentry::operator bool() const
//{
//  return !this?false:_lvalue != 0;
//}
//
//cINIentry::operator short() const
//{
//  return !this?0:(short)_lvalue;
//}
//
//cINIentry::operator int() const
//{
//  return !this?0:(int)_lvalue;
//}
//
//cINIentry::operator long() const
//{
//  return !this?0:_lvalue;
//}
//
//cINIentry::operator unsigned short() const
//{
//  return !this?0:(unsigned short)_lvalue;
//}
//
//cINIentry::operator unsigned int() const
//{
//  return !this?0:(unsigned int)_lvalue; //Because of the frequent issues with checking for a null pointer when dereferencing this, we actually check to see if the this pointer is null to save the programmer the ridiculous hassle
//}
//
//cINIentry::operator unsigned long() const
//{
//  return !this?0:(unsigned long)_lvalue;
//}
//
//cINIentry::operator float() const
//{
//  return !this?0:(float)_dvalue;
//}
//
//cINIentry::operator double() const
//{
//  return !this?0:_dvalue;
//}
//
//cINIentry::operator const char*() const
//{
//  return !this?(const char*)0:_svalue;
//}
//
//bool cINIentry::operator !=(cINIentry &other) const
//{
//  return other.GetDouble() != _dvalue || other.GetLong() != _lvalue || STRICMP(other.GetString(),_svalue) != 0;
//}
//
//bool cINIentry::operator ==(cINIentry &other) const
//{
//  return other.GetDouble() == _dvalue && other.GetLong() == _lvalue && STRICMP(other.GetString(),_svalue) == 0;
//}
//
//bool cINIentry::operator !=(cINIentry *other) const
//{
//  return other->GetDouble() != _dvalue || other->GetLong() != _lvalue || STRICMP(other->GetString(),_svalue) != 0;
//}
//
//bool cINIentry::operator ==(cINIentry *other) const
//{
//  return other->GetDouble() == _dvalue && other->GetLong() == _lvalue && STRICMP(other->GetString(),_svalue) == 0;
//}
//
//void cINIentry::Write(FILE* f)
//{
//  if(_key)
//  {
//    fwrite(_key, 1, strlen(_key), f);
//    fwrite(" = ", 1, 3, f);
//    if(_svalue) fwrite(_svalue, 1, strlen(_svalue), f);
//    if(_comment && _comment[0] != '\0')
//    {
//      fwrite(" ;", 1, 2, f);
//      fwrite(_comment, 1, strlen(_comment), f);
//    }
//  }
//  else
//    cINIcomment::Write(f);
//}