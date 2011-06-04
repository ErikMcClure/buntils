// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"
//
//#ifndef __C_BUCKET_ALLOC_H__
//#define __C_BUCKET_ALLOC_H__
//
//#include "cRBT_list.h"
//#include "cArraySort.h"
//#include "bss_dlldef.h"
//#include <assert.h>
//
//namespace bss_util
//{
//  typedef cRBT_ListNode<unsigned short,unsigned short> BUCKETMEM_NODETYPE;
//  typedef Allocator<BUCKETMEM_NODETYPE, FixedSizeAllocPolicy<BUCKETMEM_NODETYPE>> BUCKETMEM_ALLOCATOR_TYPE;
//  class cBucketAlloc;
//
//  /* All functions here are expanded since this is a required class and I don't want to force a .lib include unless absolutely necessary */
//  struct __declspec(dllexport) BUCKET_MEMALLOC
//  {
//    inline BUCKET_MEMALLOC(unsigned short size, BUCKETMEM_ALLOCATOR_TYPE* alloc, cBucketAlloc* parent);
//    inline ~BUCKET_MEMALLOC();
//    inline void* BSS_FASTCALL alloc(unsigned short halfbytes);
//    inline void BSS_FASTCALL dealloc(void* pointer);
//
//    BUCKET_MEMALLOC* _next;
//    cRBT_ListNode<unsigned short,BUCKET_MEMALLOC*>* _node;
//    inline bool ValidatePointer(void* ptr) const { return ptr>(void*)_memblock&&ptr<(void*)_memend; }
//
//    typedef unsigned short BUCKET_SlotSize;
//
//  protected:
//    cRBT_List<unsigned short,unsigned short,CompareKeys<unsigned short>,BUCKETMEM_ALLOCATOR_TYPE> _freeslots; //key is free space, data is offset
//    //cRBT_List<unsigned short,unsigned short,CompareKeys<unsigned short>> _freeslots; //key is free space, data is offset
//    cArraySort<unsigned int, CompareShortsTraits<unsigned int, RefTraits<unsigned int>>, BUCKET_SlotSize> _slotoffsets; //sorted list of all the free offsets. The offset is in the low order and the length is in the high order
//    short* _memblock; //min block size is two bytes
//    short* _memend; //stored for quick validation of pointers
//    cBucketAlloc* _parent;
//  };
//
//  class __declspec(dllexport) cBucketAlloc
//  {
//    typedef cRBT_ListNode<unsigned short,BUCKET_MEMALLOC*> B_RBTNODE;
//
//  public:
//    inline cBucketAlloc(unsigned int startsize=512);
//    inline ~cBucketAlloc();
//    inline void* BSS_FASTCALL alloc(unsigned int totalbytes);
//    inline bool BSS_FASTCALL dealloc(void* pointer);
//
//  protected:
//    friend struct BUCKET_MEMALLOC;
//    inline void BSS_FASTCALL _addbucket();
//
//    BUCKET_MEMALLOC* _root;
//    cRBT_List<unsigned short,BUCKET_MEMALLOC*,CompareKeys<unsigned short>,Allocator<B_RBTNODE, FixedSizeAllocPolicy<B_RBTNODE>>> _bucketsort;
//    //cRBT_List<unsigned short,BUCKET_MEMALLOC*,CompareKeys<unsigned short>> _bucketsort;
//    BUCKETMEM_ALLOCATOR_TYPE _allocator;
//    unsigned short _nextsize;
//  };
//
//  BUCKET_MEMALLOC::BUCKET_MEMALLOC(unsigned short size, BUCKETMEM_ALLOCATOR_TYPE* alloc, cBucketAlloc* parent) :
//  _freeslots(alloc), _next(0), _memblock((short*)malloc(size*2)), _parent(parent), _slotoffsets(size>>5)
//  {
//    _memend=_memblock+size; //Do not multiply size by two here, since _memblock is short*, not byte*
//    _freeslots.Insert(size, 0);
//    _slotoffsets.Insert(size<<16);
//  }
//  BUCKET_MEMALLOC::~BUCKET_MEMALLOC()
//  {
//    free(_memblock);
//  }
//  void* BSS_FASTCALL BUCKET_MEMALLOC::alloc(unsigned short halfbytes)
//  {
//    BUCKETMEM_NODETYPE* slot = _freeslots.GetNear(++halfbytes); //we increment this to provide room to hold its size
//    assert(slot);
//    
//    void* retval = (void*)(_memblock+slot->_data+1); //because _memblock is of type short*, it effectively multiplies _data by two
//    *(((short*)retval)-1) = halfbytes;
//    unsigned short slotfreespace = slot->GetKey();
//
//    assert(slotfreespace >= halfbytes);
//    if(slotfreespace == halfbytes) //if this is true its an exact match so just remove this altogether
//    {
//      _slotoffsets.Remove(_slotoffsets.Find(slot->_data));
//      _freeslots.Remove(slot);
//    }
//    else //otherwise just edit it
//    {
//      BUCKET_SlotSize index = _slotoffsets.Find(slot->_data);
//      slot->_data+=halfbytes;
//      assert(index!=(BUCKET_SlotSize)-1);
//
//      _slotoffsets[index] = slot->_data + ((slotfreespace-halfbytes)<<16); //we can do this without re-sorting because we know it can't possibly infringe on the other offsets
//      _freeslots.ReplaceKey(slot,slotfreespace-halfbytes); //this works because whatever slot we used was the first available one
//    }
//    
//    _parent->_bucketsort.ReplaceKey(_node,_freeslots.GetLast()->GetKey()); //the last one is the highest value
//    return retval;
//  }
//  void BSS_FASTCALL BUCKET_MEMALLOC::dealloc(void* pointer)
//  {
//    unsigned short offset = ((short*)pointer)-_memblock-1; //gets the offset so we can check if we need to extend or meld two free locations
//    unsigned short length=*(((short*)pointer)-1); //gets the length
//    BUCKET_SlotSize find = _slotoffsets.FindNearest(offset); //we use this to figure out our immediate area. If we're right next to a free slow we don't want to create a new one, and in some cases we actually fill a hole in the free spacee and end up removing one
//    unsigned int prev;
//
//    bool store;
//    BUCKET_SlotSize slotoffsetlength=_slotoffsets.GetLength();
//    if(store=find<slotoffsetlength) prev=_slotoffsets[find];
//    if(store && ((unsigned short)prev)+(prev>>16)==offset) //we are linked to our previous one NOTE: This may be an off by one error
//    {
//      unsigned int next;
//      if(store=++find<slotoffsetlength) next=_slotoffsets[find];
//      if(store && (unsigned short)(next)==offset+length) //in this case we are sandwiched between two free zones and we need to remove one
//      {
//        BUCKETMEM_NODETYPE* freeslot = _freeslots.Get(next>>16);
//        while(freeslot->_data != (unsigned short)next)
//          freeslot=freeslot->GetNext();
//
//        _slotoffsets.Remove(find);
//        _slotoffsets[--find] += (length<<16)+(next&0xFFFF0000); //this works because Remove() replaces the index with the one from in front
//        _freeslots.Remove(freeslot);
//
//        freeslot = _freeslots.Get(prev>>16);
//        while(freeslot->_data != (unsigned short)prev)
//          freeslot=freeslot->GetNext();
//        _freeslots.ReplaceKey(freeslot,_slotoffsets[find]>>16);
//      }
//      else //here we are only linked to one behind us so we just edit that one
//      {
//        BUCKETMEM_NODETYPE* freeslot = _freeslots.Get(prev>>16);
//        while(freeslot->_data != (unsigned short)prev)
//          freeslot=freeslot->GetNext();
//
//        _freeslots.ReplaceKey(freeslot,length+=freeslot->GetKey());
//        _slotoffsets[--find]=freeslot->_data+(length<<16);
//      }
//    }
//    else if(++find<slotoffsetlength && (unsigned short)(prev=_slotoffsets[find]) == offset+length) //in this case we are linked to one in front and only one in front so we just modify that
//    {
//      BUCKETMEM_NODETYPE* freeslot = _freeslots.Get(prev>>16);
//      while(freeslot->_data != (unsigned short)prev)
//        freeslot=freeslot->GetNext();
//
//      freeslot->_data = offset;
//      _freeslots.ReplaceKey(freeslot,length+=freeslot->GetKey());
//      _slotoffsets[find]=freeslot->_data+(length<<16);
//    }
//    else //otherwise we are in the middle of nowhere and must add a new free chunk
//    {
//      _slotoffsets.Insert(offset+(length<<16));
//      _freeslots.Insert(length,offset);
//    }
//
//    _parent->_bucketsort.ReplaceKey(_node,_freeslots.GetLast()->GetKey());
//  }
//
//  cBucketAlloc::cBucketAlloc(unsigned int startsize) : _nextsize(startsize), _root(0)
//  {
//  }
//  cBucketAlloc::~cBucketAlloc()
//  {
//    BUCKET_MEMALLOC* next;
//    while(_root)
//    {
//      next=_root->_next;
//      delete _root;
//      _root=next;
//    }
//  }
//  void BSS_FASTCALL cBucketAlloc::_addbucket()
//  {
//    if(!_root)
//    {
//      _root= new BUCKET_MEMALLOC(_nextsize, &_allocator, this);
//      _root->_node = _bucketsort.Insert(_nextsize,_root);
//    }
//    else
//    {
//      BUCKET_MEMALLOC* nextalloc = _root;    
//      while(nextalloc->_next) nextalloc=nextalloc->_next;
//      nextalloc->_next = new BUCKET_MEMALLOC((!(_nextsize<<=1)?_nextsize=65534:_nextsize), &_allocator, this);
//      nextalloc->_next->_node = _bucketsort.Insert(_nextsize,nextalloc->_next);
//    }
//  }
//
//  void* BSS_FASTCALL cBucketAlloc::alloc(unsigned int totalbytes)
//  {
//    if(totalbytes>65534*2)
//      return 0;//malloc(totalbytes); //65535 is the max count of unsigned int
//    totalbytes= (totalbytes>>1) + (totalbytes&0x01); //x&0x01 is the same as x%2
//    cRBT_ListNode<unsigned short,BUCKET_MEMALLOC*>* res;
//    while(!(res = _bucketsort.GetNear((unsigned short)(totalbytes+1), true)))
//      _addbucket();
//
//    return res->_data->alloc(totalbytes);
//  }
//  bool BSS_FASTCALL cBucketAlloc::dealloc(void* pointer)
//  {
//    BUCKET_MEMALLOC* curalloc = _root;
//    while(!curalloc->ValidatePointer(pointer))
//    {
//      curalloc = curalloc->_next;
//      if(!curalloc) return false;
//       //free(pointer); return; } //if we can't find it it was probably malloc'd
//    }
//    curalloc->dealloc(pointer);
//    return true;
//  }
//}
//
//#endif