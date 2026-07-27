FROM ubuntu:22.04
WORKDIR /app
COPY build/chat .
ENTRYPOINT ["./chat"]
CMD ["server"]

