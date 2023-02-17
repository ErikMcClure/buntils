TARGET := libbuntils.so
SRCDIR := buntils
BUILDDIR := bin
OBJDIR := bin/obj
C_SRCS := $(wildcard $(SRCDIR)/*.c)
CXX_SRCS := $(wildcard $(SRCDIR)/*.cpp)
INCLUDE_DIRS := include
LIBRARY_DIRS := 
LIBRARIES := rt :libfontconfig.so.1

CPPFLAGS += -fPIC -D BSS_UTIL_EXPORTS -std=c++17 -DLIBICONV_PLUG -Wall -Wshadow -Wno-attributes -Wno-unknown-pragmas -Wno-reorder -Wno-missing-braces -Wno-unused-function -Wno-char-subscripts -fsanitize=signed-integer-overflow -fuse-ld=gold -Wno-class-memaccess
LDFLAGS += -shared

include base.mk

distclean:
	@- $(RM) $(OBJS)
	@- $(RM) -r $(OBJDIR)

PREFIX = /usr

.PHONY: install
install:
	test -d $(PREFIX) || mkdir $(PREFIX)
	test -d $(PREFIX)/lib || mkdir $(PREFIX)/lib
	test -d $(PREFIX)/include || mkdir $(PREFIX)/include
	test -d $(PREFIX)/include/$(SRCDIR) || mkdir $(PREFIX)/include/$(SRCDIR)
	cp $(BUILDDIR)/$(TARGET) $(PREFIX)/lib/$(TARGET)
	cp include/$(SRCDIR)/*.h $(PREFIX)/include/$(SRCDIR)/

.PHONY: uninstall
uninstall:
	rm -f $(PREFIX)/lib/$(TARGET)
	rm -f -r $(PREFIX)/include/$(SRCDIR)