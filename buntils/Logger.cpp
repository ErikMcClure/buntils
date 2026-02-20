// Copyright ©2018 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "buntils/buntils.h"
#include "buntils/Logger.h"
#include "buntils/stream.h"
#include <fstream>
#include <iomanip>

using namespace bun;
using namespace std;

const char* Logger::DEFAULTFORMAT     = "{4} [{0}] ({1}:{2}) {3}";
const char* Logger::DEFAULTNULLFORMAT = "{4} ({1}:{2}) {3}";

Logger::Logger(Logger&& mov) :
  _levels(std::move(mov._levels)),
  _split(mov._split),
  _tz(GetTimeZoneMinutes()),
  _files(std::move(mov._files)),
  _backup(std::move(mov._backup)),
  _stream(_split),
  _maxlevel(mov._maxlevel),
  _format(mov._format),
  _nullformat(mov._nullformat)
{
  mov._split = 0;
}
Logger::Logger(std::ostream* log) :
  _levels(6),
  _split(new StreamSplitter()),
  _tz(GetTimeZoneMinutes()),
  _stream(_split),
  _maxlevel(127),
  _format(DEFAULTFORMAT),
  _nullformat(DEFAULTNULLFORMAT)
{
  _levelDefaults();
  if(log != 0)
    AddTarget(*log);
}
Logger::Logger(const char* logfile, std::ostream* log) :
  _levels(6),
  _split(new StreamSplitter()),
  _tz(GetTimeZoneMinutes()),
  _stream(_split),
  _maxlevel(127),
  _format(DEFAULTFORMAT),
  _nullformat(DEFAULTNULLFORMAT)
{
  _levelDefaults();
  AddTarget(logfile);
  if(log != 0)
    AddTarget(*log);
}
#ifdef BUN_PLATFORM_WIN32
Logger::Logger(const wchar_t* logfile, std::ostream* log) :
  _levels(6),
  _split(new StreamSplitter()),
  _tz(GetTimeZoneMinutes()),
  _stream(_split),
  _maxlevel(127),
  _format(DEFAULTFORMAT),
  _nullformat(DEFAULTNULLFORMAT)
{
  _levelDefaults();
  AddTarget(logfile);
  if(log != 0)
    AddTarget(*log);
}
#endif
Logger::~Logger()
{
  ClearTargets();
  // restore stream buffer backups so we don't blow up someone else's stream when destroying ourselves
  for(auto [out, buf] : _backup)
  {
    out.rdbuf(buf);
  }
  if(_split != 0)
    delete _split;
}
void Logger::Assimilate(std::ostream& stream)
{
  _backup.push_back(std::pair<std::ostream&, std::streambuf*>(stream, stream.rdbuf()));
  if(_split != 0)
    stream.rdbuf(_split);
}
void Logger::AddTarget(std::ostream& stream)
{
  if(_split != 0)
    _split->AddTarget(&stream);
}
void Logger::AddTarget(const char* file)
{
  if(!file)
    return;
#ifdef BUN_COMPILER_GCC
  _files.push_back(std::unique_ptr<std::ofstream>(new ofstream(BUNPOSIX_WCHAR(file), ios_base::out | ios_base::trunc)));
  AddTarget(*_files.back());
#else
  _files.push_back(ofstream(BUNPOSIX_WCHAR(file), ios_base::out | ios_base::trunc));
  AddTarget(_files.back());
#endif
}
#ifdef BUN_PLATFORM_WIN32
void Logger::AddTarget(const wchar_t* file)
{
  if(!file)
    return;
  _files.push_back(ofstream(file, ios_base::out | ios_base::trunc));
  AddTarget(_files.back());
}
#endif

void Logger::ClearTargets()
{
  if(_split != 0)
    _split->ClearTargets();
  for(auto& file : _files)
#ifdef BUN_COMPILER_GCC
    file->close();
#else
    file.close();
#endif
  _files.clear();
}

