#ifndef RUN_HEADER
#define RUN_HEADER

#include <string.h>

#include "constants.h"
#include "log.h"
#include "utils.h"

int run(struct Config *, struct Result *);

#define RESOURCE_UNLIMITED 0

#define CHILD_ERROR_EXIT(message)                                                             \
  {                                                                                           \
    log_fatal("child error: %s, errno: %d, strerror: %s; ", message, errno, strerror(errno)); \
    CLOSE_FD(input_fd);                                                                       \
    CLOSE_FD(output_fd);                                                                      \
    CLOSE_FD(err_fd);                                                                         \
    CLOSE_FD(null_fd);                                                                        \
    /** 想一个方法，前两位放 signal 后几位放 errno */                           \
    raise(SIGUSR1);                                                                           \
    exit(errno);                                                                              \
  }

#define INTERNAL_ERROR_EXIT(message)                                                            \
  {                                                                                             \
    log_fatal("Interlnal Error: %s, errno: %d, strerror: %s", message, errno, strerror(errno)); \
    exit(EXIT_FAILURE);                                                                         \
  }

#endif
