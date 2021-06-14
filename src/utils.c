#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>

extern struct Config runner_config;
extern struct Result runner_result;

#include "constants.h"
#include "log.h"
#include "utils.h"

int str_equal(const char *s, const char *s2)
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

int format_result(char *message)
{
  return sprintf(message, "{\n"
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

void log_config()
{
  char buf[256] = {'\0'};
  join_str(buf, sizeof(buf), " ", runner_config.cmd);
  log_debug("config: cmd %s", buf);
  log_debug("config: memory_check_only %d", runner_config.memory_check_only);
  log_debug("config: cpu_time_limit %d ms", runner_config.cpu_time_limit);
  log_debug("config: real_time_limit %d ms", runner_config.real_time_limit);
  log_debug("config: memory_limit %d kb", runner_config.memory_limit);
  log_debug("config: attach: STDIN %d | STDOUT %d | STDERR %d", runner_config.std_in, runner_config.std_out, runner_config.std_err);
  log_debug("config: in_file %s", runner_config.in_file);
  log_debug("config: out_file %s", runner_config.out_file);
  log_debug("config: savefile %s", runner_config.save_file);
  log_debug("config: stdout_file %s", runner_config.stdout_file);
  log_debug("config: stderr_file %s", runner_config.stderr_file);
  log_debug("config: log_file %s", runner_config.log_file);
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

void setup_pipe(int *fds, int nonblocking)
{
  if (pipe(fds) < 0)
  {
    INTERNAL_ERROR_EXIT("pipe");
  }
  for (int i = 0; i < 2; i++)
  {
    if (fcntl(fds[i], F_SETFD, fcntl(fds[i], F_GETFD) | FD_CLOEXEC) < 0 ||
        (nonblocking && fcntl(fds[i], F_SETFL, fcntl(fds[i], F_GETFL) | O_NONBLOCK) < 0))
    {
      INTERNAL_ERROR_EXIT("fcntl on pipe");
    }
  }
}

/**
 * https://stackoverflow.com/questions/4681325/join-or-implode-in-c
 * Thanks to bdonlan.
 */
static char *util_cat(char *dest, char *end, const char *str)
{
  while (dest < end && *str)
    *dest++ = *str++;
  return dest;
}

size_t join_str(char *out_string, size_t out_bufsz, const char *delim, char **chararr)
{
  char *ptr = out_string;
  char *strend = out_string + out_bufsz;
  while (ptr < strend && *chararr)
  {
    ptr = util_cat(ptr, strend, *chararr);
    printf("ptr %s\n", ptr);
    printf("chararr %s\n", *chararr);
    chararr++;
    if (*chararr)
    {
      ptr = util_cat(ptr, strend, delim);
    }
  }
  return ptr - out_string;
}
