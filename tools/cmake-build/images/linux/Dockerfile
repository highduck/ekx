FROM ubuntu:20.04

ENV TZ=Europe/Minsk
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update -y
RUN apt-get install curl -y

RUN curl -sL https://deb.nodesource.com/setup_15.x | bash
RUN apt-get install -y nodejs
RUN DEBIAN_FRONTEND=noninteractive npm install --global yarn

RUN node --version
RUN npm --version
RUN yarn --version

RUN apt-get install build-essential -y
RUN apt-get install clang-10 -y
RUN apt-get install llvm-10-dev -y
RUN ln -s /usr/bin/clang++-10 /usr/bin/clang++
RUN ln -s /usr/bin/clang-10 /usr/bin/clang
RUN ln -s /usr/bin/llvm-config-10 /usr/bin/llvm-config
RUN clang++ --version

RUN apt-get install ninja-build -y
RUN ninja --version

RUN curl -s "https://cmake.org/files/v3.19/cmake-3.19.4-Linux-x86_64.tar.gz" | tar --strip-components=1 -xz -C /usr/local
RUN cmake --version

RUN apt-get install xorg-dev -y
RUN apt-get install freeglut3-dev -y
RUN apt-get install libopengl0 -y
RUN apt-get clean
