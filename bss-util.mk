TARGET := libbss_util.so
SRCDIR := bss_util
BUILDDIR := bin
OBJDIR := bin/obj
C_SRCS := $(wildcard $(SRCDIR)/*.c)
#C_SRCS := $(SRCDIR)/bss_util_c.c $(SRCDIR)/INIParser.c
CXX_SRCS := $(wildcard $(SRCDIR)/*.cpp)
#CXX_SRCS := bss_DebugInfo.cpp bss_Log.cpp bss_util.cpp cCmdLineArgs.cpp cHighPrecisionTimer.cpp cINIentry.cpp cINIsection.cpp cINIstorage.cpp
INCLUDE_DIRS := include include/shiny
LIBRARY_DIRS := 
LIBRARIES := rt

CPPFLAGS += -fPIC -D BSS_UTIL_EXPORTS -Wall -Wno-attributes -Wno-unknown-pragmas -Wno-reorder -Wno-missing-braces -std=gnu++0x
LDFLAGS += -shared

include base.mk

distclean:
	@- $(RM) $(OBJS)
	@- $(RM) -r $(OBJDIR)
