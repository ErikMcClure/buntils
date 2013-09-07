TARGET := test
SRCDIR := test
BUILDDIR := bin
OBJDIR := bin/obj
C_SRCS := 
CXX_SRCS := main.cpp
INCLUDE_DIRS := include
LIBRARY_DIRS := 
LIBRARIES := bss_util rt pthread

CPPFLAGS += -w -std=gnu++0x
LDFLAGS += -L./bin/

include base.mk

distclean:
	@- $(RM) $(OBJS)
	@- $(RM) -r $(OBJDIR)
	@- $(RM) bin/*.txt
	@- $(RM) bin/*.ini
