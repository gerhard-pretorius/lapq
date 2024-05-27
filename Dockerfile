#---------------------------------------------------------------
from alpine:3.19 as build-lib
run apk update && \
  apk add \
  binutils=2.41-r0 \
  libtool autoconf automake pkgconf linux-headers \
  make \
  clang17=17.0.5-r0 \
  openssl-dev=3.1.5-r0 \
  cmake=3.27.8-r0 

env CC=clang CXX=clang++
env CXXFLAGS="-std=c++20 -DASIO_STANDALONE -DASIO_DYN_LINK"
env ASIO_PREFIX=/opt

workdir /opt/asio
copy lib/asio-1.30.2 .

run autoreconf -i \
&& ./configure --prefix=$ASIO_PREFIX \
      --without-boost \
      --enable-separate-compilation \
&& make && make install

workdir /opt/include
copy lib/ttmath-0.9.4/ttmath ./ttmath


#---------------------------------------------------------------
from build-lib as build-src

env CXXFLAGS="-std=c++20"

workdir /opt/lapq

copy src ./src
copy test ./test
copy CMakeLists.txt .
copy Doxyfile.in .

workdir /opt/lapq/build

run cmake -DCMAKE_BUILD_TYPE=Release \
          -DASIO_INSTALL=${ASIO_PREFIX}/include \ 
          .. \
&& cmake --build .


#---------------------------------------------------------------
entrypoint [ "/bin/sh" ]

