// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/bss_util.h"
#include "bss-util/INIstorage.h"
#include "bss-util/INIparse.h"
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <string.h>

using namespace bss;

INIsection INIstorage::_sectionsentinel;
LocklessBlockPolicy<INIstorage::_NODE> INIstorage::_alloc;

template<typename T>
const T* ltrimstr(const T* str)
{
  for(; *str > 0 && *str < 33; ++str);
  return str;
}

template<typename T>
const T* rtrimstr(const T* end, const T* begin)
{
  for(; end > begin && *end < 33; --end);
  return end;
}

void INIstorage::_openINI()
{
  Str targetpath = _path;
  targetpath += _filename;
  FILE* f;
  FOPEN(f, targetpath, "a+b"); //this will create the file if it doesn't already exist
  if(!f) return;
  fseek(f, 0, SEEK_END);
  size_t size = (size_t)ftell(f);
  fseek(f, 0, SEEK_SET);
  _ini = new Str();
  _ini->resize(size);
  size = fread(_ini->UnsafeString(), sizeof(char), size, f); //reads in the entire file
  _ini->UnsafeString()[size] = '\0';
  _ini->RecalcSize();
  fclose(f);
}

INIstorage::INIstorage(const INIstorage& copy) : _path(copy._path), _filename(copy._filename),
  _ini(!copy._ini ? 0 : new Str(*copy._ini)), _logger(copy._logger), _root(0), _last(0)
{
  _copy(copy);
}

INIstorage::INIstorage(INIstorage&& mov) : _path(std::move(mov._path)), _filename(std::move(mov._filename)), _ini(mov._ini),
  _logger(mov._logger), _sections(std::move(mov._sections)), _root(mov._root), _last(mov._last)
{
  mov._ini = 0;
  mov._root = 0;
  mov._last = 0;
}
INIstorage::INIstorage(const char* file, std::ostream* logger) : _ini(0), _logger(logger), _root(0), _last(0)
{
  FILE* f;
  FOPEN(f, file, ("rt"));
  if(f)
  {
    _loadINI(f);
    fclose(f);
  }
  _setFilePath(file);
}
INIstorage::INIstorage(FILE* file, std::ostream* logger) : _ini(0), _logger(logger), _root(0), _last(0)
{
  _loadINI(file);
}

// Destructor
INIstorage::~INIstorage()
{
  if(_ini) delete _ini;
  _destroy(); //_destroyhash checks for nullification
}
INIstorage::_NODE* INIstorage::GetSectionNode(const char* section, size_t instance) const
{
  _NODE* n = _sections[section];
  if(!n) return 0;
  if(!instance) return n;
  return (instance > n->instances.Capacity()) ? 0 : (n->instances[instance - 1]);
}
INIsection* INIstorage::GetSection(const char* section, size_t instance) const
{
  _NODE* sec = GetSectionNode(section, instance);
  return !sec ? 0 : &sec->val;
}
size_t INIstorage::GetNumSections(const char* section) const
{
  _NODE* n = _sections[section];
  if(!n) return 0;
  return n->instances.Capacity() + 1;
}
INIentry* INIstorage::GetEntryPtr(const char *section, const char* key, size_t keyinstance, size_t secinstance) const
{
  INIsection* s = GetSection(section, secinstance);
  return !s ? 0 : s->GetEntryPtr(key, keyinstance);
}
INIsection::_NODE* INIstorage::GetEntryNode(const char *section, const char* key, size_t keyinstance, size_t secinstance) const
{
  INIsection* s = GetSection(section, secinstance);
  return !s ? 0 : s->GetEntryNode(key, keyinstance);
}

