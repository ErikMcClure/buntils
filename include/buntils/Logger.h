// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BUN_LOG_H__
#define __BUN_LOG_H__

#include "buntils.h"
#include "Array.h"
#include <ostream>
#include <vector>
#include <stdarg.h>
#include <format>
#include <iterator>

#define BUNLOG(logger,level,...) ((logger).Log(0,__FILE__,__LINE__,(level),__VA_ARGS__))

namespace bun {
  class StreamSplitter;

  // Log class that can be converted into a stream and redirected to various different stream targets
  class BUN_DLLEXPORT Logger
  {
    Logger(const Logger& copy) = delete;
    Logger& operator=(const Logger& right) = delete;

  public:
    // Move semantics only
    Logger(Logger&& mov);
    // Constructor - takes a stream and adds it
    explicit Logger(std::ostream* log = 0);
    // Constructor - takes either a stream or a file (or both) and adds them
    explicit Logger(const char* logfile, std::ostream* log = 0);
#ifdef BUN_PLATFORM_WIN32
    Logger(const wchar_t* logfile, std::ostream* log);
#endif
    // Destructor - destroys any file streams
    ~Logger();
    // Redirects an existing stream to write to this log's buffer
    void Assimilate(std::ostream& stream);
    // Adds a target stream to post logs to
    void AddTarget(std::ostream& stream);
    //void AddTarget(std::wostream& stream);
    void AddTarget(const char* file);
#ifdef BUN_PLATFORM_WIN32
    void AddTarget(const wchar_t* file);
#endif
    // Sets the format for the beginning of the log entry. Defaults to "[{4}] {0} ({1}:{2}) {3}"
    void SetFormat(const char* format);
    // An optional format for log entries with a null source. Defaults to "[{4}] ({1}:{2}) {3}"
    void SetNullFormat(const char* format);
    // Clears all targets and closes all files
    void ClearTargets();
    // Gets the stream for this log
    inline std::ostream& GetStream() { return _stream; }
    // Sets a level string (which should be a constant, not something that will get deallocated)
    void SetLevel(uint8_t level, const char* str);
    // Sets the maximum level that will be logged. Useful for excluding unnecessary debug logs from release builds
    void SetMaxLevel(uint8_t level);

    Logger& operator=(Logger&& right);
    inline operator std::ostream&() { return _stream; }

    inline int PrintLog(const char* source, const char* file, size_t line, int8_t level, const char* format, ...)
    {
      va_list vl;
      va_start(vl, format);
      int r = PrintLogV(source, file, line, level, format, vl);
      va_end(vl);
      return r;
    }
    int PrintLogV(const char* source, const char* file, size_t line, int8_t level, const char* format, va_list args);

    template<typename... Args>
    BUN_FORCEINLINE void Log(const char* source, const char* file, size_t line, int8_t level, Args... args)
    {
      if(level >= _maxlevel)
        return;
      _writeLog(LogHeader(source, file, line, level), args...);
    }
    template<typename... Args>
    BUN_FORCEINLINE void LogFormat(const char* source, const char* file, size_t line, int8_t level, const char* format, Args... args)
    {
      if(level >= _maxlevel)
        return;

      std::vformat_to(std::ostream_iterator<char>(LogHeader(source, file, line, level)), format, std::make_format_args(source, file, line, level, _tz));
      _stream << std::endl;
    }
    BUN_FORCEINLINE std::ostream& LogHeader(const char* source, const char* file, size_t line, int8_t level)
    {
      assert((level < 0) || (level < _levels.Capacity()));
      return _logHeader(source, file, line, (level < 0) ? "" : _levels[level]);
    }
    static const char* _trimPath(const char* path);

    static const char* DEFAULTFORMAT;
    static const char* DEFAULTNULLFORMAT;

  protected:
    template<typename Arg, typename... Args>
    static inline void _writeLog(std::ostream& o, Arg arg, Args... args) { o << arg; _writeLog(o, args...); }
    static inline void _writeLog(std::ostream& o) { o << std::endl; }
    std::ostream& _logHeader(const char* source, const char* file, size_t line, const char* level);
    static void _header(std::ostream& o, int n, const char* source, const char* file, size_t line, const char* level, long tz);
    static bool _writeDateTime(long timezone, std::ostream& log, bool timeonly);
    void _levelDefaults();

    Array<const char*, uint8_t> _levels;
    int8_t _maxlevel;
    StreamSplitter* _split;
    const char* _format;
    const char* _nullformat;
    long _tz;
#pragma warning(push)
#pragma warning(disable:4251)
    std::vector<std::pair<std::ostream&, std::streambuf*>> _backup;
    std::ostream _stream;

#ifdef BUN_COMPILER_GCC // Until GCC fixes its fucking broken standard implementation, we're going to have to do this the hard way.
    std::vector<std::unique_ptr<std::ofstream>> _files;
#else
    std::vector<std::ofstream> _files;
#endif
#pragma warning(pop)
  };
}

#endif
