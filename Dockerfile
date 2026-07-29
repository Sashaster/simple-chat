FROM ubuntu:22.04
WORKDIR /app
COPY build/chat .
RUN chmod +x ./chat
ENTRYPOINT ["./chat"]
CMD ["server"]

