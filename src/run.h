#ifndef RUN_HEADER
#define RUN_HEADER

#include <string.h>
#include <signal.h>

#include "constants.h"
#include "log.h"
#include "utils.h"

int run();

#define RESOURCE_UNLIMITED 0

#define CHILD_ERROR_EXIT(message)                                                               \
  {                                                                                             \
    int _errno = errno;                                                                         \
    log_fatal("child error: %s, errno: %d, strerror: %s; ", message, _errno, strerror(_errno)); \
    CLOSE_FD(input_fd);                                                                         \
    CLOSE_FD(output_fd);                                                                        \
    CLOSE_FD(err_fd);                                                                           \
    CLOSE_FD(null_fd);                                                                          \
    if (_errno == 2)                                                                            \
      raise(SIGUSR2);                                                                           \
    else                                                                                        \
      raise(SIGUSR1);                                                                           \
    exit(_errno);                                                                               \
  }

#define INTERNAL_ERROR_EXIT(message)                                                            \
  {                                                                                             \
    log_fatal("Interlnal Error: %s, errno: %d, strerror: %s", message, errno, strerror(errno)); \
    exit(EXIT_FAILURE);                                                                         \
  }

#endif
