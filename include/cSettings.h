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
  // Base template cSetting class containing the master cmdhash for processing command lines.
  template<int I, int N>
  class cSetting { };

  // Static template function shortcut for getting a specific setting
  template<int I, int N>
  inline static typename cSetting<I,N>::TYPE& Setting() { return cSetting<I,N>::v; }

#ifdef INSTANTIATE_SETTINGS //Declare this in a CPP file that includes all DECL_SETTINGs used in your project to instantiate them
#define i_INST_SET_(I,N,T,INIT,NAME) T bss_util::cSetting<I,N>::v=INIT;
#else
#define i_INST_SET_(I,N,T,INIT,NAME) 
#endif

  // Main #define for declaring a setting. NAME and CMD can both be set to 0 if INIs and command line parsing, respectively, are not needed.
#define DECL_SETTING(I,N,T,INIT,NAME) template<> class bss_util::cSetting<I,N> { public: typedef T TYPE; static T v; \
  inline static const char* name() { return NAME; } }; i_INST_SET_(I,N,T,INIT,NAME);

  /* Main #define for declaring a group of settings. Note that the optional NAME parameter is for INI loading and determines the section
     name of the settings. MAX is the maximum number of settings in this group, but unless you are using LoadAllFromINI, it is optional. */
#define DECL_SETGROUP(I,NAME) namespace bss_util { template<int N> class cSetting<I,N> { public: inline static const char* secname() { return NAME; } typedef void TYPE; }; }

  // Struct class for defining the INILoad function. Can be overriden for custom types
  template<typename T>
  inline static void cSetting_INILOAD(cINIstorage& ini, T& v, const char* name, const char* section)
  { 
    cINIentry* p;
    if(name!=0 && (p=ini.GetEntryPtr(section,name))!=0) { v=*p; }
  }
  
  template<>
  inline static void cSetting_INILOAD<std::vector<cStr>>(cINIstorage& ini, std::vector<cStr>& v, const char* name, const char* section)
  { 
    cINIentry* p;
    v.clear();
    if(name!=0) {
      for(int i = 0; (p=ini.GetEntryPtr(section,name,i))!=0; ++i) { v.push_back(p->GetString()); }
    }
  }
  
  template<>
  inline static void cSetting_INILOAD<std::vector<__int64>>(cINIstorage& ini, std::vector<__int64>& v, const char* name, const char* section)
  { 
    cINIentry* p;
    v.clear();
    if(name!=0) {
      for(int i = 0; (p=ini.GetEntryPtr(section,name,i))!=0; ++i) { v.push_back(p->GetInt()); }
    }
  }
  
  template<>
  inline static void cSetting_INILOAD<std::vector<int>>(cINIstorage& ini, std::vector<int>& v, const char* name, const char* section)
  { 
    cINIentry* p;
    v.clear();
    if(name!=0) {
      for(int i = 0; (p=ini.GetEntryPtr(section,name,i))!=0; ++i) { v.push_back(*p); }
    }
  }

  template<typename T>
  BSS_FORCEINLINE static std::string cSetting_tostring(T& v)
  {
    std::stringstream s;
    s << v; 
    return s.str();
  }

  // Struct class for defining the INISave function. Can be overriden for custom types
  template<typename T>
  inline static void cSetting_INISAVE(cINIstorage& ini, T& v, const char* name, const char* section)
  { if(name!=0) ini.EditAddEntry(section,name,cSetting_tostring<T>(v).c_str()); }
  
  // Override for string types on INI loading
  template<>
  inline static void cSetting_INISAVE<const char*>(cINIstorage& ini, const char*& v, const char* name, const char* section)
  { if(name!=0) { ini.EditAddEntry(section,name,v); } }
  // Force char types to write as numbers
  template<>
  inline static void cSetting_INISAVE<unsigned char>(cINIstorage& ini, unsigned char& v, const char* name, const char* section)
  { char s[4]; ITOAx0((int)v,s,10); if(name!=0) { ini.EditAddEntry(section,name,s); } }
  template<>
  inline static void cSetting_INISAVE<char>(cINIstorage& ini, char& v, const char* name, const char* section)
  { char s[4]; ITOAx0((int)v,s,10); if(name!=0) { ini.EditAddEntry(section,name,s); } }
  template<>
  inline static void cSetting_INISAVE<std::vector<cStr>>(cINIstorage& ini, std::vector<cStr>& v, const char* name, const char* section)
  { if(name!=0) { for(unsigned int i = 0; i < v.size(); ++i) ini.EditAddEntry(section,name,v[i],i,0); } }
  template<>
  inline static void cSetting_INISAVE<std::vector<__int64>>(cINIstorage& ini, std::vector<__int64>& v, const char* name, const char* section)
  { if(name!=0) { for(unsigned int i = 0; i < v.size(); ++i) ini.EditAddEntry(section,name,cSetting_tostring<__int64>(v[i]).c_str(),i,0); } }
  template<>
  inline static void cSetting_INISAVE<std::vector<int>>(cINIstorage& ini, std::vector<int>& v, const char* name, const char* section)
  { if(name!=0) { for(unsigned int i = 0; i < v.size(); ++i) ini.EditAddEntry(section,name,cSetting_tostring<int>(v[i]).c_str(),i,0); } }

  // Main class for managing settings. Here you can load and save settings from INIs
  template<int I,int N>
  class cSettingManage
  {
    template<int N>
    struct __internal { static const int VAL=N+1; };
    struct __internal2 { static const int VAL=-1; };

  public:
    inline static void LoadFromINI(cINIstorage&& ini) { LoadFromINI(ini); } // GCC actually needs these defined for some godawful reason.
    inline static void LoadAllFromINI(cINIstorage&& ini) { LoadAllFromINI(ini); }
    inline static void SaveToINI(cINIstorage&& ini) { SaveToINI(ini); }
    inline static void SaveAllToINI(cINIstorage&& ini) { SaveAllToINI(ini); }
    inline static void LoadFromINI(cINIstorage& ini)
    {
      cSetting_INILOAD<typename cSetting<I,N>::TYPE>(ini, cSetting<I,N>::v, cSetting<I,N>::name(), cSetting<I,-1>::secname());
      cSettingManage<I,std::conditional<std::is_void<cSetting<I,N+1>::TYPE>::value,__internal2,__internal<N>>::type::VAL>::LoadFromINI(ini);
    };
    inline static void LoadAllFromINI(cINIstorage& ini)
    {
      cSettingManage<I,0>::LoadFromINI(ini);
      cSettingManage<I-1,N>::LoadAllFromINI(ini);
    };
    inline static void SaveToINI(cINIstorage& ini)
    {
      cSetting_INISAVE<typename cSetting<I,N>::TYPE>(ini, cSetting<I,N>::v, cSetting<I,N>::name(), cSetting<I,-1>::secname());
      cSettingManage<I,std::conditional<std::is_void<cSetting<I,N+1>::TYPE>::value,__internal2,__internal<N>>::type::VAL>::SaveToINI(ini);
    };
    inline static void SaveAllToINI(cINIstorage& ini)
    {
      cSettingManage<I,0>::SaveToINI(ini);
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
}

#endif
