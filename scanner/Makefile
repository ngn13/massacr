HEADERS = $(wildcard inc/*.h)
CSRCS   = $(wildcard *.c) $(wildcard */*.c)
CFLAGS  = -O3 -march=native -fstack-protector-strong -fcf-protection=full -fstack-clash-protection
LIBS    = -lpthread $(shell pkg-config --libs --cflags libnet) $(shell pkg-config --libs --cflags libmongoc-1.0)

VERSION = 1.3
prefix  = /usr
CC      = gcc

massacr: $(CSRCS) $(HEADERS)
	$(CC) $(CFLAGS) $(CSRCS) -o $@ -DVERSION=\"${VERSION}\" $(LIBS) 

install:
	install -v -m755 massacr $(DESTDIR)$(prefix)/bin/massacr 

uninstall:
	rm -v $(DESTDIR)$(prefix)/bin/massacr

format:
	clang-format -i -style=file ./*.c ./*/*.c ./*/*.h

.PHONY: install uninstall format
