FROM alpine

RUN apk add --no-cache build-base libnet curl libnet-dev curl-dev

WORKDIR /scanner

COPY *.c      ./
COPY Makefile ./
COPY inc      ./inc
COPY util     ./util

RUN make

ENTRYPOINT ["/scanner/massacr"] 
