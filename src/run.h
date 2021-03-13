#ifndef RUN_HEADER
#define RUN_HEADER

#include <string.h>

#include "constants.h"
#include "log.h"
#include "utils.h"

int run(struct Config *, struct Result *);

#define RESOURCE_UNLIMITED 0

#define CHILD_ERROR_EXIT(message)                                                           \
  {                                                                                         \
    log_fatal("child process error message: %s, strerror: %s; ", message, strerror(errno)); \
    CLOSE_FD(input_fd);                                                                     \
    CLOSE_FD(output_fd);                                                                    \
    CLOSE_FD(err_fd);                                                                       \
    CLOSE_FP(re_out);                                                                       \
    CLOSE_FP(re_err);                                                                       \
    raise(SIGUSR1);                                                                         \
    exit(EXIT_FAILURE);                                                                     \
  }

#define INTERNAL_ERROR_EXIT(message)                                                   \
  {                                                                                    \
    log_fatal("message: %s, Interlnal Error: strerror: %s", message, strerror(errno)); \
    exit(EXIT_FAILURE);                                                                \
  }

#endif
