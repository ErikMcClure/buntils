.PHONY: all clean distclean

all:
	make -f buntils.mk
	make -f test.mk

clean:
	make clean -f buntils.mk
	make clean -f test.mk

dist: all distclean
	tar -czf buntils-posix.tar.gz *

distclean:
	make distclean -f buntils.mk
	make distclean -f test.mk

debug:
	make debug -f buntils.mk
	make debug -f test.mk

install: all
	make install -f buntils.mk
  
uninstall:
	make uninstall -f buntils.mk