INIsection& INIstorage::AddSection(const char* name)
{
  if(!_ini) _openINI();
  _ini->reserve(_ini->size() + strlen(name) + 4);
  char c;
  size_t i;
  for(i = _ini->size(); i > 0;)
  {
    c = _ini->GetChar(--i);
    if(!(c == ' ' || c == '\n' || c == '\r')) { ++i; break; }
  }
  if(i > 0)
  {
    _ini->UnsafeString()[i] = '\0';
    _ini->RecalcSize();
    _ini->append(("\n\n"));
  }
  _ini->append(("["));
  _ini->append(name);
  _ini->append(("]"));
  return *_addSection(name);
}

INIsection* INIstorage::_addSection(const char* name)
{
  _NODE* p = _alloc.allocate(1);
  bssFill(*p, 0);
  khiter_t iter = _sections.Iterator(name);

  if(iter == _sections.Back())
  {
    new (&p->val) INIsection(name, this, 0);
    _sections.Insert(p->val.GetName(), p);
    if(!_root)
      _root = _last = p;
    else
      _last = LLAddAfter(p, _last);
  }
  else
  {
    assert(_last != 0 && _root != 0 && _sections.ExistsIter(iter));
    _NODE* r = _sections.Value(iter);
    _NODE* t = !r->instances.Capacity() ? r : r->instances.Back();
    LLInsertAfter(p, t, _last);
    r->instances.Insert(p, r->instances.Capacity());
    new (&p->val) INIsection(name, this, r->instances.Capacity()); // done down here because the index is actually where it is in the array + 1.
  }

  return &p->val;
}

//3 cases: removing something other then the beginning, removing the beginning, or removing the last one
bool INIstorage::RemoveSection(const char* name, size_t instance)
{
  if(!_ini) _openINI();
  INICHUNK chunk = bssFindINISection(*_ini, _ini->size(), name, instance);
  khiter_t iter = _sections.Iterator(name);

  if(iter != _sections.Back() && chunk.start != 0)
  {
    _NODE* secnode = _sections.Value(iter);
    _NODE* secroot = secnode;
    if(instance > secnode->instances.Capacity()) return false; //if keyinstance is not valid, fail
    if(instance != 0)
      secnode = secnode->instances[instance - 1];

    // If you are deleting a root node that has danglers, we just replace the root with one of the danglers and transform it into a dangler case.
    if(!instance && secnode->instances.Capacity() > 0)
    {
      instance = 1;
      secnode->val = std::move(secnode->next->val);
      secnode->val._index = 0;
      secnode = secnode->next;
    }

    LLRemove(secnode, _root, _last);
    if(!instance) // If this is true you are a root, but you don't have any danglers (see above check), so we just remove you from the hash.
      _sections.RemoveIter(iter);
    else
    { // Otherwise you are a dangler, so all we have to do is remove you from the instances array
      secroot->instances.Remove(--instance); // we decrement instance here so its valid in the below for loop
      for(; instance < secroot->instances.Capacity(); ++instance) --secroot->instances[instance]->val._index; //moves all the indices down
    }

    secnode->~_NODE(); // Calling this destructor is important in case the node has an unused array that needs to be freed
    _alloc.deallocate(secnode, 1);
    _ini->erase((const char*)chunk.start - _ini->c_str(), ((const char*)chunk.end - (const char*)chunk.start) + 1);
    return true;
  }
  return false;
}

