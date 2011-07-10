// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

char BSS_FASTCALL bss_initINI(INIParser* init, FILE* stream)
{
  if(!init) return 0;
  init->file=stream;
  init->curvalue=(CHAR*)malloc(sizeof(CHAR)); //If this isn't sizeof(CHAR), when the next line gets executed, the system thinks its 2 bytes long for wchar_t and overwrites memory
  *init->curvalue=0;
  init->curkey=(CHAR*)malloc(sizeof(CHAR));
  *init->curkey=0;
  init->cursection=(CHAR*)malloc(sizeof(CHAR));
  *init->cursection=0;
  return 1;
}
char BSS_FASTCALL bss_destroyINI(INIParser* destroy)
{
  if(!destroy) return 0;
  destroy->file=0;
  free(destroy->curvalue);
  free(destroy->curkey);
  free(destroy->cursection);
  destroy->curvalue=0;
  destroy->curkey=0;
  destroy->cursection=0;
  return 1;
}

const CHAR* BSS_FASTCALL _trimlstr(const CHAR* str)
{
  for(;*str>0 && *str<33;++str);
  return str;
}
CHAR* BSS_FASTCALL _trimrstr(CHAR* str)
{
  CHAR* inter=str+strlen(str);

  for(;inter>str && *inter<33;--inter);
  *(++inter)='\0';
  return str;
}


char BSS_FASTCALL _nextkeychar(const CHAR** output, const CHAR* line)
{
  for(;*line;++line)
  {
    switch(*line)
    {
    case ';':
      return -1;
    case '=':
      *output=line;
      return 0;
    case '[':
      *output=line;
      return 1;
    }
  }
  return -1;
}

CHAR* BSS_FASTCALL _validatesection(CHAR* start)
{
  char valid=0; //checks to make sure there's at least one valid character in the string

  for(;*start;++start)
  {
    if(*start==';') return 0; //a comment started before we found the end
    if(*start==']') break; //found it, break to end of function
    if(valid==0 && *start>=VALIDASCII) valid=1;
  }
  return valid==0?0:start;
}


char BSS_FASTCALL _validatekey(const CHAR* start, const CHAR* end)
{
  for(;start<end;++start)
    if(*start>=VALIDASCII) return 1; //32 is space and everything below is just crap.
  return 0;
}

CHAR* BSS_FASTCALL _validatevalue(CHAR* str)
{
  char valid=0; //checks to make sure there's at least one valid character in the string

  for(;*str;++str)
  {
    if(*str==';') break; //a comment started
    if(valid==0 && *str>=VALIDASCII) valid=1;
  }
  return valid==0?0:str;
}

CHAR* BSS_FASTCALL _savestr(CHAR* orig, const CHAR* str)
{
  unsigned int olength=strlen(orig);
  unsigned int nlength;
  str=_trimlstr(str);
  nlength=strlen(str);
  if(olength<nlength)
  {
    free(orig);
    orig=(CHAR*)malloc((nlength+1)*sizeof(CHAR));
  }
  memcpy(orig,str,nlength*sizeof(CHAR));
  orig[nlength]='\0';
  _trimrstr(orig);
  return orig;
}

void BSS_FASTCALL _newsection(INIParser* parse, const CHAR* str)
{
  parse->cursection=_savestr(parse->cursection,str);
  parse->newsection=1;
}
void _newkeyvaluepair(INIParser* parse, const CHAR* start, CHAR* mid)
{
  *mid='\0';
  parse->curkey=_savestr(parse->curkey,start);
  parse->curvalue=_savestr(parse->curvalue,mid+1);
}

char BSS_FASTCALL bss_parseLine(INIParser* parse)
{
  //const CHAR* line;
  CHAR* tc;
  CHAR* sec;
  parse->newsection=0;

  while(!feof(parse->file))
  {
    fgets(parse->buf,MAXLINELENGTH,parse->file);
    switch(_nextkeychar(&tc,parse->buf))
    {
    //case -1: //Garbage line
    //  break;
    case 0: //key value pair
      if(_validatekey(parse->buf,tc)!=0 && (sec=_validatevalue(tc+1))!=0)
      {
        *sec='\0';
        _newkeyvaluepair(parse,parse->buf,tc);
        return 1;
      }
      break;
    case 1: //section identifier
      sec=_validatesection(tc); //if the validation function returns NULL its garbage
      if(sec!=0 && sec!=(++tc)) { sec[0]='\0'; _newsection(parse,tc); return -1; }
      break;
    }
  }
  return 0;
}

