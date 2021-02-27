#ifndef RUN_HEADER
#define RUN_HEADER

#include <string.h>

#include "constants.h"
#include "log.h"
#include "utils.h"

int run(struct Config *_config, struct Result *_result);

#define RESOURCE_UNLIMITED 0

#define CHILD_ERROR_EXIT(message)                                                           \
  {                                                                                         \
    log_fatal("child process error message: %s, strerror: %s; ", message, strerror(errno)); \
    close_fd(input_fd);                                                                     \
    close_fd(output_fd);                                                                    \
    close_fd(err_fd);                                                                       \
    raise(SIGUSR1);                                                                         \
    exit(EXIT_FAILURE);                                                                     \
  }

#define INTERNAL_ERROR_EXIT(message)                                                   \
  {                                                                                    \
    log_fatal("message: %s, Interlnal Error: strerror: %s", message, strerror(errno)); \
    exit(EXIT_FAILURE);                                                                \
  }

#endif