char INIstorage::EditEntry(const char* section, const char* key, const char* nvalue, size_t keyinstance, size_t secinstance)
{
  if(!_ini) _openINI();
  if(secinstance == (size_t)~0)
    secinstance = AddSection(section)._index;

  khiter_t iter = _sections.Iterator(section);
  if(iter == _sections.Back()) return -1; //if it doesn't exist at this point, fail
  _NODE* secnode = _sections.Value(iter);
  //if(secinstance==(size_t)~0) secinstance=secnode->instances.Capacity()-1; //This is now done at the start of the function
  if(secinstance > secnode->instances.Capacity()) return -2; //if secinstance is not valid, fail
  if(secinstance > 0)
    secnode = secnode->instances[secinstance - 1];

  INIsection* psec = &secnode->val;
  INICHUNK chunk = bssFindINISection(_ini->c_str(), _ini->size(), section, secinstance);
  if(!chunk.start) return -3; //if we can't find it in the INI, fail

  if(keyinstance == (size_t)~0) //insertion
  {
    psec->_addEntry(key, nvalue);
    //_ini->reserve(_ini->size()+strlen(key)+strlen(nvalue)+2); // This is incredibly stupid. It invalidates all our pointers causing strange horrifying bugs.
    Str construct = StrF("\n%s=%s", key, !nvalue ? ("") : nvalue); //build keyvalue string
    //const char* peek=(const char*)chunk.end-1; //DEBUG
    const char* ins = strchr((const char*)chunk.end - 1, '\n');
    if(!ins) //end of file
    {
      _ini->append(construct);
      return 0;
    }

    _ini->insert(ins - _ini->c_str(), construct);
    return 0;
  } //If it wasn't an insert we need to find the entry before the other two possible cases

  typedef INIsection::_NODE _SNODE; //This makes things a bit easier

  iter = psec->_entries.Iterator(key);
  if(iter == psec->_entries.Back()) return -4; //if key doesn't exist at this point, fail
  _SNODE* entnode = psec->_entries.Value(iter);
  _SNODE* entroot = entnode;
  if(keyinstance > entnode->instances.Capacity()) return -5; //if keyinstance is not valid, fail
  if(keyinstance != 0)
    entnode = entnode->instances[keyinstance - 1];
  chunk = bssFindINIEntry(chunk, key, keyinstance);
  if(!chunk.start) return -7; //if we can't find it

  if(!nvalue) //deletion
  {
    // If you are deleting a root node that has danglers, we just replace the root with one of the danglers and transform it into a dangler case.
    if(!keyinstance && entnode->instances.Capacity() > 0)
    {
      keyinstance = 1;
      entnode->val = std::move(entnode->next->val);
      entnode = entnode->next;
    }

    LLRemove(entnode, psec->_root, psec->_last);
    if(!keyinstance) // If this is true you are a root, but you don't have any danglers (see above check), so we just remove you from the hash.
      psec->_entries.RemoveIter(iter);
    else // Otherwise you are a dangler, so all we have to do is remove you from the instances array
      entroot->instances.Remove(keyinstance - 1);

    entnode->~_SNODE(); // Calling this destructor is important in case the node has an unused array that needs to be freed
    INIsection::_alloc.deallocate(entnode, 1);
    const char* end = strchr((const char*)chunk.end, '\n'); // Now we remove it from the actual INI
    end = !end ? (const char*)chunk.end : end + 1;
    _ini->erase((const char*)chunk.start - _ini->c_str(), end - (const char*)chunk.start);
  }
  else //edit
  {
    entnode->val.Set(nvalue);
    const char* start = strchr((const char*)chunk.start, '=');
    if(!start) return -6; //if this happens something is borked
    //start=ltrimstr(++start);
    ++start;
    _ini->replace(start - _ini->c_str(), (rtrimstr(((const char*)chunk.end) - 1, start) - start) + 1, nvalue);
  }

  return 0;
}
void INIstorage::EndINIEdit(const char* overridepath)
{
  if(!_ini) return;
  if(overridepath != 0) _setFilePath(overridepath);
  Str targetpath = _path;
  targetpath += _filename;
  FILE* f;
  FOPEN(f, targetpath, ("wb"));
  if(!f) return; // IF the file fails, bail out and do not discard the edit.
  fwrite(*_ini, sizeof(char), _ini->size(), f);
  fclose(f);
  DiscardINIEdit();
}
void INIstorage::DiscardINIEdit()
{
  if(_ini) delete _ini;
  _ini = 0;
}

