// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

//#define ENABLECHESS

#ifdef ENABLECHESS

#include <stdio.h>
#include "cThread.h"
//#include "cLocklessByteQueue.h"
#include "include/bss_win32_includes.h"

/* This is an implementation of the lockless queue using critical sections that mimic the atomic behavior of CAS, allowing Chess to debug the code */

#ifndef __C_LOCKLESS_QUEUE_H__

#include "bss_util.h"
#include <stdio.h>
#include <utility>
#include <stdlib.h>
//#include "ATOMIC.H"

CRITICAL_SECTION _critwrite;
CRITICAL_SECTION _critread;

namespace bss_util {
  /* This provides lockless queue functionality for two interacting threads using an expandable circular buffer */
  class cLocklessByteQueue
  {
  public:
    inline cLocklessByteQueue(unsigned int startsize);
    inline ~cLocklessByteQueue();
    inline std::pair<void*,unsigned int> ReadNextChunk(); //This will return chunk after consecutive chunk until there are none left, at which point it will return null. Each successive call will set the previous chunk as read.
    inline void FinishRead(); //Automatically called by ReadNextChunk()
    inline void* StartWrite(unsigned int size);
    inline void FinishWrite();
    inline void Clear();
    inline void DEBUGDUMP(const char* file) const;
    inline bool DEBUGVERIFY() const;

  protected:
    inline void _expand(unsigned int minsize);

    char* _nextwrite;
    char* _writebuf;
    char* _readstart; //marks the beginning of the current reserved read chunk 
    char* _readbuf;
    char* _endread;
    char* _mem;
    char* _memend;
    unsigned int _size;
    char* _curmemread; //read pointer to current buffer in case it gets superseded.
    char* _endmemread; //read pointer to end of current buffer
  };

  cLocklessByteQueue::cLocklessByteQueue(unsigned int startsize)
  {
    InitializeCriticalSection(&_critwrite);
    InitializeCriticalSection(&_critread);
    _mem=(char*)malloc(startsize+(sizeof(char*)*2));
    _size=startsize+sizeof(char*);
    _memend=_mem+_size+sizeof(char*);
    *((char**)_mem)=_memend;
    _mem+=sizeof(char*);
    _endread=0;
    _nextwrite=_writebuf=_curmemread=_mem;
    _endmemread=_memend;
    *(((char**)_memend)-1) = _mem;
    _readbuf=_readstart=(_memend-sizeof(char*));
    //_readbuf=_readstart=_mem;
    //_mem=*((char**)_readbuf);
  }

  cLocklessByteQueue::~cLocklessByteQueue()
  {
    Clear(); //gets rid of any lingering read buffer
    free((void*)(_mem-=sizeof(char*)));
    DeleteCriticalSection(&_critwrite);
    DeleteCriticalSection(&_critread);
  }

#define CHECKWRITEWALL if(prev==_endread) \
    { \
      _readbuf=prev;  \
      _endread=0;  \
      return std::pair<void*,unsigned int>((void*)0,0); \
    }

  std::pair<void*,unsigned int> cLocklessByteQueue::ReadNextChunk()
  {
    if(!_endread) {
      EnterCriticalSection(&_critwrite);
      _endread=_nextwrite; //if we haven't sampled _nextwrite yet we need to do so
      LeaveCriticalSection(&_critwrite);
    }
    else FinishRead(); //otherwise we have a read to finish
    //assert(_readstart==_readbuf);
    EnterCriticalSection(&_critread);
    char* prev=_readstart;
    LeaveCriticalSection(&_critread);
    _readbuf=*((char**)prev);
    CHECKWRITEWALL;

    if(_readbuf==_curmemread) //we circled around
    {
      prev=_curmemread;
      _readbuf=*((char**)prev);
      CHECKWRITEWALL;
    }
    if(!(_readbuf>_curmemread&&_readbuf<_endmemread)) //We hit a new memory block, so we have to delete our old one and move to the new one
    {
      //DEBUGDUMP("dump.txt");
      prev=_readbuf;
      free((void*)(_curmemread-=sizeof(char*)));
      _readbuf=*((char**)prev);
      _curmemread=prev;
      _endmemread=((char**)_curmemread)[-1];
    }
    CHECKWRITEWALL;

    return std::pair<void*,unsigned int>((void*)(prev+sizeof(char*)),(_readbuf-prev)-sizeof(char*)); //or if we haven't return this as another chunk to read
  }
  inline void cLocklessByteQueue::FinishRead()
  {
    if(_readbuf!=0) {
      //asmcas<char*>(&_readstart,_readbuf,_readstart);
      EnterCriticalSection(&_critread);
      _readstart=_readbuf;
      LeaveCriticalSection(&_critread);
      assert(_readstart==_readbuf);
    }
  }

