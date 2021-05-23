#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <limits.h>

#include "constants.h"
#include "log.h"
#include "utils.h"

int equalStr(const char *s, const char *s2)
{
  while (*s && *s2)
  {
    if (*s++ != *s2++)
    {
      return 0;
    }
  }

  return 1;
}

void format_result(char *message)
{
  sprintf(message, "{\n"
                   "  \"status\": %d,\n"
                   "  \"cpu_time_used\": %d,\n"
                   "  \"cpu_time_used_us\": %ld,\n"
                   "  \"real_time_used\": %d,\n"
                   "  \"real_time_used_us\": %ld,\n"
                   "  \"memory_used\": %d,\n"
                   "  \"error_code\": %d,\n"
                   "  \"signal_code\": %d,\n"
                   "  \"exit_code\": %d\n"
                   "}",
          runner_result.status,
          runner_result.cpu_time_used,
          runner_result.cpu_time_used_us,
          runner_result.real_time_used,
          runner_result.real_time_used_us,
          runner_result.memory_used,
          runner_result.error_code,
          runner_result.signal_code,
          runner_result.exit_code);
}

long tv_to_ms(const struct timeval *tv)
{
  return (tv->tv_sec * 1000) + ((tv->tv_usec + 500) / 1000);
}

long tv_to_us(const struct timeval *tv)
{
  return (tv->tv_sec * 1000 * 1000) + tv->tv_usec;
}

int write_file(const char *filename, const char *content)
{
  FILE *fptr = fopen(filename, "w");
  if (fptr == NULL)
  {
    log_error("open %s error", filename);
    return 1;
  }
  fprintf(fptr, "%s", content);
  fclose(fptr);
  return 0;
}
