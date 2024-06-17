FROM archlinux

RUN pacman -Syy --noconfirm make gcc pkgconf libnet mongo-c-driver

WORKDIR /scanner

COPY *.c      ./
COPY Makefile ./
COPY inc      ./inc
COPY util     ./util

RUN make

ENTRYPOINT ["/scanner/massacr"]
