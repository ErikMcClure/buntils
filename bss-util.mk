TARGET := libbss_util.so
SRCDIR := bss_util
BUILDDIR := bin
OBJDIR := bin/obj
#C_SRCS := $(wildcard *.c)
C_SRCS := bss_util_c.c INIParser.c
#CXX_SRCS := $(wildcard *.cpp)
CXX_SRCS := bss_DebugInfo.cpp bss_Log.cpp bss_util.cpp cCmdLineArgs.cpp cHighPrecisionTimer.cpp cINIentry.cpp cINIsection.cpp cINIstorage.cpp
INCLUDE_DIRS := include
LIBRARY_DIRS := 
LIBRARIES := rt

CPPFLAGS += -fPIC -w -Wno-attributes -Wno-unknown-pragmas -Wno-reorder -std=gnu++0x
LDFLAGS += -shared

include base.mk