char BSS_FASTCALL _snextkeychar(const CHAR** output, const CHAR* start, const CHAR* end)
{
  for(;start<end;++start)
  {
    switch(*start)
    {
    case ';':
      return -1;
    case '=':
      *output=start;
      return 0;
    case '[':
      *output=start;
      return 1;
    }
  }
  return -1;
}

const CHAR* BSS_FASTCALL _svalidatesection(const CHAR* start, const CHAR* end)
{
  char valid=0; //checks to make sure there's at least one valid character in the string

  for(;start<end;++start)
  {
    if(*start==';') return 0; //a comment started before we found the end
    if(*start==']') break; //found it, break to end of function
    if(valid==0 && *start>=VALIDASCII) valid=1;
  }
  return valid==0?0:start;
}
const CHAR* BSS_FASTCALL _svalidatevalue(const CHAR* start, const CHAR* end)
{
  char valid=0; //checks to make sure there's at least one valid character in the string

  for(;start<end;++start)
  {
    if(*start==';') break; //a comment started
    if(valid==0 && *start>=VALIDASCII) valid=1;
  }
  return valid==0?0:start;
}

const CHAR* BSS_FASTCALL _trimrstralt(const CHAR* end,const CHAR* begin)
{
  for(;end>begin && *end<33;--end);
  return end;
}

char BSS_FASTCALL comparevalues(const CHAR* start, const CHAR* end, const CHAR* comp)
{
  start=_trimlstr(start);
  end=_trimrstralt(--end,start);
  return _strnicmp(start,comp,_minfunc((end-start)+1,strlen(comp)));
}
INICHUNK BSS_FASTCALL bss_findINIsection(const void* data, unsigned length, const CHAR* section, unsigned int instance)
{
  const CHAR* cur=(const CHAR*)data;
  const CHAR* end=cur+length;
  const CHAR* line;
  const CHAR* tc;
  const CHAR* sec;
  unsigned int curinstance=(unsigned int)-1;
  INICHUNK retval;
  memset(&retval,0,sizeof(INICHUNK));

  while(cur<end)
  {
    line = strchr(cur,'\n');
    if(!line) line=end; //if that returned nothing we're on the last line
    if(_snextkeychar(&tc,cur,line)==1)
    {
      if(retval.start!=0) //if this is true we already found the section and now we're just looking for the next valid one
      {
        retval.end=--cur;
        break;
      }
      sec=_svalidatesection(tc,line); //if the validation function returns NULL its garbage
      if(sec!=0 && sec!=(tc+1) && comparevalues(tc+1,sec,section)==0 && (++curinstance)==instance)
      {
        retval.start=tc; //this is safe because we already verified that there is at least one valid character in this string. Ok, it still isn't safe, but it won't explode.
        retval.end=end; //just in case this is the last section
      } //Now we search for the next section
    }
    cur=++line;
  }
  return retval;
}

INICHUNK BSS_FASTCALL bss_findINIentry(INICHUNK section, const CHAR* key, unsigned int instance)
{
  const CHAR* cur=(const CHAR*)section.start;
  const CHAR* end=(const CHAR*)section.end;
  const CHAR* line;
  const CHAR* tc;
  const CHAR* sec;
  unsigned int curinstance=(unsigned int)-1;
  INICHUNK retval;
  memset(&retval,0,sizeof(INICHUNK));

  while(cur<end)
  {
    line = strchr(cur,'\n');
    if(!line) line=end; //if that returned nothing we're on the last line
    if(_snextkeychar(&tc,cur,line)==0)
    {
      if(_validatekey(cur,tc)!=0 && (sec=_svalidatevalue(tc+1,line))!=0 && comparevalues(cur,tc,key)==0 && (++curinstance)==instance)
      {
        retval.start=_trimlstr(cur); //this is safe because we already verified that there is at least one valid character in this string. Ok, it still isn't safe, but it won't explode.
        retval.end=sec;
        break;
      }
    }
    cur=++line;
  }
  return retval;
}