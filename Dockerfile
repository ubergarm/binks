FROM alpine:edge

RUN apk add --no-cache git \
                       build-base \
                       autoconf \
                       automake \
                       libtool

RUN mkdir -p /usr/local/src

# install libdill (skip make check as tests/threads *sometimes* fails WTF?)
RUN cd /usr/local/src && \
    git clone https://github.com/sustrik/libdill.git && \
    cd libdill && \
    ./autogen.sh && \
    ./configure --disable-shared --enable-debug && \
    make -j8 && \
    make install

# install libdsock (skip make check as tests/bthrottler sometimes fails???)
RUN cd /usr/local/src && \
    git clone https://github.com/sustrik/dsock.git && \
    cd dsock && \
    ./autogen.sh && \
    ./configure --disable-shared --enable-debug && \
    make -j8 && \
    make install

# install picohttpparser
RUN cd /usr/local/src && \
    git clone https://github.com/h2o/picohttpparser.git && \
    cd picohttpparser && \
    git submodule init && \
    git submodule update && \
    make test

COPY . /app

WORKDIR /app

CMD /bin/sh
