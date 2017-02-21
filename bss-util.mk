TARGET := libbss-util.so
SRCDIR := bss-util
BUILDDIR := bin
OBJDIR := bin/obj
C_SRCS := $(wildcard $(SRCDIR)/*.c)
CXX_SRCS := $(wildcard $(SRCDIR)/*.cpp)
INCLUDE_DIRS := include
LIBRARY_DIRS := 
LIBRARIES := rt :libfontconfig.so.1

CPPFLAGS += -fPIC -D BSS_UTIL_EXPORTS -std=gnu++11 -Wall -Wno-attributes -Wno-unknown-pragmas -Wno-reorder -Wno-missing-braces -Wno-unused-function -Wno-comment -Wno-char-subscripts
LDFLAGS += -shared

include base.mk

distclean:
	@- $(RM) $(OBJS)
	@- $(RM) -r $(OBJDIR)