void Logger::SetFormat(const char* format) { _format = format; }

void Logger::SetNullFormat(const char* format) { _nullformat = format; }

void Logger::SetLevel(uint8_t level, const char* str)
{
  if(_levels.Capacity() >= level)
    _levels.SetCapacity(level + 1);
  _levels[level] = str;
}
void Logger::SetMaxLevel(uint8_t level) { _maxlevel = level; }

bool Logger::_writeDateTime(long timez, std::ostream& log, bool timeonly)
{
  time_t rawtime;
  TIME64(&rawtime);
  tm stm;
  tm* ptm = &stm;
  if(GMTIMEFUNC(&rawtime, ptm) != 0)
    return false;

  long m = ptm->tm_hour * 60 + ptm->tm_min + 1440 +
           timez; //+1440 ensures this is never negative because % does not properly respond to negative numbers.
  long h = ((m / 60) % 24);
  m %= 60;

  char logchar             = log.fill();
  std::streamsize logwidth = log.width();
  log.fill('0');
  // Write date if we need to
  if(!timeonly)
    log << std::setw(4) << ptm->tm_year + 1900 << std::setw(1) << '-' << std::setw(2) << ptm->tm_mon + 1 << std::setw(1)
        << '-' << std::setw(2) << ptm->tm_mday << std::setw(1) << ' ';
  // Write time and flush stream
  log << std::setw(1) << h << std::setw(0) << ':' << std::setw(2) << m << std::setw(0) << ':' << std::setw(2)
      << ptm->tm_sec;

  // Reset values of stream
  log.width(logwidth);
  log.fill(logchar);

  return true;
}
Logger& Logger::operator=(Logger&& right)
{
  _tz          = GetTimeZoneMinutes();
  _files       = std::move(right._files);
  _backup      = std::move(right._backup);
  _split       = right._split;
  right._split = 0;
  _stream.rdbuf(_split);
  return *this;
}
const char* Logger::_trimPath(const char* path)
{
  const char* r  = strrchr(path, '/');
  const char* r2 = strrchr(path, '\\');
  r              = std::max(r, r2);
  return (!r) ? path : (r + 1);
}
void Logger::_levelDefaults()
{
  SetLevel(0, "FATAL: ");
  SetLevel(1, "ERROR: ");
  SetLevel(2, "WARNING: ");
  SetLevel(3, "NOTICE: ");
  SetLevel(4, "INFO: ");
  SetLevel(5, "DEBUG: ");
}
int Logger::PrintLogV(const char* source, const char* file, size_t line, int8_t level, const char* format, va_list args)
{
  if(level >= _maxlevel)
    return 0;
  LogHeader(source, file, line, level);

  va_list vltemp;
  va_copy(vltemp, args);
  size_t _length = (size_t)internal::STR_CT<char>::VPCF(format, vltemp) +
                   1; // If we didn't copy vl here, it would get modified by vsnprintf and blow up.
  va_end(vltemp);
  VARARRAY(char, buf, _length);
  int r = internal::STR_CT<char>::VPF(buf.data(), _length, format, args);
  _stream << buf.data() << std::endl;
  return r;
}
void Logger::_header(std::ostream& o, int n, const char* source, const char* file, size_t line, const char* level, long tz)
{
  switch(n)
  {
  case 0: o << source; break;
  case 1: o << file; break;
  case 2: o << line; break;
  case 3: o << level; break;
  case 4: _writeDateTime(tz, o, true); break;
  case 5: _writeDateTime(tz, o, false); break;
  }
}

std::ostream& Logger::_logHeader(const char* source, const char* file, size_t line, const char* level)
{
  file = _trimPath(file);
  std::vformat_to(std::ostream_iterator<char>(_stream), ((!source && _nullformat != 0) ? _nullformat : _format),
                  std::make_format_args(source, file, line, level, _tz));
  return _stream;
}