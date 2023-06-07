// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#include "buntils/INIparse.h"
#include "buntils/buntils_c.h"
#include <malloc.h>
#include <string.h>

const char VALIDASCII = 33; //technically this works for unicode too

//int _minfunc(int a, int b)
//{
//  return a<b?a:b;
//}

// this is a wrapper function that decodes a utf-8 file but still behaves like fgets
//void _fgetutf8(wchar_t* buf, int max, FILE* file)
//{
//  char ibuf[MAXLINELENGTH]; //we actually ignore max here because we know it will be this - we accept it so this is compatible with the other call
//  fgets(ibuf,--max,file); //we subtract one from max so we have room for the null terminator in the unicode string
//  max=UTF8Decode2BytesUnicode(ibuf,buf);
//  buf[max]=0;
//}


char bun_InitINI(INIParser* init, FILE* stream)
{
  if(!init) return 0;
  init->file = stream;
  init->curvalue = (char*)malloc(sizeof(char)); //If this isn't sizeof(char), when the next line gets executed, the system thinks its 2 bytes long for wchar_t and overwrites memory
  if(!init->curvalue) return 0;
  *init->curvalue = 0;
  init->curkey = (char*)malloc(sizeof(char));
  if(!init->curkey) return 0;
  *init->curkey = 0;
  init->cursection = (char*)malloc(sizeof(char));
  if(!init->cursection) return 0;
  *init->cursection = 0;
  return 1;
}
char bun_DestroyINI(INIParser* destroy)
{
  if(!destroy) return 0;
  destroy->file = 0;
  free(destroy->curvalue);
  free(destroy->curkey);
  free(destroy->cursection);
  destroy->curvalue = 0;
  destroy->curkey = 0;
  destroy->cursection = 0;
  return 1;
}

const char* _trimlstr(const char* str)
{
  for(; *str > 0 && *str < 33; ++str);
  return str;
}
char* _trimrstr(char* str)
{
  char* inter = str + strlen(str);

  for(; inter > str && *inter < 33; --inter);
  *(++inter) = '\0';
  return str;
}


char _nextkeychar(const char** output, const char* line)
{
  for(; *line; ++line)
  {
    switch(*line)
    {
    case ';':
      return -1;
    case '=':
      *output = line;
      return 0;
    case '[':
      *output = line;
      return 1;
    }
  }
  return -1;
}

char* _validatesection(char* start)
{
  char valid = 0; //checks to make sure there's at least one valid character in the string

  for(; *start; ++start)
  {
    if(*start == ';') return 0; //a comment started before we found the end
    if(*start == ']') break; //found it, break to end of function
    if(valid == 0 && *start >= VALIDASCII) valid = 1;
  }
  return valid == 0 ? 0 : start;
}


char _validatekey(const char* start, const char* end)
{
  for(; start < end; ++start)
    if(*start >= VALIDASCII) return 1; //32 is space and everything below is just crap.
  return 0;
}

// Finds the end of a value line (before a comment starts) but doesn't care about whether the value has valid characters or not.
char* _validatevalue(char* str)
{
  for(; *str; ++str)
    if(*str == ';') break;
  return str;
}

char* _savestr(char* orig, const char* str)
{
  size_t olength = strlen(orig);
  size_t nlength;
  str = _trimlstr(str);
  nlength = strlen(str);
  if(olength < nlength)
  {
    char* norig = (char*)realloc(orig, (nlength + 1) * sizeof(char));
    if(!norig)
      free(orig);
    orig = norig;
  }
  if(!orig) return 0;
  memcpy(orig, str, nlength * sizeof(char));
  orig[nlength] = '\0';
  _trimrstr(orig);
  return orig;
}

void _newsection(INIParser* parse, const char* str)
{
  parse->cursection = _savestr(parse->cursection, str);
  parse->newsection = 1;
}
void _newkeyvaluepair(INIParser* parse, const char* start, char* mid)
{
  *mid = '\0';
  parse->curkey = _savestr(parse->curkey, start);
  parse->curvalue = _savestr(parse->curvalue, mid + 1);
}

