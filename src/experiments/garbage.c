/*

  libdill example on how to garbage collect coroutine handles upon completion

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
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define errno_assert(x) \
    do {\
        if(!(x)) {\
            fprintf(stderr, "%s [%d] (%s:%d)\n", strerror(errno),\
                (int)errno, __FILE__, __LINE__);\
            fflush(stderr);\
            abort();\
        }\
    } while(0)

coroutine void dowork(int worker_ch, int id) {
    // do work
    msleep(now() + 5 * id);
    printf("Coroutine handle %d done.\n", id);
    int rc = chsend(worker_ch, &id, sizeof(id), -1);
}

coroutine void garbage(int worker_ch, int done_ch, int num_workers) {
    printf("Garbage collector starting.\n");
    for(int i=0;i<num_workers;i++) {
        int op;
        int rc = chrecv(worker_ch, &op, sizeof(op), -1);
        errno_assert(rc == 0);
        printf("Garbage collecting handle: %d\n", op);
        hclose(op);
    }
    int rc = chdone(done_ch);
    errno_assert(rc == 0);
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("usage: %s <number-of-workers>\n", argv[0]);
        return 1;
    }

    int num_workers = atoi(argv[1]);
    int done_ch = chmake(sizeof(int));
    errno_assert(done_ch >= 0);
    printf("done_ch handle: %d\n", done_ch);
    int worker_ch = chmake(sizeof(int));
    errno_assert(worker_ch >= 0);
    printf("worker_ch handle: %d\n", worker_ch);

    // spawn the garbage collector coroutine
    int gcr = go(garbage(worker_ch, done_ch, num_workers));
    errno_assert(gcr >= 0);
    printf("garbage collector handle is %d\n", gcr);

    // spawn N worker coroutines
    for(int i=0;i<num_workers;i++) {
        // offset by 3 because 2 channels and 1 coroutine already started
        // this is a _hack_! how can one get the handle inside the coroutine?
        // 1) this hack won't work as handles are free'd and recycled
        // 2) possibly send the coroutines handle into it through a channel?
        int handle_offset = 3;
        int cr = go(dowork(worker_ch, i+handle_offset));
        errno_assert(cr >= 0);
    }

    // block on done channel until someone calls chdone() on it
    printf("Main function blocking untile all garbage collected.\n");
    int op;
    int rc = chrecv(done_ch, &op, sizeof(op), -1);
    errno_assert(rc == -1);
    errno_assert(errno == EPIPE);

    // close garbage collector's coroutine
    hclose(gcr);

    // close channels
    hclose(worker_ch);
    hclose(done_ch);
}

