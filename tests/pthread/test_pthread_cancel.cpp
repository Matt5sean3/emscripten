// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <pthread.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <emscripten/em_asm.h>

_Atomic long res = 43;
static void cleanup_handler(void *arg)
{
  EM_ASM(out('Called clean-up handler with arg ' + $0), arg);
  long a = (long)arg;
  res -= a;
}

static void *thread_start(void *arg)
{
  pthread_cleanup_push(cleanup_handler, (void*)42);
  EM_ASM(out('Thread started!'));
  for(;;)
  {
    pthread_testcancel();
  }
  res = 1000; // Shouldn't ever reach here.
  pthread_cleanup_pop(0);
}

pthread_t thr;

int main()
{
  int s = pthread_create(&thr, NULL, thread_start, (void*)0);
  assert(s == 0);
  EM_ASM(out('Canceling thread..'););
  s = pthread_cancel(thr);
  assert(s == 0);

  for(;;)
  {
    int result = res;
    if (result == 1)
    {
      EM_ASM_INT( { out('After canceling, shared variable = ' + $0 + '.'); }, result);
      return 0;
    }
  }

  __builtin_unreachable();
}