char bun_ParseLine(INIParser* parse)
{
  //const char* line;
  char* tc;
  char* sec;
  parse->newsection = 0;

  while(!feof(parse->file))
  {
    fgets(parse->buf, MAXLINELENGTH, parse->file);
    switch(_nextkeychar((const char**)&tc, parse->buf))
    {
    //case -1: //Garbage line
    //  break;
    case 0: //key value pair
      if(_validatekey(parse->buf, tc) != 0)
      {
        sec = _validatevalue(tc + 1); // We used to check if this returned nonzero, but now it ALWAYS returns nonzero.
        *sec = '\0';
        _newkeyvaluepair(parse, parse->buf, tc);
        return 1;
      }
      break;
    case 1: //section identifier
      sec = _validatesection(tc); //if the validation function returns nullptr its garbage
      if(sec != 0 && sec != (++tc)) { sec[0] = '\0'; _newsection(parse, tc); return -1; }
      break;
    }
  }
  return 0;
}

char _snextkeychar(const char** output, const char* start, const char* end)
{
  for(; start < end; ++start)
  {
    switch(*start)
    {
    case ';':
      return -1;
    case '=':
      *output = start;
      return 0;
    case '[':
      *output = start;
      return 1;
    }
  }
  return -1;
}

const char* _svalidatesection(const char* start, const char* end)
{
  char valid = 0; //checks to make sure there's at least one valid character in the string

  for(; start < end; ++start)
  {
    if(*start == ';') return 0; //a comment started before we found the end
    if(*start == ']') break; //found it, break to end of function
    if(valid == 0 && *start >= VALIDASCII) valid = 1;
  }
  return valid == 0 ? 0 : start;
}
const char* _svalidatevalue(const char* start, const char* end)
{
  for(; start < end; ++start)
    if(*start == ';') break; //a comment started
  return start;
}

const char* _trimrstralt(const char* end, const char* begin)
{
  for(; end > begin && *end < 33; --end);
  return end;
}

int comparevalues(const char* start, const char* end, const char* comp)
{
  ptrdiff_t a, b;
  start = _trimlstr(start);
  end = _trimrstralt(end - 1, start);
  a = (end - start) + 1;
  b = strlen(comp);
  return STRNICMP(start, comp, bun_min(a, b));
}
INICHUNK bun_FindINISection(const void* data, size_t length, const char* section, size_t instance)
{
  const char* cur = (const char*)data;
  const char* end = cur + length;
  const char* line;
  const char* tc;
  const char* sec;
  size_t curinstance = (size_t)~0;
  INICHUNK retval;
  memset(&retval, 0, sizeof(INICHUNK));

  while(cur < end)
  {
    line = strchr(cur, '\n');
    if(!line) line = end; //if that returned nothing we're on the last line
    if(_snextkeychar(&tc, cur, line) == 1)
    {
      if(retval.start != 0) //if this is true we already found the section and now we're just looking for the next valid one
      {
        retval.end = --cur;
        break;
      }
      sec = _svalidatesection(tc, line); //if the validation function returns nullptr its garbage
      if(sec != 0 && sec != (tc + 1) && comparevalues(tc + 1, sec, section) == 0 && (++curinstance) == instance)
      {
        retval.start = tc; //this is safe because we already verified that there is at least one valid character in this string. Ok, it still isn't safe, but it won't explode.
        retval.end = end; //just in case this is the last section
      } //Now we search for the next section
    }
    cur = line + 1;
  }
  return retval;
}

INICHUNK bun_FindINIEntry(INICHUNK section, const char* key, size_t instance)
{
  const char* cur = (const char*)section.start;
  const char* end = (const char*)section.end;
  const char* line;
  const char* tc;
  const char* sec;
  size_t curinstance = (size_t)~0;
  INICHUNK retval;
  memset(&retval, 0, sizeof(INICHUNK));

  while(cur < end)
  {
    line = strchr(cur, '\n');
    if(!line) line = end; //if that returned nothing we're on the last line
    if(_snextkeychar(&tc, cur, line) == 0)
    {
      if(_validatekey(cur, tc) != 0 && comparevalues(cur, tc, key) == 0 && (++curinstance) == instance)
      {
        sec = _svalidatevalue(tc + 1, line);
        retval.start = _trimlstr(cur); //this is safe because we already verified that there is at least one valid character in this string. Ok, it still isn't safe, but it won't explode.
        retval.end = sec;
        break;
      }
    }
    cur = line + 1;
  }
  return retval;
}