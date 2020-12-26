#include <stdlib.h>
#include <string.h>

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
