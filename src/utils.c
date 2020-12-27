#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"

void split(char **arr, char *str, const char *del)
{
  char *s = NULL;
  s = strtok(str, del);
  while (s != NULL)
  {
    *arr++ = s;
    s = strtok(NULL, del);
  }
  *arr++ = NULL;
}

void close_fd(int fd)
{
  if (fd > 0)
  {
    close(fd);
  }
}

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
