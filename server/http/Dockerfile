FROM gcc:14.2.0-bookworm AS build

COPY . /app
WORKDIR /app
RUN make

FROM busybox:glibc
COPY --from=build /app/bin/webby .
ENTRYPOINT ["./webby"]
