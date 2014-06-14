// Copyright ©2014 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_LOG_H__
#define __BSS_LOG_H__

#include <ostream>
#include <vector>
#include "bss_util.h"
#include "cArray.h"

#define BSSLOG(logger,level) ((logger).FORMATLOG(level,__FILE__,__LINE__))
#define BSSLOGV(logger,level,...) ((logger).WriteLog(level,__FILE__,__LINE__,__VA_ARGS__))

namespace bss_util {
  class StreamSplitter;

  // Log class that can be converted into a stream and redirected to various different stream targets
  class BSS_DLLEXPORT cLog
  {
    cLog(const cLog& copy) BSS_DELETEFUNC
    cLog& operator=(const cLog& right) BSS_DELETEFUNCOP
  public:
    // Move semantics only
    cLog(cLog&& mov);
    // Constructor - takes a stream and adds it
    explicit cLog(std::ostream* log=0);
    // Constructor - takes either a stream or a file (or both) and adds them
    explicit cLog(const char* logfile, std::ostream* log=0);
#ifdef BSS_PLATFORM_WIN32
    cLog(const wchar_t* logfile, std::ostream* log);
#endif
    // Destructor - destroys any file streams
    ~cLog();
    // Redirects an existing stream to write to this log's buffer
    void BSS_FASTCALL Assimilate(std::ostream& stream); // Resistance is futile
    // Adds a target stream to post logs to
    void BSS_FASTCALL AddTarget(std::ostream& stream);
    //void BSS_FASTCALL AddTarget(std::wostream& stream);
    void BSS_FASTCALL AddTarget(const char* file);
#ifdef BSS_PLATFORM_WIN32
    void BSS_FASTCALL AddTarget(const wchar_t* file);
#endif
    // Clears all targets and closes all files
    void ClearTargets();
    // Gets the stream for this log
    inline std::ostream& GetStream() { return _stream; }
    // Sets a level string (which should be a constant, not something that will get deallocated)
    void BSS_FASTCALL SetLevel(unsigned char level, const char* str);

    cLog& operator=(cLog&& right);
    inline operator std::ostream&() { return _stream; }

#ifdef BSS_VARIADIC_TEMPLATES
    template<typename... Args>
    BSS_FORCEINLINE void BSS_FASTCALL WriteLog(unsigned char level, const char* file, unsigned int line, Args... args) { _writelog(FORMATLOGLEVEL(_levels[level], file, line), args...); }
#endif
    BSS_FORCEINLINE std::ostream& BSS_FASTCALL FORMATLOG(unsigned char level, const char* file, unsigned int line) { return FORMATLOGLEVEL(_levels[level], file, line); }
    inline std::ostream& BSS_FASTCALL FORMATLOGLEVEL(const char* level, const char* file, unsigned int line)
    {
      _stream << '[';
      _writedatetime(_tz, _stream, true);
      _stream << "] (" << _trimpath(file) << ':' << line << ") " << level;
      return _stream;
    }

  protected:
#ifdef BSS_VARIADIC_TEMPLATES
    template<typename Arg, typename... Args>
    static inline void _writelog(std::ostream& s, Arg arg, Args... args) { s << arg; _writelog(s, args...); }
    static inline void _writelog(std::ostream& s) { s << std::endl; }
#endif
    static bool BSS_FASTCALL _writedatetime(long timezone, std::ostream& log, bool timeonly);
    static const char* BSS_FASTCALL _trimpath(const char* path);
    void _leveldefaults();

    WArray<const char*, unsigned char>::t _levels;
    StreamSplitter* _split;
    long _tz;
#pragma warning(push)
#pragma warning(disable:4251)
#ifdef BSS_COMPILER_GCC // Until GCC fixes its fucking broken standard implementation, we're going to have to do this the hard way.
    std::vector<std::unique_ptr<std::ofstream>> _files;
#else
    std::vector<std::ofstream> _files;
#endif
    std::vector<std::pair<std::ostream&, std::streambuf*>> _backup;
    std::ostream _stream;
#pragma warning(pop)
  };
}

#endif
