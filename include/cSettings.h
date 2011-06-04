// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_SETTINGS_H__ 
#define __C_SETTINGS_H__

#include "cINIstorage.h"
#include <sstream>
#include <functional>
#include "cCmdLineArgs.h"

namespace bss_util {
  /* Function to convert any standard value type to a string */
  template<typename C, typename T>
  static std::basic_string<C> tostring(const T& t)
  {
    std::basic_stringstream<C> s;
    s << t;
    return s.str();
  }
  
  /* Special constructor cKhash implementation, which cKhash is not really meant for so do not use this ever */
  template<typename K=char, typename T=void*, bool ismap=true>
  class __declspec(dllexport) cKhash_StringTInsConstruct : public cKhash_StringTIns<K,T,ismap>
  {
  public:
    cKhash_StringTInsConstruct() : cKhash_StringTIns<K,T,ismap>() {}
    ~cKhash_StringTInsConstruct()
    {
      ResetWalk();
      khiter_t iter;
      while((iter=GetNext())!=kh_end(_h))
        kh_val(_h,iter).~T();
    }
    bool Insert(KHKEY key, const_ref value)
		{
      if(!KH_STR_VALIDATEPTR<const K*>(key)) return false;
			if(kh_size(_h) >= _h->n_buckets) _resize();
			int r;
			khiter_t retval = kh_put_template(_h,key,&r);
			if(r>0) //Only insert the value if the key didn't exist
			{
        new((_h)->vals+retval) T();
        kh_val(_h,retval)=value;
        return true;
			}
			return false;
		}
  };

  /* Base template cSetting class containing the master cmdhash for processing command lines. */
  template<int I, int N>
  class cSetting
  { 
  public: 
    static cKhash_StringTInsConstruct<char,std::function<void (cCmdLineArgs<char>&,unsigned int&)>> cmdhash;
  }; 
  cKhash_StringTInsConstruct<char,std::function<void (cCmdLineArgs<char>&,unsigned int&)>> cSetting<-1,-1>::cmdhash;

  /* Class triggered when a setting is declared, adding it to the master command hash if necessary. */
  template<typename C>
  struct AddToSettingHash { 
    AddToSettingHash(const C* cmdname, const std::function<void (cCmdLineArgs<C>&,unsigned int&)>& func)
    { if(cmdname!=0) cSetting<-1,-1>::cmdhash.Insert(cmdname,func); }
  };

  /* Static template function shortcut for getting a specific setting */
  template<int I, int N>
  static typename cSetting<I,N>::TYPE& Setting() { return cSetting<I,N>::v; }

  /* Main #define for declaring a setting. NAME and CMD can both be set to 0 if INIs and command line parsing, respectively, are not needed. */
#define DECL_SETTING(I,N,T,INIT,NAME,CMD) template<> class cSetting<I,N> { public: typedef T TYPE; static T v; \
  inline static const char* name() { return NAME; } inline static const char* cmd() { return CMD; } \
  static AddToSettingHash<char> _shashinit; }; T cSetting<I,N>::v=INIT; \
  AddToSettingHash<char> cSetting<I,N>::_shashinit(CMD,[](cCmdLineArgs<char>& rcmd, unsigned int& ind) -> void { cSetting_CMDLOAD<T,char>::CmdLoad(rcmd,cSetting<I,N>::v,ind); })

  /* Main #define for declaring a group of settings. Note that the optional NAME parameter is for INI loading and determines the section
     name of the settings. MAX is the maximum number of settings in this group, but unless you are using LoadAllFromINI, it is optional. */
