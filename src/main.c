#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <sys/utsname.h>
#include <sched.h>
#include <stdint.h>
#include <sys/mman.h>

#include "argv.h"
#include "constants.h"
#include "log.h"
#include "utils.h"
#include "sandbox.h"

extern struct Config runner_config;
extern struct Result runner_result;

FILE *log_fp = NULL;

// 100 MB
#define STACK_SIZE (100 * 1024 * 1024) /* Stack size(bytes) for cloned child */

int run_in_sandbox()
{
  /* Allocate memory to be used for the stack of the child. */
  char *stack; /* Start of stack buffer */

  // Stacks grow downward on all
  // processors that run Linux (except the HP PA processors), so stack
  // usually points to the topmost address of the memory space set up
  // for the child stack.
  stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
               MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
  if (stack == MAP_FAILED)
    INTERNAL_ERROR_EXIT("Cannot run proxy, mmap err");

  pid_t proxy_pid = clone(
      sandbox_proxy,
      stack + STACK_SIZE,
      SIGCHLD | CLONE_NEWIPC | (runner_config.share_net ? 0 : CLONE_NEWNET) | CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWUTS,
      0);
  if (proxy_pid < 0)
    INTERNAL_ERROR_EXIT("Cannot run proxy, clone failed");
  if (!proxy_pid)
    INTERNAL_ERROR_EXIT("Cannot run proxy, clone returned 0");

  int stat;
  pid_t p = waitpid(proxy_pid, &stat, 0);

  if (p < 0)
    INTERNAL_ERROR_EXIT("Cannot run proxy, waitpid() failed");
  return 0;
}

void clean()
{
  CLOSE_FP(log_fp);
}

int main(int argc, char *argv[])
{
  log_set_quiet(true);
  parse_argv(argc, argv);

  if (runner_config.log_file)
  {
    log_fp = fopen(runner_config.log_file, "a");
    if (log_fp != NULL)
    {
      log_add_fp(log_fp, LOG_DEBUG);
      log_config();
    }
  }
  run_in_sandbox();
  atexit(clean);
  return 0;
}
