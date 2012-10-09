// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_SETTINGS_H__BSS__ 
#define __C_SETTINGS_H__BSS__

#include "cINIstorage.h"
#include <sstream>
#include <functional>
#include <vector>
#include "cCmdLineArgs.h"

namespace bss_util {
  // Function to convert any standard value type to a string
  template<typename C, typename T>
  static std::basic_string<C> tostring(const T& t)
  {
    std::basic_stringstream<C> s;
    s << t;
    return s.str();
  }
  
  // Special constructor cKhash implementation, which cKhash is not really meant for so do not use this ever
  template<typename K=char, typename T=void*, bool ismap=true>
  class BSS_COMPILER_DLLEXPORT cKhash_StringTInsConstruct : public cKhash_StringTIns<K,T,ismap>
  {
  public:
    cKhash_StringTInsConstruct() : cKhash_StringTIns<K,T,ismap>() {}
    ~cKhash_StringTInsConstruct()
    {
      for(khiter_t iter=kh_begin(_h); iter<kh_end(_h); ++iter)
        if(kh_exist(_h,iter))
          kh_val(_h,iter).~T();
    }
    bool Insert(KHKEY key, const KHVAL& value)
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

  // Base template cSetting class containing the master cmdhash for processing command lines.
  template<int I, int N>
  class cSetting
  { 
  public: 
    static cKhash_StringTInsConstruct<char,std::function<void (cCmdLineArgs&,unsigned int&)>> cmdhash;
  }; 
  cKhash_StringTInsConstruct<char,std::function<void (cCmdLineArgs&,unsigned int&)>> cSetting<-1,-1>::cmdhash;

  // Class triggered when a setting is declared, adding it to the master command hash if necessary.
  struct AddToSettingHash { 
    AddToSettingHash(const char* cmdname, const std::function<void (cCmdLineArgs&,unsigned int&)>& func)
    { if(cmdname!=0) cSetting<-1,-1>::cmdhash.Insert(cmdname,func); }
  };

  // Static template function shortcut for getting a specific setting
  template<int I, int N>
  static typename cSetting<I,N>::TYPE& Setting() { return cSetting<I,N>::v; }

#ifdef INSTANTIATE_SETTINGS //Declare this in a CPP file that includes all DECL_SETTINGs used in your project to instantiate them
#define i_INST_SET_(I,N,T,INIT,NAME,CMD) T bss_util::cSetting<I,N>::v=INIT; \
  bss_util::AddToSettingHash bss_util::cSetting<I,N>::_shashinit(CMD,[](cCmdLineArgs& rcmd, unsigned int& ind) -> void { bss_util::cSetting_CMDLOAD<T,char>::CmdLoad(rcmd,bss_util::cSetting<I,N>::v,ind); })
#else
#define i_INST_SET_(I,N,T,INIT,NAME,CMD) 
#endif

  // Main #define for declaring a setting. NAME and CMD can both be set to 0 if INIs and command line parsing, respectively, are not needed.
#define DECL_SETTING(I,N,T,INIT,NAME,CMD) template<> class bss_util::cSetting<I,N> { public: typedef T TYPE; static T v; \
  inline static const char* name() { return NAME; } inline static const char* cmd() { return CMD; } \
  static bss_util::AddToSettingHash _shashinit; }; i_INST_SET_(I,N,T,INIT,NAME,CMD)

  /* Main #define for declaring a group of settings. Note that the optional NAME parameter is for INI loading and determines the section
     name of the settings. MAX is the maximum number of settings in this group, but unless you are using LoadAllFromINI, it is optional. */
#define DECL_SETGROUP(I,NAME,MAX) template<int N> class bss_util::cSetting<I,N> { public: inline static const char* secname() { return NAME; } static const unsigned int COUNT=(MAX-1); };

  // Struct class for defining the INILoad function. Can be overriden for custom types
  template<typename T, typename C>
  struct cSetting_INILOAD {
    inline static void INILoad(cINIstorage& ini, T& v, const C* name, const C* section)
    { 
      cINIentry* p;
      if(name!=0 && (p=ini.GetEntryPtr(section,name))!=0) { v=*p; }
    }
  };
  
  template<typename C>
  struct cSetting_INILOAD<std::vector<cStrT<C>>,C> {
    inline static void INILoad(cINIstorage& ini, std::vector<cStrT<C>>& v, const C* name, const C* section) { 
      cINIentry* p;
      v.clear();
      if(name!=0) {
        for(int i = 0; (p=ini.GetEntryPtr(section,name,i))!=0; ++i) { v.push_back(p->GetString()); }
      }
    }
  };

  // Struct class for defining the INISave function. Can be overriden for custom types
  template<typename T, typename C>
  struct cSetting_INISAVE {
    inline static void INISave(cINIstorage& ini, T& v, const C* name, const C* section)
    { if(name!=0) { ini.EditAddEntry(section,name,tostring<C,T>(v).c_str()); } }
  };
  
