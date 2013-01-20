.PHONY: all clean distclean

all:
	make -f bss-util.mk
	make -f test.mk

clean:
	make clean -f bss-util.mk
	make clean -f test.mk

dist:
	make dist -f bss-util.mk
	make dist -f test.mk

distclean:
	make distclean -f bss-util.mk
	make distclean -f test.mk