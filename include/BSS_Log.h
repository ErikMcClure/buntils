// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_LOG_H__
#define __BSS_LOG_H__

#include "bss_dlldef.h"
#include "bss_traits.h"

#define BSSLOG(logger,level,message) (logger)->WriteLog(message, level, __FILE__,__LINE__)
#define BSSLOGONE(logger,level,format,arg1) (logger)->WriteLog(format,level,  __FILE__,__LINE__,arg1)
#define BSSLOGTWO(logger,level,format,arg1,arg2) (logger)->WriteLog(format, level, __FILE__,__LINE__,arg1,arg2)

enum __declspec(dllexport) BSS_Log_Levels : unsigned char
{
	BSS_LOG_NOTICE=0,
	BSS_LOG_WARNING=1,
	BSS_LOG_ERROR=2,
	BSS_LOG_FATAL=3
};

//We do a crapload of predefs here because this is a logger header and thus must be included anywere you want to log something. To avoid massive unnecessary header inclusion we do a few predefining tricks here.
namespace std
{
	template<class _Elem> struct char_traits;
	template<class _Elem, class _Traits> class basic_ostream;
	typedef basic_ostream<char, char_traits<char>> ostream;
  typedef basic_ostream<wchar_t, char_traits<wchar_t>> wostream;
	template<class _Elem, class _Traits > class basic_filebuf;
	typedef basic_filebuf<char, char_traits<char>> filebuf;
  typedef basic_filebuf<wchar_t, char_traits<wchar_t>> wfilebuf;
  template<class _Elem,class _Traits> class basic_fstream;
  typedef basic_fstream<char, char_traits<char>> fstream;
  typedef basic_fstream<wchar_t, char_traits<wchar_t>> wfstream;
  template<class _Ty>	class allocator;
  template<class _Ty,class _Ax> class vector;
  template<class _Elem,class _Traits,class _Ax>	class basic_string;
}

namespace bss_util
{
  template<class T>
  bool PointerCompare(const T* a, const T* b);
	template<class T, class Traits, typename SizeType>
  class cLinkedArray;
  template<class T, typename SizeType>
  class cArraySimple;
  template<typename T>
  class StandardAllocPolicy;
  template<typename T>
  class ObjectTraits;
  template<typename T, typename Policy, typename Traits>
	class Allocator;
}

template<typename T, typename Alloc>
class cStrT;

