/*

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

coroutine void worker(int ch) {
    // TODO: exit cleanly on hclose() or error
    while(1) {
        // TODO: actually read the request data
        int s;
        int rc = chrecv(ch, &s, sizeof(s), -1);
        errno_assert(rc == 0);
        rc = bsend(s, "HTTP/1.1 200 OK\r\n\r\n", 19, -1);
        errno_assert(rc == 0);
        rc = hclose(s);
        errno_assert(rc == 0);
    }
}

int main(int argc, char *argv[]) {

    int port = 5555;
    int numworkers = 10;
    if(argc > 1)
        port = atoi(argv[1]);
    if(argc > 2)
        numworkers = atoi(argv[2]);

    ipaddr addr;
    int rc = ipaddr_local(&addr, NULL, port, 0);
    errno_assert(rc == 0);
    int ls = tcp_listen(&addr, 10);
    if(ls < 0) {
        perror("Can't open listening socket");
        return 1;
    }

    int ch = chmake(sizeof(int));
    errno_assert(ch >= 0);

    // TODO show actual info in proper log format instead of this fake stuff
    printf("Binks: Goin' Fast @ %s://%s:%d\n", "http", "0.0.0.0", port);
    printf("INFO: Spinning up %d workers per %d thread(s)...\n", numworkers, 1);

    for(int i=0;i<numworkers;i++) {
        // TODO: push these handles on a stack to garbage collect later
        int cr = go(worker(ch));
        errno_assert(cr >= 0);
    }

    // TODO listen for <control>-<c> signal and exit cleanly
    while(1) {
        int s = tcp_accept(ls, NULL, -1);
        errno_assert(s >= 0);
        int rc = chsend(ch, &s, sizeof(s), -1);
        errno_assert(rc >= 0);
    }
    hclose(ch);
    // TODO: clean up coroutine handles
}
