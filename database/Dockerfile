FROM golang:1.21.6

WORKDIR /app
COPY *.mod  ./
COPY *.sum  ./
COPY *.go   ./
COPY lib    ./lib
COPY static ./static

EXPOSE 3231
RUN go build .

ENTRYPOINT ["/app/database"]
