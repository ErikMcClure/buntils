.PHONY: all clean distclean

all:
	make -f bss-util.mk
	make -f test.mk

clean:
	make clean -f bss-util.mk
	make clean -f test.mk

dist: all distclean
	tar -czf bss_util-posix.tar.gz *

distclean:
	make distclean -f bss-util.mk
	make distclean -f test.mk
