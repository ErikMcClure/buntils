// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss_Log.h"
#include "bss_deprecated.h"
#include "StreamSplitter.h"
#include "bss_util.h"
#include <fstream>
#include <iomanip>

using namespace bss_util;
using namespace std;

bss_Log::bss_Log(const bss_Log& copy) : _split(new StreamSplitter(*copy._split)), _stream(_split), _tz(GetTimeZoneMinutes())
{
}
bss_Log::bss_Log(std::ostream* log) : _split(new StreamSplitter()), _stream(_split), _tz(GetTimeZoneMinutes())
{
  if(log!=0)
    AddTarget(*log);
}
bss_Log::bss_Log(const char* logfile, std::ostream* log) : _split(new StreamSplitter()), _stream(_split), _tz(GetTimeZoneMinutes())
{
  AddTarget(logfile);
  if(log!=0)
    AddTarget(*log);
}
bss_Log::bss_Log(const wchar_t* logfile, std::ostream* log) : _split(new StreamSplitter()), _stream(_split), _tz(GetTimeZoneMinutes())
{
  AddTarget(logfile);
  if(log!=0)
    AddTarget(*log);
}
bss_Log::~bss_Log()
{
  ClearTargets();
  for(size_t i = 0; i < _backup.size(); ++i) //restore stream buffer backups so we don't blow up someone else's stream when destroying ourselves (suicide bombing is bad for your health)
    _backup[i].first.rdbuf(_backup[i].second);
  delete _split;
}
void BSS_FASTCALL bss_Log::Assimilate(std::ostream& stream)
{
  _backup.push_back(std::pair<std::ostream&, std::streambuf*>(stream,stream.rdbuf()));
  stream.rdbuf(_split);
}
void BSS_FASTCALL bss_Log::AddTarget(std::ostream& stream)
{
  _split->AddTarget(&stream);
}
void BSS_FASTCALL bss_Log::AddTarget(const char* file)
{
  if(!file) return;
  _files.push_back(ofstream(cStrW(file).c_str(),ios_base::out|ios_base::trunc));
  AddTarget(_files.back());
}
void BSS_FASTCALL bss_Log::AddTarget(const wchar_t* file)
{
  if(!file) return;
  _files.push_back(ofstream(file,ios_base::out|ios_base::trunc));
  AddTarget(_files.back());
}
void bss_Log::ClearTargets()
{
  _split->ClearTargets();
  for(size_t i = 0; i < _files.size(); ++i)
    _files[i].close();
  _files.clear();
}
bool BSS_FASTCALL bss_Log::_writedatetime(long timez, std::ostream& log, bool timeonly)
{
  TIMEVALUSED rawtime;
  FTIME(&rawtime);
  tm stm;
  tm* ptm=&stm;
  GMTIMEFUNC(&rawtime, ptm);

  long m=ptm->tm_hour*60 + ptm->tm_min + 1440 + timez; //+1440 ensures this is never negative because % does not properly respond to negative numbers.
  long h=((m/60)%24);
  m%=60;

  char logchar = log.fill();
  std::streamsize logwidth = log.width();
  log.fill('0');
  //Write date if we need to
  if(!timeonly) log << std::setw(4) << ptm->tm_year+1900 << std::setw(1) << '-' << std::setw(2) << ptm->tm_mon+1 << std::setw(1) << '-' << std::setw(2) << ptm->tm_mday << std::setw(1) << ' ';
  //Write time and flush stream
  log << std::setw(1) << h << std::setw(0) << ':' << std::setw(2) << m << std::setw(0) << ':' << std::setw(2) << ptm->tm_sec;
  
  //Reset values of stream
  log.width(logwidth);
  log.fill(logchar);

  return true;
}
bss_Log& bss_Log::operator=(const bss_Log& right)
{
  ClearTargets();
  *_split=*right._split;

  return *this; //We don't copy _stream because its attached to _split which gets copied anyway and we want it to stay that way.
}