void INIstorage::_loadINI(FILE* f)
{
  INIParser parse;
  bssInitINI(&parse, f);
  INIsection* cursec = 0;
  while(bssParseLine(&parse) != 0)
  {
    if(parse.newsection)
      cursec = _addSection(parse.cursection);
    else
    {
      if(!cursec) cursec = _addSection((""));
      cursec->_addEntry(parse.curkey, parse.curvalue);
    }
  }

  //if(parse.newsection) //We don't check for empty sections
  //  cursec=_addSection(parse.cursection); 

  bssDestroyINI(&parse);
}
void INIstorage::_setFilePath(const char* file)
{
  const char* hold = strrchr(file, '/');
  hold = !hold ? file : hold + 1;
  const char* hold2 = strrchr(hold, '\\');
  hold = !hold2 ? hold : hold2 + 1;
  _filename = hold;
  size_t length = hold - file;
  _path.resize(length + 1); // resize only accounts for null terminator if it feels like it
  memcpy(_path.UnsafeString(), file, length);
  _path.UnsafeString()[length] = '\0';
  _path.RecalcSize();
}
void INIstorage::_destroy()
{
  _NODE* t;
  while(_root)
  {
    t = _root->next;
    _root->~_NODE();
    _alloc.deallocate(_root, 1);
    _root = t;
  }
  _last = 0;
}

void INIstorage::_copy(const INIstorage& copy)
{
  assert(!_root && !_last);
  _NODE* t = copy._root;
  _NODE* last = 0;
  size_t c = 0;
  while(t)
  {
    _NODE* p = _alloc.allocate(1);
    bssFill(*p, 0);
    new (&p->val) INIsection(t->val);
    if(!_root)
      _root = _last = p;
    else
      _last = LLAddAfter(p, _last);

    if(t->instances.Capacity() != 0)
    {
      _sections.Insert(p->val.GetName(), p);
      p->instances.SetCapacity(c = t->instances.Capacity());
      last = t;
      --c;
    }
    else if(c > 0)
    {
      assert(last != 0);
      last->instances[last->instances.Capacity() - (c--) - 1] = p; //This never goes negative because c>0 and is therefore at least 1
    }
    else
      _sections.Insert(p->val.GetName(), p);

    t = t->next;
  }
}

INIstorage& INIstorage::operator=(const INIstorage& right)
{
  if(&right == this) return *this;
  _path = right._path;
  _filename = right._filename;
  if(_ini != 0) delete _ini;
  _ini = !right._ini ? 0 : new Str(*right._ini);
  _logger = right._logger;
  _destroy();
  _sections.Clear();
  _copy(right);
  return *this;
}
INIstorage& INIstorage::operator=(INIstorage&& mov)
{
  if(&mov == this) return *this;
  _path = std::move(mov._path);
  _filename = mov._filename;
  if(_ini != 0) delete _ini;
  _ini = mov._ini;
  _root = mov._root;
  _last = mov._last;
  mov._ini = 0;
  mov._root = 0;
  mov._last = 0;
  _logger = mov._logger;
  _destroy();
  _sections = std::move(mov._sections);
  return *this;
}

/*
void INIstorage<wchar_t>::_openINI()
{
  StrW targetpath=_path;
  targetpath+=_filename;
  FILE* f;
  WFOPEN(f,targetpath,L"a+b"); //this will create the file if it doesn't already exist
  if(!f) return;
  fseek(f,0,SEEK_END);
  unsigned long size=ftell(f);
  fseek(f,0,SEEK_SET);
  Str buf(size+1);
  size=fread(buf.UnsafeString(),sizeof(char),size,f); //reads in the entire file
  buf.UnsafeString()[size]='\0';
  size=UTF8Decode2BytesUnicode(buf,0);
  _ini = new StrW();
  _ini->resize(size);
  UTF8Decode2BytesUnicode(buf,_ini->UnsafeString());
  _ini->UnsafeString()[size]='\0';
  _ini->RecalcCapacity();
  fclose(f);
}*/
