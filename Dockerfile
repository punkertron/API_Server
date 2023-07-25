FROM debian:bullseye

LABEL author="punkertron"

EXPOSE 18080

RUN apt-get update && \
    apt-get install -y \
        g++ \
        make \
        libpq-dev \
        git \
        libboost-dev \
        python3 \
        libcrypto++-dev \
        wget && \
        git clone https://github.com/jtv/libpqxx.git && \
        cd libpqxx && \
        git checkout 7.7.5 && \
        ./configure --enable-static --disable-shared CXXFLAGS="-std=c++17 -O3" --disable-documentation && \
        make -j$(nproc) && \
        make install && \
        cd ../ && \
        rm -rf libpqxx && \
        wget https://github.com/CrowCpp/Crow/releases/download/v1.0%2B5/crow-v1.0+5.deb && \
        dpkg -i crow-v1.0+5.deb && \
        rm crow-v1.0+5.deb && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /server

COPY incs incs
COPY srcs srcs
COPY MakefileServer/Makefile Makefile

RUN make -j$(nproc) && \
    make clean

ENTRYPOINT ["./server"]
