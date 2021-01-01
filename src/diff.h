#ifndef DIFF_HEADER
#define DIFF_HEADER

#include "constants.h"

int check_diff(int rightout_fd, int userout_fd, int *status);
int diff(struct Config *config, int *status);

#endif
