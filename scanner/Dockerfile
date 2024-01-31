FROM alpine

RUN apk update && apk upgrade
RUN apk add --no-cache build-base libnet curl libnet-dev curl-dev

WORKDIR /app

COPY *.c      ./
COPY Makefile ./
COPY inc      ./inc
COPY util     ./util

RUN make

ENTRYPOINT ["/app/massacr"] 