  // Override for string types on INI loading
  template<typename C>
  struct cSetting_INISAVE<const C*,C> {
    inline static void INISave(cINIstorage& ini, const C* v, const C* name, const C* section)
    { if(name!=0) { ini.EditAddEntry(section,name,v); } }
  };
  // Force char types to write as numbers
  template<typename C>
  struct cSetting_INISAVE<unsigned char,C> {
    inline static void INISave(cINIstorage& ini, unsigned char v, const C* name, const C* section)
    { char s[4]; ITOAx0((int)v,s,10); if(name!=0) { ini.EditAddEntry(section,name,s); } }
  };
  template<typename C>
  struct cSetting_INISAVE<char,C> {
    inline static void INISave(cINIstorage& ini, char v, const C* name, const C* section)
    { char s[4]; ITOAx0((int)v,s,10); if(name!=0) { ini.EditAddEntry(section,name,s); } }
  };
  template<typename C>
  struct cSetting_INISAVE<std::vector<cStrT<C>>,C> {
    inline static void INISave(cINIstorage& ini, const std::vector<cStrT<C>>& v, const C* name, const C* section)
    { if(name!=0) { for(unsigned int i = 0; i < v.size(); ++i) ini.EditAddEntry(section,name,v[i],i,0); } }
  };

  // Struct class for defining the CmdLoad function. Can be overriden for custom types
  template<typename T, typename C>
  struct cSetting_CMDLOAD {
    inline static void CmdLoad(cCmdLineArgs& ini, T& v, unsigned int& index)
    { std::basic_stringstream<C>(std::basic_string<C>(ini[index++]), std::stringstream::in) >> v; }
  };

  template<typename C>
  struct cSetting_CMDLOAD<const C*,C> {
    inline static void CmdLoad(cCmdLineArgs& ini, const C*& s, unsigned int& index)
    { s=ini[index++]; }
  };

  template<typename C>
  struct cSetting_CMDLOAD<bool,C> {
    inline static void CmdLoad(cCmdLineArgs& ini, bool& v, unsigned int& index)
    { v=true; }
  };

  template<typename C>
  struct cSetting_CMDLOAD<std::vector<cStrT<C>>,C> {
    inline static void CmdLoad(cCmdLineArgs& ini, std::vector<cStrT<C>>& v, unsigned int& index) {} //You cannot load this from the command line
  };

  // Main class for managing settings. Here you can load and save settings from INIs
  template<int I,int N>
  class cSettingManage
  {
  public:
    inline static void LoadFromINI(cINIstorage& ini)
    {
      cSetting_INILOAD<typename cSetting<I,N>::TYPE,char>::INILoad(ini, cSetting<I,N>::v, cSetting<I,N>::name(), cSetting<I,-1>::secname());
      cSettingManage<I,N-1>::LoadFromINI(ini);
    };
    inline static void LoadAllFromINI(cINIstorage& ini)
    {
      cSettingManage<I,cSetting<I,-1>::COUNT>::LoadFromINI(ini);
      cSettingManage<I-1,N>::LoadAllFromINI(ini);
    };
    inline static void SaveToINI(cINIstorage& ini)
    {
      cSetting_INISAVE<typename cSetting<I,N>::TYPE,char>::INISave(ini, cSetting<I,N>::v, cSetting<I,N>::name(), cSetting<I,-1>::secname());
      cSettingManage<I,N-1>::SaveToINI(ini);
    };
    inline static void SaveAllToINI(cINIstorage& ini)
    {
      cSettingManage<I,cSetting<I,-1>::COUNT>::SaveToINI(ini);
      cSettingManage<I-1,N>::SaveAllToINI(ini);
    };
  };
  
  template<int I>
  class cSettingManage<I,-1>
  {
  public:
    inline static void LoadFromINI(cINIstorage& ini) {} // recursive template evaluation terminator
    inline static void SaveToINI(cINIstorage& ini) { ini.EndINIEdit(); } // recursive template evaluation terminator
  };

  template<int N>
  class cSettingManage<-1,N>
  {
  public:
    inline static void LoadAllFromINI(cINIstorage& ini) {} // recursive template evaluation terminator
    inline static void SaveAllToINI(cINIstorage& ini) {} // recursive template evaluation terminator
  };

  // Static template function for loading settings from a command line.
  inline static void LoadFromCmd(cCmdLineArgs& cmd) 
  {
    unsigned int i=0; // Must be unsigned because of what pfunc accepts
    while(i<cmd.Size())
    {
      std::function<void (cCmdLineArgs&,unsigned int&)>* pfunc = cSetting<-1,-1>::cmdhash.GetKey(cmd[i]);
      ++i;
      if(pfunc!=0)
        (*pfunc)(cmd,i); //function must increment i
    }
  }
}

#endif