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

#define RETURN(rst)       \
  {                       \
    result->status = rst; \
    return 0;             \
  }

int check_diff(int rightout_fd, int userout_fd, struct Result *result)
{
  char *userout, *rightout;
  const char *cuser, *cright, *end_user, *end_right;

  off_t userout_len, rightout_len;
  rightout_len = lseek(rightout_fd, 0, SEEK_END);
  userout_len = lseek(userout_fd, 0, SEEK_END);

  if (userout_len == -1)
  {
    log_error("lseek userout_len failure: %s\n", strerror(errno));
    RETURN(SYSTEM_ERROR);
  }

  if (rightout_len == -1)
  {
    log_error("lseek userout_len failure: %s\n", strerror(errno));
    RETURN(SYSTEM_ERROR);
  }

  lseek(userout_fd, 0, SEEK_SET);
  lseek(rightout_fd, 0, SEEK_SET);

  // 说明两个文件有一个长度为 0
  if ((userout_len && rightout_len) == 0)
  {
    // 如果有一个不为 0，则答案错误
    if (userout_len || rightout_len)
      RETURN(WRONG_ANSWER)
    else
      RETURN(ACCEPTED)
  }

  if ((userout = (char *)mmap(NULL, userout_len, PROT_READ,
                              MAP_PRIVATE, userout_fd, 0)) == MAP_FAILED)
  {
    munmap(userout, userout_len);
    log_error("mmap userout filure\n");
    RETURN(SYSTEM_ERROR);
  }

  if ((rightout = (char *)mmap(NULL, rightout_len, PROT_READ,
                               MAP_PRIVATE, rightout_fd, 0)) == MAP_FAILED)
  {
    munmap(rightout, rightout_len);
    log_error("mmap right filure\n");
    RETURN(SYSTEM_ERROR);
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
    // 逃逸掉中间输出结果的行末回车换行输出，可以只判断最后一行的
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

void diff(struct Config *config, struct Result *result)
{
  int right_fd = open(config->out_file, O_RDONLY, 0644);
  int userout_fd = open(config->user_out_file, O_RDONLY, 0644);
  check_diff(right_fd, userout_fd, result);
  close_fd(right_fd);
  close_fd(userout_fd);
}
