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

void clean()
{
  CLOSE_FP(log_fp);
}

int main(int argc, char *argv[])
{
  if (geteuid())
  {
    fprintf(stderr, "Must be started as root\n");
    return 1;
  }

  if (getegid() && setegid(0) < 0)
  {
    fprintf(stderr, "Cannot switch to root group\n");
    return 1;
  }

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
