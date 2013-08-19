// Copyright ©2013 Black Sphere Studios
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
  template<typename T>
  inline static std::string tostring(const T& t)
  {
    std::stringstream s;
    s << t;
    return s.str();
  }
  
  // Special constructor cKhash implementation, which cKhash is not really meant for so do not use this ever
  template<typename T=void*, bool ismap=true>
  class BSS_COMPILER_DLLEXPORT cKhash_StringInsConstruct : public cKhash_StringIns<T,ismap>
  {
  protected:
    typedef typename cKhash_StringIns<T,ismap>::KHKEY KHKEY;
    typedef typename cKhash_StringIns<T,ismap>::KHVAL KHVAL;
    using cKhash_StringIns<T,ismap>::_h;

  public:
    cKhash_StringInsConstruct() : cKhash_StringIns<T,ismap>() {}
    ~cKhash_StringInsConstruct()
    {
      for(khiter_t iter=kh_begin(_h); iter<kh_end(_h); ++iter)
        if(kh_exist(_h,iter))
          kh_val(_h,iter).~T();
    }
    bool Insert(KHKEY key, const KHVAL& value)
		{
      if(!key) return false;
			if(kh_size(_h) >= _h->n_buckets) cKhash_StringIns<T,ismap>::_resize();
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
  class cSetting { };
  static cKhash_StringInsConstruct<std::function<void (cCmdLineArgs&,unsigned int&)>> csetting_cmdhash;

  // Class triggered when a setting is declared, adding it to the master command hash if necessary.
  struct AddToSettingHash { 
    AddToSettingHash(const char* cmdname, const std::function<void (cCmdLineArgs&,unsigned int&)>& func)
    { if(cmdname!=0) csetting_cmdhash.Insert(cmdname,func); }
  };

  // Static template function shortcut for getting a specific setting
  template<int I, int N>
  inline static typename cSetting<I,N>::TYPE& Setting() { return cSetting<I,N>::v; }

#ifdef INSTANTIATE_SETTINGS //Declare this in a CPP file that includes all DECL_SETTINGs used in your project to instantiate them
#define i_INST_SET_(I,N,T,INIT,NAME,CMD) T bss_util::cSetting<I,N>::v=INIT; \
  bss_util::AddToSettingHash bss_util::cSetting<I,N>::_shashinit(CMD,[](cCmdLineArgs& rcmd, unsigned int& ind) -> void { bss_util::cSetting_CMDLOAD<T>::CmdLoad(rcmd,bss_util::cSetting<I,N>::v,ind); })
#else
#define i_INST_SET_(I,N,T,INIT,NAME,CMD) 
#endif

  // Main #define for declaring a setting. NAME and CMD can both be set to 0 if INIs and command line parsing, respectively, are not needed.
#define DECL_SETTING(I,N,T,INIT,NAME,CMD) template<> class bss_util::cSetting<I,N> { public: typedef T TYPE; static T v; \
  inline static const char* name() { return NAME; } inline static const char* cmd() { return CMD; } \
  static bss_util::AddToSettingHash _shashinit; }; i_INST_SET_(I,N,T,INIT,NAME,CMD);

  /* Main #define for declaring a group of settings. Note that the optional NAME parameter is for INI loading and determines the section
     name of the settings. MAX is the maximum number of settings in this group, but unless you are using LoadAllFromINI, it is optional. */
#define DECL_SETGROUP(I,NAME,MAX) namespace bss_util { template<int N> class cSetting<I,N> { public: inline static const char* secname() { return NAME; } static const unsigned int COUNT=(MAX-1); }; }

  // Struct class for defining the INILoad function. Can be overriden for custom types
  template<typename T>
  struct cSetting_INILOAD {
    inline static void INILoad(cINIstorage& ini, T& v, const char* name, const char* section)
    { 
      cINIentry* p;
      if(name!=0 && (p=ini.GetEntryPtr(section,name))!=0) { v=*p; }
    }
  };
  
  template<>
  struct cSetting_INILOAD<std::vector<cStr>> {
    inline static void INILoad(cINIstorage& ini, std::vector<cStrT<char>>& v, const char* name, const char* section) { 
      cINIentry* p;
      v.clear();
      if(name!=0) {
        for(int i = 0; (p=ini.GetEntryPtr(section,name,i))!=0; ++i) { v.push_back(p->GetString()); }
      }
    }
  };

  // Struct class for defining the INISave function. Can be overriden for custom types
  template<typename T>
  struct cSetting_INISAVE {
    inline static void INISave(cINIstorage& ini, T& v, const char* name, const char* section)
    { if(name!=0) { ini.EditAddEntry(section,name,tostring<T>(v).c_str()); } }
  };
  
  // Override for string types on INI loading
  template<>
  struct cSetting_INISAVE<const char*> {
    inline static void INISave(cINIstorage& ini, const char* v, const char* name, const char* section)
    { if(name!=0) { ini.EditAddEntry(section,name,v); } }
  };
  // Force char types to write as numbers
  template<>
  struct cSetting_INISAVE<unsigned char> {
    inline static void INISave(cINIstorage& ini, unsigned char v, const char* name, const char* section)
    { char s[4]; ITOAx0((int)v,s,10); if(name!=0) { ini.EditAddEntry(section,name,s); } }
  };
  template<>
  struct cSetting_INISAVE<char> {
    inline static void INISave(cINIstorage& ini, char v, const char* name, const char* section)
    { char s[4]; ITOAx0((int)v,s,10); if(name!=0) { ini.EditAddEntry(section,name,s); } }
  };
  template<>
  struct cSetting_INISAVE<std::vector<cStr>> {
    inline static void INISave(cINIstorage& ini, const std::vector<cStr>& v, const char* name, const char* section)
    { if(name!=0) { for(unsigned int i = 0; i < v.size(); ++i) ini.EditAddEntry(section,name,v[i],i,0); } }
  };

  // Struct class for defining the CmdLoad function. Can be overriden for custom types
  template<typename T>
  struct cSetting_CMDLOAD {
    inline static void CmdLoad(cCmdLineArgs& ini, T& v, unsigned int& index)
    { std::stringstream(std::string(ini[index++]), std::stringstream::in) >> v; }
  };

  template<>
  struct cSetting_CMDLOAD<const char*> {
    inline static void CmdLoad(cCmdLineArgs& ini, const char*& s, unsigned int& index)
    { s=ini[index++]; }
  };

  template<>
  struct cSetting_CMDLOAD<bool> {
    inline static void CmdLoad(cCmdLineArgs& ini, bool& v, unsigned int& index)
    { v=true; }
  };

  template<>
  struct cSetting_CMDLOAD<std::vector<cStr>> {
    inline static void CmdLoad(cCmdLineArgs& ini, std::vector<cStr>& v, unsigned int& index) {} //You cannot load this from the command line
  };

  // Main class for managing settings. Here you can load and save settings from INIs
  template<int I,int N>
  class cSettingManage
  {
  public:
    inline static void LoadFromINI(cINIstorage&& ini) { LoadFromINI(ini); } // GCC actually needs these defined for some godawful reason.
    inline static void LoadAllFromINI(cINIstorage&& ini) { LoadAllFromINI(ini); }
    inline static void SaveToINI(cINIstorage&& ini) { SaveToINI(ini); }
    inline static void SaveAllToINI(cINIstorage&& ini) { SaveAllToINI(ini); }
    inline static void LoadFromINI(cINIstorage& ini)
    {
      cSetting_INILOAD<typename cSetting<I,N>::TYPE>::INILoad(ini, cSetting<I,N>::v, cSetting<I,N>::name(), cSetting<I,-1>::secname());
      cSettingManage<I,N-1>::LoadFromINI(ini);
    };
    inline static void LoadAllFromINI(cINIstorage& ini)
    {
      cSettingManage<I,cSetting<I,-1>::COUNT>::LoadFromINI(ini);
      cSettingManage<I-1,N>::LoadAllFromINI(ini);
    };
    inline static void SaveToINI(cINIstorage& ini)
    {
      cSetting_INISAVE<typename cSetting<I,N>::TYPE>::INISave(ini, cSetting<I,N>::v, cSetting<I,N>::name(), cSetting<I,-1>::secname());
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
  inline void LoadFromCmd(cCmdLineArgs& cmd) 
  {
    unsigned int i=0; // Must be unsigned because of what pfunc accepts
    while(i<cmd.Size())
    {
      std::function<void (cCmdLineArgs&,unsigned int&)>* pfunc = csetting_cmdhash[cmd[i]];
      ++i;
      if(pfunc!=0)
        (*pfunc)(cmd,i); //function must increment i
    }
  }
}

#endif
