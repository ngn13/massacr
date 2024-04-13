HEADERS = $(wildcard inc/*.h)
CSRCS   = $(wildcard *.c) $(wildcard */*.c)
VERSION = 1.2
prefix  = /usr
CC      = gcc

massacr: $(CSRCS) $(HEADERS)
	$(CC) $(CFLAGS) $(CSRCS) -o $@ -DVERSION=\"${VERSION}\" -lnet -lcurl 

install:
	install -v -m755 massacr $(DESTDIR)$(prefix)/bin/massacr 

uninstall:
	rm -v $(DESTDIR)$(prefix)/bin/massacr

format:
	clang-format -i -style=file ./*.c ./*/*.c ./*/*.h

.PHONY: install uninstall format
