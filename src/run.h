#ifndef RUN_HEADER
#define RUN_HEADER

#include <string.h>

#include "constants.h"
#include "log.h"
#include "utils.h"

int run(struct Config *_config, struct Result *_result);

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
