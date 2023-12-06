FROM ubuntu:22.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake \
    make \
    ninja-build \
    g++ \
    g++-12 \
    git \
    ca-certificates \
    sudo \
  && rm -rf /var/lib/apt/lists/* \
  && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 11 \
  && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 11 \
  && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 12 \
  && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 12 \
  && useradd worker \
  && usermod -aG sudo worker \
  && echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

USER worker
ADD --chown=worker:worker . /workdir
WORKDIR /workdir

# RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
#   && cmake --build build/
