#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include "diff.h"
#include "constants.h"

int equalStr(const char *s, const char *s2)
{
  while (*s && *s2)
  {
    if (*s++ != *s2++)
    {
      return 1;
    }
  }

  return 0;
}

#define RETURN(rst) \
  {                 \
    *result = rst;  \
    return 0;       \
  }

int checkDiff(int rightout_fd, int userout_fd, int *result)
{
  char *userout, *rightout;
  const char *cuser, *cright, *end_user, *end_right;

  off_t userout_len, rightout_len;
  userout_len = lseek(userout_fd, 0, SEEK_END);
  rightout_len = lseek(rightout_fd, 0, SEEK_END);

  if (userout_len == -1 || rightout_len == -1)
  {
    printf("lseek failure\n");
    _exit(1);
  }

  if (userout_len >= MAX_OUTPUT)
    RETURN(OLE);

  lseek(userout_fd, 0, SEEK_SET);
  lseek(rightout_fd, 0, SEEK_SET);

  if ((userout_len && rightout_len) == 0)
  {
    if (userout_len || rightout_len)
      RETURN(WA)
    else
      RETURN(AC)
  }

  if ((userout = (char *)mmap(NULL, userout_len, PROT_READ | PROT_WRITE,
                              MAP_PRIVATE, userout_fd, 0)) == MAP_FAILED)
  {
    printf("mmap userout filure\n");
    _exit(1);
  }

  if ((rightout = (char *)mmap(NULL, rightout_len, PROT_READ | PROT_WRITE,
                               MAP_PRIVATE, rightout_fd, 0)) == MAP_FAILED)
  {
    munmap(userout, userout_len);
    printf("mmap right filure\n");
    _exit(1);
  }

  if ((userout_len == rightout_len) && equalStr(userout, rightout) == 0)
  {
    munmap(userout, userout_len);
    munmap(rightout, rightout_len);
    RETURN(AC);
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
    RETURN(PE);
  }

  munmap(userout, userout_len);
  munmap(rightout, rightout_len);
  RETURN(WA);
}
