TARGET := test
SRCDIR := test
BUILDDIR := bin
OBJDIR := bin/obj
C_SRCS := $(wildcard $(SRCDIR)/*.c)
CXX_SRCS := $(wildcard $(SRCDIR)/*.cpp)
INCLUDE_DIRS := include
LIBRARY_DIRS := 
LIBRARIES := buntils rt pthread

CPPFLAGS += -std=c++17 -pthread -DLIBICONV_PLUG -Wall -Wshadow -Wno-attributes -Wno-unknown-pragmas -Wno-reorder -Wno-missing-braces -Wno-unused-function -Wno-comment -Wno-char-subscripts -Wno-sign-compare -Wno-unused-variable -Wno-switch -fsanitize=signed-integer-overflow -fuse-ld=gold -Wno-class-memaccess
LDFLAGS += -L./bin/
  
include base.mk

PRECOMPILE:
	$(COMPILE.cpp) $(SRCDIR)/test.h -o $(SRCDIR)/test.h.gch

$(OBJS): PRECOMPILE
  
distclean:
	@- $(RM) $(OBJS)
	@- $(RM) -r $(OBJDIR)
	@- $(RM) bin/*.txt
	@- $(RM) bin/*.ini
	@- $(RM) bin/*.json
	@- $(RM) bin/*.ubj
	@- $(RM) bin/*.xml
	@- $(RM) $(SRCDIR)/test.h.gch

