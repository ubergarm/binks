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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dsock.h"
#include "/usr/local/src/dsock/utils.h"

coroutine void worker(int ch) {
    while(1) {
        int s;
        int rc = chrecv(ch, &s, sizeof(s), -1);
        dsock_assert(rc == 0);
        /* start http protocol */
        int hs = http_start(s);
        dsock_assert(hs >= 0);

        /* receive http request header */
        char cmd[16];
        char url[16];
        rc = http_recvrequest(hs, cmd, sizeof(cmd), url, sizeof(url), -1);
        if( rc != 0) {
            printf("http_recvrequest() errno = %d\n", errno);
            goto cleanup;
        }
        printf("%s %s\n", cmd, url);

        /* receive all fields */
        while(1) {
            char name[32];
            char value[32];
            rc = http_recvfield(hs, name, sizeof(name), value, sizeof(value), -1);
            if( rc == -1 && errno == EPIPE ) break;
            if( rc != 0) {
                printf("http_recvfield() errno = %d\n", errno);
                goto cleanup;
            }
            printf("%s %s\n", name, value);
        }
        printf("\n");

        /* receive data if content length specified */
        /* TODO */

        /* send reply */
        rc = http_sendstatus(hs, 200, "OK", -1);
        if( rc != 0) {
            printf("http_sendstatus() errno = %d\n", errno);
            goto cleanup;
        }
        rc = http_done(hs, -1);
        dsock_assert(rc == 0);

cleanup:
        rc = hclose(s);
        dsock_assert(rc == 0);
    }
}

int main(int argc, char *argv[]) {

    int port = 5555;
    int numworkers = 3;
    if(argc > 1)
        port = atoi(argv[1]);
    if(argc > 2)
        numworkers = atoi(argv[2]);

    /* open TCP listener on specified socket */
    ipaddr addr;
    int rc = ipaddr_local(&addr, NULL, port, 0);
    dsock_assert(rc == 0);
    int ls = tcp_listen(&addr, 10);
    dsock_assert(ls >= 0);

    /* create channel to push handles to workers */
    int ch = chmake(sizeof(int));
    dsock_assert(ch >= 0);

    /* spin up specified number of worker coroutines < ~32k */
    for(int i=0;i<numworkers;i++) {
        printf("Starting coroutine: %d\n", i+1);
        int cr = go(worker(ch));
        dsock_assert(cr >= 0);
    }

    while(1) {
        /* accept incoming connections */
        int s = tcp_accept(ls, NULL, -1);
        dsock_assert(s >= 0);
        /* push incoming connection handle to channel */
        int rc = chsend(ch, &s, sizeof(s), -1);
        dsock_assert(rc >= 0);
    }
}

