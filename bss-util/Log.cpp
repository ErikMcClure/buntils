// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/bss_log.h"
#include "bss-util/bss_stream.h"
#include "bss-util/bss_util.h"
#include <fstream>
#include <iomanip>

using namespace bss;
using namespace std;

const char* Log::DEFAULTFORMAT = "{4} [{0}] ({1}:{2}) {3}";
const char* Log::DEFAULTNULLFORMAT = "{4} ({1}:{2}) {3}";

Log::Log(Log&& mov) : _levels(std::move(mov._levels)), _split(mov._split), _tz(GetTimeZoneMinutes()), _files(std::move(mov._files)),
  _backup(std::move(mov._backup)), _stream(_split), _maxlevel(mov._maxlevel), _format(mov._format), _nullformat(mov._nullformat)
{
  mov._split = 0;
}
Log::Log(std::ostream* log) : _levels(6), _split(new StreamSplitter()), _tz(GetTimeZoneMinutes()), _stream(_split), _maxlevel(127),
  _format(DEFAULTFORMAT), _nullformat(DEFAULTNULLFORMAT)
{
  _levelDefaults();
  if(log != 0)
    AddTarget(*log);
}
Log::Log(const char* logfile, std::ostream* log) : _levels(6), _split(new StreamSplitter()), _tz(GetTimeZoneMinutes()), _stream(_split),
  _maxlevel(127), _format(DEFAULTFORMAT), _nullformat(DEFAULTNULLFORMAT)
{
  _levelDefaults();
  AddTarget(logfile);
  if(log != 0)
    AddTarget(*log);
}
#ifdef BSS_PLATFORM_WIN32
Log::Log(const wchar_t* logfile, std::ostream* log) : _levels(6), _split(new StreamSplitter()), _tz(GetTimeZoneMinutes()), _stream(_split),
  _maxlevel(127), _format(DEFAULTFORMAT), _nullformat(DEFAULTNULLFORMAT)
{
  _levelDefaults();
  AddTarget(logfile);
  if(log != 0)
    AddTarget(*log);
}
#endif
Log::~Log()
{
  ClearTargets();
  for(size_t i = 0; i < _backup.size(); ++i) //restore stream buffer backups so we don't blow up someone else's stream when destroying ourselves
    _backup[i].first.rdbuf(_backup[i].second);
  if(_split != 0) delete _split;
}
void Log::Assimilate(std::ostream& stream)
{
  _backup.push_back(std::pair<std::ostream&, std::streambuf*>(stream, stream.rdbuf()));
  if(_split != 0) stream.rdbuf(_split);
}
void Log::AddTarget(std::ostream& stream)
{
  if(_split != 0) _split->AddTarget(&stream);
}
void Log::AddTarget(const char* file)
{
  if(!file) return;
#ifdef BSS_COMPILER_GCC 
  _files.push_back(std::unique_ptr<std::ofstream>(new ofstream(BSSPOSIX_WCHAR(file), ios_base::out | ios_base::trunc)));
  AddTarget(*_files.back());
#else
  _files.push_back(ofstream(BSSPOSIX_WCHAR(file), ios_base::out | ios_base::trunc));
  AddTarget(_files.back());
#endif
}
#ifdef BSS_PLATFORM_WIN32
void Log::AddTarget(const wchar_t* file)
{
  if(!file) return;
  _files.push_back(ofstream(file, ios_base::out | ios_base::trunc));
  AddTarget(_files.back());
}
#endif

void Log::ClearTargets()
{
  if(_split != 0) _split->ClearTargets();
  for(size_t i = 0; i < _files.size(); ++i)
#ifdef BSS_COMPILER_GCC 
    _files[i]->close();
#else
    _files[i].close();
#endif
  _files.clear();
}

void Log::SetFormat(const char* format)
{
  _format = format;
}

void Log::SetNullFormat(const char* format)
{
  _nullformat = format;
}

void Log::SetLevel(uint8_t level, const char* str)
{
  if(_levels.Capacity() >= level)
    _levels.SetCapacity(level + 1);
  _levels[level] = str;
}
void Log::SetMaxLevel(uint8_t level)
{
  _maxlevel = level;
}

bool Log::_writeDateTime(long timez, std::ostream& log, bool timeonly)
{
  time_t rawtime;
  TIME64(&rawtime);
  tm stm;
  tm* ptm = &stm;
  if(GMTIMEFUNC(&rawtime, ptm) != 0)
    return false;

  long m = ptm->tm_hour * 60 + ptm->tm_min + 1440 + timez; //+1440 ensures this is never negative because % does not properly respond to negative numbers.
  long h = ((m / 60) % 24);
  m %= 60;

  char logchar = log.fill();
  std::streamsize logwidth = log.width();
  log.fill('0');
  //Write date if we need to
  if(!timeonly) log << std::setw(4) << ptm->tm_year + 1900 << std::setw(1) << '-' << std::setw(2) << ptm->tm_mon + 1 << std::setw(1) << '-' << std::setw(2) << ptm->tm_mday << std::setw(1) << ' ';
  //Write time and flush stream
  log << std::setw(1) << h << std::setw(0) << ':' << std::setw(2) << m << std::setw(0) << ':' << std::setw(2) << ptm->tm_sec;

  //Reset values of stream
  log.width(logwidth);
  log.fill(logchar);

  return true;
}
Log& Log::operator=(Log&& right)
{
  _tz = GetTimeZoneMinutes();
  _files = std::move(right._files);
  _backup = std::move(right._backup);
  _split = right._split;
  right._split = 0;
  _stream.rdbuf(_split);
  return *this;
}
const char* Log::_trimpath(const char* path)
{
  const char* r = strrchr(path, '/');
  const char* r2 = strrchr(path, '\\');
  r = bssmax(r, r2);
  return (!r) ? path : (r + 1);
}
void Log::_levelDefaults()
{
  SetLevel(0, "FATAL: ");
  SetLevel(1, "ERROR: ");
  SetLevel(2, "WARNING: ");
  SetLevel(3, "NOTICE: ");
  SetLevel(4, "INFO: ");
  SetLevel(5, "DEBUG: ");
}
int Log::PrintLogV(const char* source, const char* file, uint32_t line, int8_t level, const char* format, va_list args)
{
  if(level >= _maxlevel)
    return 0;
  LogHeader(source, file, line, level);

  va_list vltemp;
  va_copy(vltemp, args);
  size_t _length = (size_t)CSTR_CT<char>::VPCF(format, vltemp) + 1; // If we didn't copy vl here, it would get modified by vsnprintf and blow up.
  va_end(vltemp);
  DYNARRAY(char, buf, _length);
  int r = CSTR_CT<char>::VPF(buf, _length, format, args);
  _stream << buf << std::endl;
  return r;
}
void Log::_header(std::ostream& o, int n, const char* source, const char* file, uint32_t line, const char* level, long tz)
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

std::ostream& Log::_logHeader(const char* source, const char* file, uint32_t line, const char* level)
{
  file = _trimpath(file);
  __safeFormat<const char*, const char*, uint32_t, const char*, long>::F<&_header>(_stream, ((!source && _nullformat != 0) ? _nullformat : _format), source, file, line, level, _tz);
  return _stream;
}