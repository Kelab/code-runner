#ifndef RUN_HEADER
#define RUN_HEADER

#include <string.h>

#include "log.h"
#include "utils.h"

struct result
{
  int status;
  int cpu_time_used;
  long cpu_time_used_us;
  int memory_used;
  long memory_used_b;
  int signal;
  int exit_code;
};

int run(char *args[], int time_limit, int memory_limit, char *in_file, char *out_file, struct result *_result);

#define CHILD_ERROR_EXIT(message)                                                             \
  {                                                                                           \
    log_fatal("Error: System errno: %s; Internal error message: " #message, strerror(errno)); \
    close_fd(input_fd);                                                                       \
    close_fd(output_fd);                                                                      \
    close_fd(err_fd);                                                                         \
    raise(SIGUSR1);                                                                           \
    exit(EXIT_FAILURE);                                                                       \
  }

#endif
