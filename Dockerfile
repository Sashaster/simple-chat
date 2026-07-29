FROM ubuntu:22.04 AS builder
WORKDIR /app
RUN apt update && apt install -y build-essential cmake software-properties-common && \
    add-apt-repository -y ppa:ubuntu-toolchain-r/test && apt update && \
    apt install -y gcc-13 g++-13
COPY src/ src/
COPY include/ include/
COPY CMakeLists.txt .
RUN CC=gcc-13 CXX=g++-13 cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF && \
        cmake --build build

FROM ubuntu:22.04
WORKDIR /app
RUN apt update && apt install -y software-properties-common && \
    add-apt-repository -y ppa:ubuntu-toolchain-r/test && \
    apt update && \
    apt install -y libstdc++6
COPY --from=builder /app/build/chat .
ENTRYPOINT ["./chat"]
CMD ["server"]