const char* BSS_FASTCALL bss_Log::_trimpath(const char* path)
{
	const char* r=strrchr(path,'/');
	const char* r2=strrchr(path,'\\');
  r=bssmax(r,r2);
  return (!r)?path:(r+1);
}

//const wchar_t* BSS_FASTCALL bss_Log::_trimpath(const wchar_t* path)
//{
//	const wchar_t* retval=wcsrchr(path,'/');
//	if(!retval)
//	{
//		retval=wcsrchr(path,'\\');
//		if(!retval) retval=path;
//		else ++retval;
//	}
//	else
//	{
//		path=wcsrchr(path,'\\');
//		retval=path>retval?++path:++retval;
//	}
//	return retval;
//}

/*
#include "BSS_Log.h"
#include "bss_deprecated.h"
#include <fstream>
#include <stdarg.h> //va_start() va_end()
#include <time.h>
#include <iomanip> //setw
#include "cLinkedArray.h"
#include "cStr.h"

const char* BSS_Log::_errlevels[NUMERRLEVELS] = { "INFO: ", "WARNING: ", "ERROR: ", "FATAL: ", "DEBUG: " };
//const wchar_t* BSS_Log::_werrlevels[NUMERRLEVELS] = { L"INFO: ", L"WARNING: ", L"ERROR: ", L"FATAL: ", L"DEBUG: " };

BSS_Log::BSS_Log(const BSS_Log& copy) : _curtimez(0), _winit(copy._winit),_init(copy._init), _wname(*new cStrW()), _name(*new cStr()), _cutofflevel(copy._cutofflevel), _streams(*(new TSTREAMVAL(copy._streams))), _filestreams(*(new TFILESTREAMVAL())),  _buf(*(new TBUFVAL(32)))
{
  NameModule(copy._wname);
}
BSS_Log::BSS_Log(std::ostream* log) : _winit(-1),_curtimez(0), _wname(*new cStrW()), _name(*new cStr()), _streams(*(new TSTREAMVAL())), _filestreams(*(new TFILESTREAMVAL())),_buf(*(new TBUFVAL(32)))
{
	_cutofflevel=0;
	_init=!log?(IDVAL)-1:AddSource(*log);
}
BSS_Log::BSS_Log(const char* logfile, std::ostream* log) : _winit(-1),_curtimez(0), _wname(*new cStrW()), _name(*new cStr()), _streams(*(new TSTREAMVAL())), _filestreams(*(new TFILESTREAMVAL())),  _buf(*(new TBUFVAL(32)))
{
	_cutofflevel=0;
	IDVAL hold=!log?(IDVAL)-1:AddSource(*log);
	_init=AddSource(logfile);
	_init=(_init==(IDVAL)-1)?hold:_init;
}
BSS_Log::BSS_Log(const wchar_t* logfile, std::ostream* log) : _init(-1),_curtimez(0), _wname(*new cStrW()), _name(*new cStr()), _streams(*(new TSTREAMVAL())), _filestreams(*(new TFILESTREAMVAL())), _buf(*(new TBUFVAL(32)))
{
	_cutofflevel=0;
	IDVAL hold=!log?(IDVAL)-1:AddSource(*log);
	_winit=AddSource(logfile);
	_winit=(_winit==(IDVAL)-1)?hold:_winit;
}
BSS_Log::~BSS_Log()
{
  for(size_t i = 0; i < _filestreams.size(); ++i)
    _filestreams[i].close();

	delete &_streams;
	delete &_filestreams;
	//delete &_wstreams;
	//delete &_wfilestreams;
	delete &_buf;
	delete &_name;
	delete &_wname;
}

char BSS_FASTCALL BSS_Log::WriteLog(const char* format, unsigned char level, const char* file, int line, ...)
{
	if(!format || level>=NUMERRLEVELS) return -1;
	if(level<_cutofflevel) return 1;
 
  va_list args;
  va_start(args, line);

#if __STDC_WANT_SECURE_LIB__
  size_t length = _vscprintf(format, args); //This gets a little complicated because the secure version of this function does not behave the same way as the unsecure function. To use the secure function we make use of a different function to find the length.
#else
  size_t length = vsnprintf(0,0,format,args);
#endif

  //char* buf = new char[++length]; //+1 for null terminator
  if(_buf.size()<++length) _buf.reserve(length); //this prevents lots of memory from being allocated, drastically speeding up logging
  if(VSNPRINTF(_buf.UnsafeString(), length, format, args) < 0) //if this is negative something blew up
    return -2;

  va_end(args);

  std::ostream* ref;
  const char* addtmp=_name.empty()?"":"|";
  
  IDVAL i = _streams.Start();
  while(i!=(IDVAL)-1)
  {
    ref =_streams[i];
		(*ref) << '[';
		_writedatetime(_curtimez, ref,true);
    (*ref) << "] (" << _trimpath(file) << ':' << line << ") " << (const char*)_name.c_str() << addtmp << _errlevels[level] << ((const char*)_buf) << std::endl;
    _streams.Next(i);
  }

  return 0;
}

char BSS_FASTCALL BSS_Log::_writelog(const wchar_t* format, unsigned char level, const wchar_t* file, int line, va_list args)
{
#if __STDC_WANT_SECURE_LIB__
  size_t length = _vscwprintf(format, args); //This gets a little complicated because the secure version of this function does not behave the same way as the unsecure function. To use the secure function we make use of a different function to find the length.
#else
  size_t length = _vsnwprintf(0,0,format,args);
#endif

  const char* addtmp=_wname.empty()?"":"|";
  file=_trimpath(file);
#if __STDC_WANT_SECURE_LIB__
  length += _scwprintf(L"%s:%i) %s%s", file,line,_wname.c_str(),addtmp); //This gets a little complicated because the secure version of this function does not behave the same way as the unsecure function. To use the secure function we make use of a different function to find the length.
#else
  length += _snwprintf(0,0,L"%s:%i) %s%s",file,line,_wname.c_str(),addtmp);
#endif

  if(_buf.size()<((length+=2)<<1)) _buf.reserve(length<<1); //account for both null terminators
  size_t length2;
  if((length2=VSWPRINTF((wchar_t*)(char*)_buf.UnsafeString(), length, format, args)) < 0) //if this is negative something blew up
    return -2;

  size_t length3=++length2; //increment length2 to account for null terminator
#if __STDC_WANT_SECURE_LIB__
  if((length3+=swprintf_s(((wchar_t*)(char*)_buf.UnsafeString())+length2, length-length2, L"%s:%i) %s%s", file,line,_wname.c_str(),addtmp)) < 0) //if this is negative something blew up
    return -3;
#else
  if((length3+=swprintf(((wchar_t*)(char*)_buf.UnsafeString())+length2,L"%s:%i) %s%s", file,line,_wname.c_str(),addtmp)) < 0)
    return -3;
#endif
  _buf.SetSize((++length3)<<1);

  size_t utflength=UTF8Encode2BytesUnicode((const wchar_t*)(const char*)_buf,0); //Now we convert the first half into UTF-8...
  if(_buf.size()<(length3<<1)+(++utflength)) _buf.reserve(utflength+(length3<<1)); //increment for null terminator
  utflength=UTF8Encode2BytesUnicode((const wchar_t*)(const char*)_buf,(unsigned char*)(_buf.UnsafeString()+(length3<<1)));
  _buf.UnsafeString()[(length3<<1)+utflength]=0;
  _buf.SetSize((length3<<1)+(++utflength));
  
  const char* view=((const char*)_buf)+(length3<<1);
  const wchar_t* view3=((const wchar_t*)(const char*)_buf)+length2;

  size_t utflength2 =utflength+UTF8Encode2BytesUnicode(((const wchar_t*)(const char*)_buf)+length2,0); 
  if(_buf.size()<(length3<<1)+(++utflength2)) _buf.reserve(utflength2+(length3<<1)); //And then the second half.
  UTF8Encode2BytesUnicode(((const wchar_t*)(const char*)_buf)+length2,((unsigned char*)_buf.UnsafeString())+(length3<<1)+utflength);
  _buf.UnsafeString()[(length3<<1)+utflength2-1]=0;
  _buf.SetSize((length3<<1)+utflength2);

  view=((const char*)_buf)+(length3<<1);
  const char* view2=((const char*)_buf)+(length3<<1)+utflength;

  std::ostream* ref;
  unsigned int svar=_streams.Size();
  for(unsigned int i = 0; i < svar; ++i)
  {
    ref =_streams[i];
		(*ref) << '[';
		_writedatetime(_curtimez, ref,true);
    (*ref) << "] (" << (((const char*)_buf)+(length3<<1)+utflength) << _errlevels[level] << (((const char*)_buf)+(length3<<1)) << std::endl;
  }

  return 0;
}
char BSS_FASTCALL BSS_Log::WriteLog(const wchar_t* format, unsigned char level, const char* file, int line, ...)
{
	if(!format || level>=NUMERRLEVELS) return -1;
	if(level<_cutofflevel) return 1;

  va_list args;
  va_start(args, line);
  cStrW wfile(file);
  char ret=_writelog(format,level,wfile,line,args);
  va_end(args);
  return ret;
}

char BSS_FASTCALL BSS_Log::WriteLog(const wchar_t* format, unsigned char level, const wchar_t* file, int line, ...)
{
	if(!format || level>=NUMERRLEVELS) return -1;
	if(level<_cutofflevel) return 1;

  va_list args;
  va_start(args, line);
  char ret=_writelog(format,level,file,line,args);
  va_end(args);
  return ret;
}


BSS_Log::IDVAL BSS_FASTCALL BSS_Log::AddSource(const char* file)
{
  if(!file) return -1;
  std::ostream* stream = GetFileStream(file);
	return !stream?-1:AddSource(*stream);
}
BSS_Log::IDVAL BSS_FASTCALL BSS_Log::AddSource(const wchar_t* file)
{
  if(!file) return -1;
  std::ostream* stream = GetFileStream(file);
	return !stream?0:AddSource(*stream);
}
BSS_Log::IDVAL BSS_FASTCALL BSS_Log::AddSource(std::ostream& stream)
{
	//if(!stream) return 0;
	//BSS_LOG_STREAMDATA* add = new BSS_LOG_STREAMDATA(stream);
	return _streams.Add(&stream);
}
//BSS_Log::IDVAL BSS_FASTCALL BSS_Log::AddSource(std::wostream& stream)
//{
//  return _wstreams.Add(&stream);
//}
bool BSS_FASTCALL BSS_Log::RemoveSource(BSS_Log::IDVAL ID)
{
  if(ID>=_streams.Size()) return false;
	std::ostream* hold=_streams.Remove(ID);

  for(unsigned int i = 0; i < _filestreams.size(); ++i)
  {
    if(((std::ostream*)&_filestreams[i])==hold)
    {
      _filestreams.erase(_filestreams.begin()+i);
      break;
    }
  }

  return true;
}
std::ostream* BSS_Log::GetStream(BSS_Log::IDVAL ID) const
{
  return (ID >= _streams.Size())?0:_streams[ID];
}
//std::wostream* BSS_Log::GetStreamW(BSS_Log::IDVAL ID) const
//{
//  return (ID >= _wstreams.Size())?0:_wstreams[ID];
//}

//Format: HH:MM:SS
bool BSS_FASTCALL BSS_Log::WriteTime(char ptimez, std::ostream* log) //this cannot be a parameter called timezone, becaue timezone is already defined
{
  return _writedatetime(ptimez, log, true);
}

//Format: YYYY-MM-DD HH:MM:SS
bool BSS_FASTCALL BSS_Log::WriteDateTime(char ptimez, std::ostream* log)
{
  return _writedatetime(ptimez, log, false);
}

bool BSS_FASTCALL BSS_Log::_writedatetime(char ptimez, std::ostream* log, bool timeonly)
{
  tm* ptm=0;
  TIMEVALUSED rawtime;
  FTIME(&rawtime);
#if __STDC_WANT_SECURE_LIB__
  ptm = new tm();
#endif
  GMTIMEFUNC(&rawtime, ptm);

  char logchar = log->fill();
  std::streamsize logwidth = log->width();
  log->fill('0');
  //Write date if we need to
  if(!timeonly) (*log) << std::setw(4) << ptm->tm_year+1900 << std::setw(1) << '-' << std::setw(2) << ptm->tm_mon+1 << std::setw(1) << '-' << std::setw(2) << ptm->tm_mday << std::setw(1) << ' ';
  //Write time and flush stream
  (*log) << std::setw(1) << (ptm->tm_hour+ptimez)%24 << std::setw(0) << ':' << std::setw(2) << ptm->tm_min << std::setw(0) << ':' << std::setw(2) << ptm->tm_sec;
  
  //Reset values of stream
  log->width(logwidth);
  log->fill(logchar);

#if __STDC_WANT_SECURE_LIB__
  delete ptm;
#endif
  return true;
}
//char BSS_Log::GetTimeZone()
//{
//	TIME_ZONE_INFORMATION tzi;
//	GetTimeZoneInformation(&tzi);
//	return (char)(tzi.Bias/60); //This is given in minutes, and we want hours, so we divide by 60
//}

BSS_Log& BSS_Log::operator=(const BSS_Log& right)
{
  _streams.Clear();
  _filestreams.clear();
  NameModule(right._wname);
  _cutofflevel=right._cutofflevel;
  _streams=right._streams;
  //_wstreams=right._wstreams;
  _init=right._init;
	return *this;
}

void BSS_FASTCALL BSS_Log::SetFilter(unsigned char level)
{
  _cutofflevel=level>=NUMERRLEVELS?NUMERRLEVELS-1:level;
}

unsigned char BSS_Log::GetFilter() const
{
	return _cutofflevel;
}

std::ostream* BSS_Log::GetInitStream() const
{
	return (_init==(IDVAL)-1)?0:GetStream(_init);
}
//std::wostream* BSS_Log::GetInitStreamW() const
//{
//	return (_winit==(IDVAL)-1)?0:GetStreamW(_winit);
//}

const char* BSS_FASTCALL BSS_Log::_trimpath(const char* path)
{
	const char* retval=strrchr(path,'/');
	if(!retval)
	{
		retval=strrchr(path,'\\');
		if(!retval) retval=path;
		else ++retval;
	}
	else
	{
		path=strrchr(path,'\\');
		retval=path>retval?++path:++retval;
	}
	return retval;
}

const wchar_t* BSS_FASTCALL BSS_Log::_trimpath(const wchar_t* path)
{
	const wchar_t* retval=wcsrchr(path,'/');
	if(!retval)
	{
		retval=wcsrchr(path,'\\');
		if(!retval) retval=path;
		else ++retval;
	}
	else
	{
		path=wcsrchr(path,'\\');
		retval=path>retval?++path:++retval;
	}
	return retval;
}

std::ostream* BSS_FASTCALL BSS_Log::GetFileStream(const char* logfile)
{
  unsigned int ref=_filestreams.size();
  _filestreams.push_back(std::fstream());
  _filestreams[ref].open(logfile,std::ios_base::trunc+std::ios_base::out);
  return _filestreams[ref].fail()?0:&_filestreams[ref];
}

std::ostream* BSS_FASTCALL BSS_Log::GetFileStream(const wchar_t* logfile)
{
  unsigned int ref=_filestreams.size();
  _filestreams.push_back(std::fstream());
  _filestreams[ref].open(logfile,std::ios_base::trunc+std::ios_base::out);
  return _filestreams[ref].fail()?0:&_filestreams[ref];
}

void BSS_FASTCALL BSS_Log::NameModule(const char* name)
{
  _wname=name;
  _name=name;
}
void BSS_FASTCALL BSS_Log::NameModule(const wchar_t* wname)
{
  _wname=wname;
  _name=wname;
}
*/