/* Standard Logging stream handling class. You may initialize this with an existing stream, tell it to create a file stream of its own, or initialize it with nothing (in which case all logs will be ignored) */
typedef class BSS_DLLEXPORT BSS_Log
{
  typedef unsigned char IDVAL;
public:
	/* Constructor - copies streams from another log */
	BSS_Log(const BSS_Log& copy);
	/* Constructor - takes a stream and adds it */
	BSS_Log(std::ostream* log=0);
	/* Constructor - takes either a stream or a file (or both) and adds them */
	BSS_Log(const char* logfile, std::ostream* log=0);
	BSS_Log(const wchar_t* logfile, std::ostream* log=0);
	/* Destructor - closes any streams associated with this log. Any further attempts to write to this log will cause undefined behavoir */
  ~BSS_Log();
	/* Writes a line to the log. format is a standard printf string, level is 0-3 (Notice, Warning, Error, and FATAL ERROR), file is __FILE__ and line is __LINE__. This function returns 0 if successful, 1 if the level did not pass the filter, -1 if the arguments are invalid and -2 if an internal error occured. */
	char BSS_FASTCALL WriteLog(const char* format, unsigned char level, const char* file, int line, ...);
	char BSS_FASTCALL WriteLog(const wchar_t* format, unsigned char level, const wchar_t* file, int line, ...);
	char BSS_FASTCALL WriteLog(const wchar_t* format, unsigned char level, const char* file, int line, ...); //handy dandy shortcut function
	/* Adds a file stream to write to */
	IDVAL BSS_FASTCALL AddSource(const char* file);
	IDVAL BSS_FASTCALL AddSource(const wchar_t* file);
	/* Adds an external stream to write to */
	IDVAL BSS_FASTCALL AddSource(std::ostream& stream);
	//IDVAL BSS_FASTCALL AddSource(std::wostream& stream);
	/* Removes a stream */
	bool BSS_FASTCALL RemoveSource(IDVAL ID);
	/* Sets message filter. Any log messages with an error level below this are ignored. Set to 0 (off) by default */
	void BSS_FASTCALL SetFilter(unsigned char level);
	/* Gets message filter */
	unsigned char GetFilter() const;
	/* Gets specified stream */
	std::ostream* BSS_FASTCALL GetStream(IDVAL ID) const;
	//std::wostream* BSS_FASTCALL GetStreamW(IDVAL ID) const;
	/* Gets stream this logger was initialized with, if any */
	std::ostream* GetInitStream() const;
	//std::wostream* GetInitStreamW() const;
  /* Returns a file stream for the given path. Do not delete the returned pointer */
  std::ostream* BSS_FASTCALL GetFileStream(const char* logfile);
  std::ostream* BSS_FASTCALL GetFileStream(const wchar_t* logfile);
  /* Lets you name this logger so log messages from different modules can be differentiated */
  void BSS_FASTCALL NameModule(const char* name);
  void BSS_FASTCALL NameModule(const wchar_t* name);

	BSS_Log& operator=(const BSS_Log& right);
	
  /* Writes the current time according to the given time zone offset (UTC would be +0, United States Eastern would be -6, China would be +8 etc.) to a stream. */
  static bool BSS_FASTCALL WriteTime(char timezone, std::ostream* log); //HH:MM:SS
  /* Writes the current time according to the given time zone offset (UTC would be +0, United States Eastern would be -6, China would be +8 etc.) to a stream. */
  static bool BSS_FASTCALL WriteDateTime(char timezone, std::ostream* log); //YYYY-MM-DD HH:MM:SS
	/* Gets system time zone */
	//static char BSS_FASTCALL GetTimeZone();

  static const int NUMERRLEVELS=5;

protected:
	static const char* _errlevels[NUMERRLEVELS];
	//static const wchar_t* _werrlevels[NUMERRLEVELS];
  static bool BSS_FASTCALL _writedatetime(char timezone, std::ostream* log, bool timeonly);
  //static bool BSS_FASTCALL _writedatetime(char timezone, std::wostream* log, bool timeonly);
	static const char* BSS_FASTCALL _trimpath(const char* path);
	static const wchar_t* BSS_FASTCALL _trimpath(const wchar_t* path);
  char BSS_FASTCALL _writelog(const wchar_t* format, unsigned char level, const wchar_t* file, int line, char* args);

	char _curtimez;
	unsigned char _cutofflevel;
	IDVAL _init;
	IDVAL _winit;

  typedef cStrT<char,std::allocator<char>> TBUFVAL;
  //typedef bss_util::cArraySimple<char,unsigned int> TBUFVAL;
  typedef bss_util::cLinkedArray<std::ostream*,bss_util::ValueTraits<std::ostream*>,IDVAL> TSTREAMVAL;
  //typedef bss_util::cLinkedArray<std::wostream*,bss_util::ValueTraits<std::wostream*>,IDVAL> TWSTREAMVAL;
  typedef std::vector<std::fstream,std::allocator<std::fstream>> TFILESTREAMVAL;
  //typedef std::vector<std::wfstream,std::allocator<std::wfstream>> TWFILESTREAMVAL;
  
	TSTREAMVAL& _streams;
	//TWSTREAMVAL& _wstreams;
  TFILESTREAMVAL& _filestreams;
  //TWFILESTREAMVAL& _wfilestreams;
  TBUFVAL& _buf;
  cStrT<wchar_t,std::allocator<wchar_t>>& _wname;
  cStrT<char,std::allocator<char>>& _name;
} BSS_LOG;

#endif