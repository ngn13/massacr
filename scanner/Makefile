HEADERS = $(wildcard inc/*.h)
CSRCS = $(wildcard *.c) $(wildcard */*.c)
prefix = /usr
CC = gcc

massacr: $(CSRCS) $(HEADERS)
	$(CC) $(CFLAGS) $(CSRCS) -o $@ -lnet -lcurl 

install:
	install -v -m755 massacr $(DESTDIR)$(prefix)/bin/massacr 

uninstall:
	rm -v $(DESTDIR)$(prefix)/bin/massacr

.PHONY: install uninstall