  inline void* cLocklessByteQueue::StartWrite(unsigned int size)
  {
    EnterCriticalSection(&_critread);
    char* readptr = _readstart;
    LeaveCriticalSection(&_critread);
    EnterCriticalSection(&_critwrite);
    char* retval=_nextwrite; //_nextwrite has already been verified and evaluated
    LeaveCriticalSection(&_critwrite);
    char* check=retval+(sizeof(char*)*2)+size;

    if(readptr>=_mem&&readptr<_memend)//There are two possible situations - we are in a normal circle buffer, or we are currently expanding the buffer size.
    { //normal case
      if(check>=_memend) //wrap around
      {
        *((char**)retval) = _mem;
        retval=_mem;
        check=retval+(sizeof(char*)*2)+size;
      }
      if(retval<readptr && check>=readptr) //if we hit read pointer, trigger expansion
      {
        _expand(size+sizeof(char*));
        *((char**)retval) = _mem;
        retval=_mem;
      }
    }
    else if(check>=_memend) //We are currently expanding the buffer, so we will never hit the read wall, but when we hit the end we have to make a new buffer
    {
      _expand(size+sizeof(char*));
      *((char**)retval) = _mem;
      retval=_mem;
    } //If none of these trigger, we didn't hit anything so retval is valid and no changing is required

    //_writebuf=retval+sizeof(char*)+size;
    //*((char**)retval)=_writebuf;
    *((char**)retval)=retval+sizeof(char*)+size;
    _writebuf=*((char**)retval);
    retval+=sizeof(char*);

    return (void*)retval;
  }
  void cLocklessByteQueue::FinishWrite()
  {
    //asmcas<char*>(&_nextwrite,_writebuf,_nextwrite);
    EnterCriticalSection(&_critwrite);
    _nextwrite=_writebuf;
    LeaveCriticalSection(&_critwrite);
    assert(_nextwrite==_writebuf);
  }
  void cLocklessByteQueue::Clear() //This does not even try to be atomic in any way.
  {
    while(ReadNextChunk().first); //Forces a read to the write spot, clearing all hanging buffers

    *(((char**)_memend)-1) = _mem;
    _readbuf=(_memend-sizeof(char*));
    //asmcas<char*>(&_readstart,_readbuf,_readstart);
    _readstart=_readbuf;

    _writebuf=_mem;
    //asmcas<char*>(&_nextwrite,_writebuf,_nextwrite);
    _nextwrite=_writebuf;
    _curmemread=_mem;
    _endmemread=_memend;
  }
  inline void cLocklessByteQueue::_expand(unsigned int minsize)
  {
    _size*=2;
    if(_size<minsize) _size=minsize;
    _size+=sizeof(char*);
    _mem=(char*)malloc(_size+sizeof(char*));
    _memend=_mem+_size+sizeof(char*);
    *((char**)_mem)=_memend;
    _mem+=sizeof(char*);
  }

  inline bool cLocklessByteQueue::DEBUGVERIFY() const
  {
    DEBUGDUMP("dump.txt");
    char* start=_curmemread;
    char* end;

    while(true)
    {
      if(start==_readbuf) return true;
      end=*((char**)start);
      if(!(end>=_curmemread&&end<_endmemread)) break;
      start=end;
    }

    return false;
  }

  inline void cLocklessByteQueue::DEBUGDUMP(const char* file) const
  {
    FILE* f;
    fopen_s(&f,file,"w");

    char* start=_curmemread;
    char* end;
    while(true)
    {
      fprintf(f,"0x%p",start);
      if(start==_nextwrite) fputs(", _nextwrite",f);
      if(start==_writebuf) fputs(", _writebuf",f);
      if(start==_readstart) fputs(", _readstart",f);
      if(start==_readbuf) fputs(", _readbuf",f);
      if(start==_endread) fputs(", _endread",f);
      fputs("\n",f);
      end=*((char**)start);
      if(!(end>=_curmemread&&end<_endmemread)) break;
      start+=sizeof(char*);
      fwrite((void*)start,1,end-start,f);
      fputs("\n",f);
      start=end;
    }

    fclose(f);
  }
}

#endif

using namespace bss_util;

static const int NUM_RUNS=1;
static const int MAX_ALLOC=50;
unsigned int BSS_COMPILER_STDCALL dorandomcrap(void* arg)
{
  //Sleep(1); //lets the other thread catch up
  cLocklessByteQueue* args = (cLocklessByteQueue*)arg;

  int id;
  for(int i = 0; i < NUM_RUNS; ++i)
  {
    id=(i<<3)%MAX_ALLOC;
    memset(args->StartWrite(id),id,id);
    args->FinishWrite();
  }
  memset(args->StartWrite(MAX_ALLOC+1),MAX_ALLOC+1,MAX_ALLOC+1);
  args->FinishWrite();
  return 0;
}

extern "C" 
BSS_COMPILER_DLLEXPORT int ChessTestRun()
{
  cLocklessByteQueue qtest(512);
  bool expResult = true;

  cThread crapthread(&dorandomcrap);
  crapthread.Start(&qtest);
  
  std::pair<void*, unsigned int> hold;
  bool br=true;
  int num=-1;
  for(int i = 0; i < 2; ++i)
    while((hold=qtest.ReadNextChunk()).first!=0)
    {
      if(hold.second==MAX_ALLOC+1) { ++num; break; }
      else if(hold.second!=(((++num)<<3)%MAX_ALLOC))
        expResult=false;
    }
  
  crapthread.Join();
    while((hold=qtest.ReadNextChunk()).first!=0)
    {
      if(hold.second==MAX_ALLOC+1) { ++num; break; }
      else if(hold.second!=(((++num)<<3)%MAX_ALLOC))
        expResult=false;
    }
  //if(qtest.ReadNextChunk().first!=0) expResult=false; //This should have been done by now
  //if(expResult)
  //  printf("True\n");
  //else
  //  printf("**** False ****\n");

  return expResult ? 0 : -1;
}
#endif