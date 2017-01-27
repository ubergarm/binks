/*

  Minimal HTTP Response Server Test
  Modified from [sustrik/dsock/tutorial/step3.c]
  Test with `wrk -t2 -c100 -d30s localhost:5555`

  Copyright (c) 2016 Martin Sustrik
  Copyright (c) 2017 John W. Leimgruber III

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

*/

#include <libdill.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dsock.h"

#define errno_assert(x) \
    do {\
        if(!(x)) {\
            fprintf(stderr, "%s [%d] (%s:%d)\n", strerror(errno),\
                (int)errno, __FILE__, __LINE__);\
            fflush(stderr);\
            abort();\
        }\
    } while(0)

coroutine void dialogue(int s) {
    int rc = bsend(s, "HTTP/1.1 200 OK\r\n\r\n", 19, -1);
    errno_assert(rc == 0);
    rc = hclose(s);
    errno_assert(rc == 0);
}

int main(int argc, char *argv[]) {

    int port = 5555;
    if(argc > 1)
        port = atoi(argv[1]);

    ipaddr addr;
    int rc = ipaddr_local(&addr, NULL, port, 0);
    errno_assert(rc == 0);
    int ls = tcp_listen(&addr, 10);
    if(ls < 0) {
        perror("Can't open listening socket");
        return 1;
    }

    int cnt=0;
    while(1) {
        int s = tcp_accept(ls, NULL, -1);
        errno_assert(s >= 0);
        int cr = go(dialogue(s));
        printf("%d\n", ++cnt);
        errno_assert(cr >= 0);
        hclose(cr);
    }
}
