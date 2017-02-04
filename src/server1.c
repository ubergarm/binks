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
/* for dsock_assert() */
#include "/usr/local/src/dsock/utils.h"

int main(int argc, char *argv[]) {

    int port = 5555;
    if(argc > 1)
        port = atoi(argv[1]);

    ipaddr addr;
    int rc = ipaddr_local(&addr, NULL, port, 0);
    dsock_assert(rc == 0);
    int ls = tcp_listen(&addr, 10);
    if(ls < 0) {
        perror("Can't open listening socket");
        return 1;
    }

    while(1) {
        /* listen for incoming connections */
        int s = tcp_accept(ls, NULL, -1);
        dsock_assert(s >= 0);
        /* print message to console then close socket */
        printf("Accepted incoming TCP connection.\n");
        rc = hclose(s);
        dsock_assert(rc == 0);
    }
}

