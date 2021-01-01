#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include "constants.h"
#include "diff.h"
#include "log.h"
#include "utils.h"

#define RETURN(rst) \
  {                 \
    *status = rst;  \
    return 0;       \
  }

int check_diff(int rightout_fd, int userout_fd, int *status)
{
  char *userout, *rightout;
  const char *cuser, *cright, *end_user, *end_right;

  off_t userout_len, rightout_len;
  rightout_len = lseek(rightout_fd, 0, SEEK_END);
  userout_len = lseek(userout_fd, 0, SEEK_END);

  if (userout_len == -1)
  {
    log_error("lseek userout_len failure: %s\n", strerror(errno));
    _exit(1);
  }

  if (rightout_len == -1)
  {
    log_error("lseek userout_len failure: %s\n", strerror(errno));
    _exit(1);
  }

  if (userout_len >= MAX_OUTPUT)
    RETURN(OUTPUT_LIMIT_EXCEEDED);

  lseek(userout_fd, 0, SEEK_SET);
  lseek(rightout_fd, 0, SEEK_SET);

  if ((userout_len && rightout_len) == 0)
  {
    if (userout_len || rightout_len)
      RETURN(WRONG_ANSWER)
    else
      RETURN(ACCEPTED)
  }

  if ((userout = (char *)mmap(NULL, userout_len, PROT_READ | PROT_WRITE,
                              MAP_PRIVATE, userout_fd, 0)) == MAP_FAILED)
  {
    log_error("mmap userout filure\n");
    _exit(1);
  }

  if ((rightout = (char *)mmap(NULL, rightout_len, PROT_READ | PROT_WRITE,
                               MAP_PRIVATE, rightout_fd, 0)) == MAP_FAILED)
  {
    munmap(userout, userout_len);
    log_error("mmap right filure\n");
    _exit(1);
  }

  if ((userout_len == rightout_len) && equalStr(userout, rightout) == 0)
  {
    munmap(userout, userout_len);
    munmap(rightout, rightout_len);
    RETURN(ACCEPTED);
  }

  cuser = userout;
  cright = rightout;
  end_user = userout + userout_len;
  end_right = rightout + rightout_len;
  while ((cuser < end_user) && (cright < end_right))
  {
    while ((cuser < end_user) && (*cuser == ' ' || *cuser == '\n' || *cuser == '\r' || *cuser == '\t'))
      cuser++;
    while ((cright < end_right) && (*cright == ' ' || *cright == '\n' || *cright == '\r' || *cright == '\t'))
      cright++;
    if (cuser == end_user || cright == end_right)
      break;
    if (*cuser != *cright)
      break;
    cuser++;
    cright++;
  }
  while ((cuser < end_user) && (*cuser == ' ' || *cuser == '\n' || *cuser == '\r' || *cuser == '\t'))
    cuser++;
  while ((cright < end_right) && (*cright == ' ' || *cright == '\n' || *cright == '\r' || *cright == '\t'))
    cright++;
  if (cuser == end_user && cright == end_right)
  {
    munmap(userout, userout_len);
    munmap(rightout, rightout_len);
    RETURN(PRESENTATION_ERROR);
  }

  munmap(userout, userout_len);
  munmap(rightout, rightout_len);
  RETURN(WRONG_ANSWER);
}

int diff(struct Config *config, int *status)
{
  int right_fd = open(config->out_file, O_RDONLY, 0644);
  int userout_fd = open(config->user_out_file, O_RDONLY, 0644);
  check_diff(right_fd, userout_fd, status);
  close_fd(right_fd);
  close_fd(userout_fd);
  return 0;
}
