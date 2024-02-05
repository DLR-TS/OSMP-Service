FROM ubuntu:22.04 AS osmp_builder
MAINTAINER frank.baumgarten@dlr.de

ENV DEBIAN_FRONTEND=noninteractive MAKEFLAGS="-j$(nproc)"
RUN apt-get update && apt-get install -y cmake build-essential pip git && rm -rf /var/lib/apt/lists/*
RUN pip install conan==1.59.0

RUN mkdir osmpservice && mkdir osmpservice/build
WORKDIR /osmpservice/build
COPY . /osmpservice/

RUN cmake .. -DBUILD_SHARED_LIBS=false -DCMAKE_BUILD_TYPE=Release
RUN cmake --build . --target OSMPService

FROM ubuntu:22.04
RUN apt-get update && apt-get install -y libprotobuf23 && rm -rf /var/lib/apt/lists/*
COPY --from=osmp_builder /osmpservice/build/bin/OSMPService .
ENTRYPOINT ./OSMPService
