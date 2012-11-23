// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_LOG_H__
#define __BSS_LOG_H__

#include <ostream>
#include <vector>
#include "bss_dlldef.h"
#include "bss_util.h"

#define BSSLOG(logger,level) ((logger).FORMATLOG<level>(__FILE__,__LINE__))

namespace bss_util { class StreamSplitter; }

// template defined error messages
template<unsigned char ERRLEVEL> struct bss_LOGERRLVL {};
template<> struct bss_LOGERRLVL<0> { inline static const char* ERRLVL() { return "INFO: "; } };
template<> struct bss_LOGERRLVL<1> { inline static const char* ERRLVL() { return "WARNING: "; } };
template<> struct bss_LOGERRLVL<2> { inline static const char* ERRLVL() { return "ERROR: "; } };
template<> struct bss_LOGERRLVL<3> { inline static const char* ERRLVL() { return "FATAL: "; } };
template<> struct bss_LOGERRLVL<4> { inline static const char* ERRLVL() { return "DEBUG: "; } };

// Log class that can be converted into a stream and redirected to various different stream targets
class BSS_DLLEXPORT bss_Log
{
public:
	// Move semantics only
	bss_Log(bss_Log&& mov);
	// Constructor - takes a stream and adds it
	explicit bss_Log(std::ostream* log=0);
	// Constructor - takes either a stream or a file (or both) and adds them
	explicit bss_Log(const char* logfile, std::ostream* log=0);
	bss_Log(const wchar_t* logfile, std::ostream* log);
  // Destructor - destroys any file streams
  ~bss_Log();
  // Redirects an existing stream to write to this log's buffer
  void BSS_FASTCALL Assimilate(std::ostream& stream); //Resistance is futile
  // Adds a target stream to post logs to
  void BSS_FASTCALL AddTarget(std::ostream& stream);
  //void BSS_FASTCALL AddTarget(std::wostream& stream);
  void BSS_FASTCALL AddTarget(const char* file);
  void BSS_FASTCALL AddTarget(const wchar_t* file);
  // Clears all targets and closes all files
  void ClearTargets();
  // Gets the stream for this log
  inline std::ostream& GetStream() { return _stream; }

  bss_Log& operator=(bss_Log&& right);
  inline operator std::ostream&() { return _stream; }
  
  template<unsigned char errlevel>
  inline std::ostream& BSS_FASTCALL FORMATLOG(const char* file, unsigned int line) { return FORMATLOGLEVEL(bss_LOGERRLVL<errlevel>::ERRLVL(),file,line); }
  inline std::ostream& BSS_FASTCALL FORMATLOGLEVEL(const char* level, const char* file, unsigned int line)
  {
		_stream << '[';
    _writedatetime(_tz,_stream,true);
    _stream << "] (" << _trimpath(file) << ':' << line << ") " << level;
    return _stream;
  }

private:
	bss_Log(const bss_Log& copy);
  bss_Log& operator=(const bss_Log& right);
  static bool BSS_FASTCALL _writedatetime(long timezone, std::ostream& log, bool timeonly);
  static const char* BSS_FASTCALL _trimpath(const char* path);
  //static const wchar_t* BSS_FASTCALL _trimpath(const wchar_t* path);

  bss_util::StreamSplitter* _split;
  long _tz;
#pragma warning(push)
#pragma warning(disable:4251)
#ifdef BSS_COMPILER_GCC // Until GCC fixes its fucking broken standard implementation, we're going to have to do this the hard way.
  std::vector<std::unique_ptr<std::ofstream>> _files;
#else
  std::vector<std::ofstream> _files;
#endif
  std::vector<std::pair<std::ostream&,std::streambuf*>> _backup;
  std::ostream _stream;
#pragma warning(pop)
};

#endif
