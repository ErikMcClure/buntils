// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#if defined(DEBUG) || defined(_DEBUG) || defined(BSS_LEAKTEST_RELEASEMODE) //debug only unless overriden
#ifndef __BSS_LEAKTEST_H__
#define __BSS_LEAKTEST_H__

#include "cKhash.h"
#include "BSS_Log.h"
#include <stdio.h>

typedef struct
{
  size_t size;
  char* file;
  int line;
  void* ptr;
} BSS_LEAKINFO;

class cBSS_LeakTracker : BSS_Log
{
public:
  cBSS_LeakTracker()
  {
    fopen_s(&f,"memleaks.txt","w");
  }
  ~cBSS_LeakTracker()
  {
    fputs("\n--- Memory leaks---", f);
    _leakinfo.ResetWalk();
    khiter_t curiter;
    BSS_LEAKINFO* pinfo;
    while((curiter=_leakinfo.GetNext())!=_leakinfo.End())
    {
      pinfo = _leakinfo.Get(curiter);
      fprintf(f, "%p (Size: %i) leaked at %s:%i\n", pinfo->ptr, pinfo->size, pinfo->file, pinfo->line);
      free((void*)pinfo->file);
      free(pinfo); //Honestly it doesn't matter if we leak these or not since the process is about to explode but its nice to be tidy.
    }

    fclose(f);
  }

  void Add(void* ptr, size_t size, const char* file, int line)
  {
    const char* hold=strrchr(file,'\\');
    file=strrchr(file,'/');
    file=(file>hold)?file : hold;

    if(_leakinfo.Exists(_leakinfo.GetIterator(ptr))) //ensure there is no conflict, because if there is somethign else terribly wrong is going on.
      throw "DUPLICATE ASSIGNMENT ERROR";

    size_t length=strlen(file);
    BSS_LEAKINFO* pinfo = (BSS_LEAKINFO*)malloc(sizeof(BSS_LEAKINFO)); 
    pinfo->size=size;
    pinfo->file=(char*)malloc(++length);
    pinfo->line=line;
    pinfo->ptr=ptr;
    memcpy(pinfo->file, file, length);
    _leakinfo.Insert(ptr,pinfo);
  }
  void Remove(void* ptr, const char* file, int line)
  {
    BSS_LEAKINFO* pinfo = _leakinfo.Remove(ptr);
    if(!pinfo)
    {
      fprintf(f, "Attemped to delete unassigned memory location (%p) at %s:%i\n", ptr, file, line);
      return;
    }
  }

protected:
  bss_util::cKhash_Pointer<BSS_LEAKINFO*> _leakinfo;
  FILE* f;
};

static cBSS_LeakTracker bss_leaktrack;
static const char* del_file_hold;
static int del_line_hold;

void* operator new(size_t size, const char* file, int line)
{
  void* retval = malloc(size);
  bss_leaktrack.Add(retval, size, file, line);
  return retval;
}


void* operator new[](size_t size, const char *file, int line) 
{
  return operator new(size, file, line); //This shouldn't work. Are the constructors called elsewhere?
}

void operator delete(void* ptr) 
{
  bss_leaktrack.Remove(ptr, del_file_hold,del_line_hold);
  free(ptr);
}

void operator delete[](void* ptr)
{
  operator delete(ptr);
}

void deletep(const char* file, int line)
{
  del_file_hold = file;
  del_line_hold = line;
}

#define new new(__FILE__, __LINE__)
#define delete deletep(__FILE__, __LINE__), delete
#define BSS_LEAKTEST_ACTIVE

#endif
#endif