#define DECL_SETGROUP(I,NAME,MAX) template<int N> class cSetting<I,N> { public: inline static const char* secname() { return NAME; } static const unsigned int COUNT=(MAX-1); };
#define GtS(GROUP,NAME) 

  /* Struct class for defining the INILoad function. Can be overriden for custom types */
  template<typename T, typename C>
  struct cSetting_INILOAD {
    inline static void INILoad(cINIstorage<C>& ini, T& v, const C* name, const C* section)
    { 
      cINIentry<C>* p;
      if(name!=0 && (p=ini.GetEntryPtr(section,name))!=0) { v=*p; }
    }
  };

  /* Struct class for defining the INISave function. Can be overriden for custom types */
  template<typename T, typename C>
  struct cSetting_INISAVE {
    inline static void INISave(cINIstorage<C>& ini, T& v, const C* name, const C* section)
    { if(name!=0) { ini.EditAddEntry(section,name,tostring<C,T>(v).c_str()); } }
  };
  
  /* Override for string types on INI loading */
  template<typename C>
  struct cSetting_INISAVE<const C*,C> {
    inline static void INISave(cINIstorage<C>& ini, const C* v, const C* name, const C* section)
    { if(name!=0) { ini.EditAddEntry(section,name,v); } }
  };

  /* Struct class for defining the CmdLoad function. Can be overriden for custom types */
  template<typename T, typename C>
  struct cSetting_CMDLOAD {
    inline static void CmdLoad(cCmdLineArgs<C>& ini, T& v, unsigned int& index)
    { std::basic_stringstream<C>(std::basic_string<C>(ini[index++]), std::stringstream::in) >> v; }
  };

  /* Main class for managing settings. Here you can load and save settings from INIs */
  template<int I,int N>
  class cSettingManage
  {
  public:
    template<typename C>
    inline static void LoadFromINI(cINIstorage<C>& ini)
    {
      cSetting_INILOAD<typename cSetting<I,N>::TYPE,C>::INILoad(ini, cSetting<I,N>::v, cSetting<I,N>::name(), cSetting<I,-1>::secname());
      cSettingManage<I,N-1>::LoadFromINI<C>(ini);
    };
    template<typename C>
    inline static void LoadAllFromINI(cINIstorage<C>& ini)
    {
      cSettingManage<I,cSetting<I,-1>::COUNT>::LoadFromINI<C>(ini);
      cSettingManage<I-1,N>::LoadAllFromINI<C>(ini);
    };
    template<typename C>
    inline static void SaveToINI(cINIstorage<C>& ini)
    {
      cSetting_INISAVE<typename cSetting<I,N>::TYPE,C>::INISave(ini, cSetting<I,N>::v, cSetting<I,N>::name(), cSetting<I,-1>::secname());
      cSettingManage<I,N-1>::SaveToINI<C>(ini);
    };
    template<typename C>
    inline static void SaveAllToINI(cINIstorage<C>& ini)
    {
      cSettingManage<I,cSetting<I,-1>::COUNT>::SaveToINI<C>(ini);
      cSettingManage<I-1,N>::SaveAllToINI<C>(ini);
    };
  };
  
  template<int I>
  class cSettingManage<I,-1>
  {
  public:
    template<typename C>
    inline static void LoadFromINI(cINIstorage<C>& ini) {} // recursive template evaluation terminator
    template<typename C>
    inline static void SaveToINI(cINIstorage<C>& ini) { ini.EndINIEdit(); } // recursive template evaluation terminator
  };

  template<int N>
  class cSettingManage<-1,N>
  {
  public:
    template<typename C>
    inline static void LoadAllFromINI(cINIstorage<C>& ini) {} // recursive template evaluation terminator
    template<typename C>
    inline static void SaveAllToINI(cINIstorage<C>& ini) {} // recursive template evaluation terminator
  };

  /* Static template function for loading settings from a command line. */
  template<typename C>
  inline static void LoadFromCmd(cCmdLineArgs<C>& cmd) 
  {
    unsigned int i=0;
    while(i<cmd.Size())
    {
      std::function<void (cCmdLineArgs<char>&,unsigned int&)>* pfunc = cSetting<-1,-1>::cmdhash.GetKey(cmd[i]);
      ++i;
      if(pfunc!=0)
        (*pfunc)(cmd,i); //function must increment i
    }
  }
}

#endif