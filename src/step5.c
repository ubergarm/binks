/*

  Copyright (c) 2016 Martin Sustrik

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

#define CONN_ESTABLISHED 1
#define CONN_SUCCEEDED 2
#define CONN_FAILED 3

coroutine void statistics(int ch) {
    int active = 0;
    int succeeded = 0;
    int failed = 0;
    
    while(1) {
        int op;
        int rc = chrecv(ch, &op, sizeof(op), -1);
        assert(rc == 0);

        switch(op) {
        case CONN_ESTABLISHED:
            ++active;
            break;
        case CONN_SUCCEEDED:
            --active;
            ++succeeded;
            break;
        case CONN_FAILED:
            --active;
            ++failed;
            break;
        }

        printf("active: %-5d  succeeded: %-5d  failed: %-5d\n",
            active, succeeded, failed);
    }
}

coroutine void dialogue(int s, int ch) {
    int op = CONN_ESTABLISHED;
    int rc = chsend(ch, &op, sizeof(op), -1);
    assert(rc == 0);
    int64_t deadline = now() + 10000;
    rc = msend(s, "What's your name?", 17, deadline);
    if(rc != 0) goto cleanup;
    char inbuf[256];
    ssize_t sz = mrecv(s, inbuf, sizeof(inbuf), deadline);
    if(sz < 0) goto cleanup;
    inbuf[sz] = 0;
    char outbuf[256];
    rc = snprintf(outbuf, sizeof(outbuf), "Hello, %s!", inbuf);
    rc = msend(s, outbuf, rc, deadline);
    if(rc != 0) goto cleanup;
cleanup:
    op = errno == 0 ? CONN_SUCCEEDED : CONN_FAILED;
    rc = chsend(ch, &op, sizeof(op), -1);
    assert(rc == 0);
    rc = hclose(s);
    assert(rc == 0);
}

int main(int argc, char *argv[]) {

    int port = 5555;
    if(argc > 1)
        port = atoi(argv[1]);

    ipaddr addr;
    int rc = ipaddr_local(&addr, NULL, port, 0);
    assert(rc == 0);
    int ls = tcp_listen(&addr, 10);
    if(ls < 0) {
        perror("Can't open listening socket");
        return 1;
    }

    int ch = chmake(sizeof(int));
    assert(ch >= 0);
    int cr = go(statistics(ch));
    assert(cr >= 0);

    while(1) {
        int s = tcp_accept(ls, NULL, -1);
        assert(s >= 0);
        s = crlf_start(s);
        assert(s >= 0);
        cr = go(dialogue(s, ch));
        assert(cr >= 0);
    }
}

