#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "constants.h"
#include "diff.h"
#include "log.h"
#include "run.h"
#include "utils.h"

char message[1000];

void init_result(struct result *_result)
{
  _result->status = AC;
  _result->cpu_time_used = _result->cpu_time_used_us = 0;
  _result->memory_used = _result->memory_used_b = 0;
  _result->signal = _result->exit_code = 0;
}

void print_result(struct result *_result)
{
  sprintf(message, "{\n"
                   "  \"status\": %d,\n"
                   "  \"cpu_time_used\": %d,\n"
                   "  \"cpu_time_used_us\": %ld,\n"
                   "  \"memory_used\": %d,\n"
                   "  \"memory_used_b\": %ld,\n"
                   "  \"signal\": %d,\n"
                   "  \"exit_code\": %d\n"
                   "}",
          _result->status,
          _result->cpu_time_used,
          _result->cpu_time_used_us,
          _result->memory_used,
          _result->memory_used_b,
          _result->signal,
          _result->exit_code);
  printf("%s\n", message);
}

int main(int argc, char *argv[])
{

  char *cmd[20];
  split(cmd, argv[1], "@");
  int time_limit = atoi(argv[2]);
  int memory_limit = atoi(argv[3]);
  char *in_file = argv[4];
  char *out_file = argv[5];
  char *user_out_file = argv[6];
  char *log_file = argv[7];
  FILE *log_fp = fopen(log_file, "a");
  if (log_fp == NULL)
  {
    fprintf(stderr, "can not open log file");
  }
  log_set_quiet(true);
  log_add_fp(log_fp, 0);

  struct result result;
  init_result(&result);
  run(cmd, time_limit, memory_limit, in_file, user_out_file, &result);
  print_result(&result);
  if (result.exit_code || result.signal)
  {
    exit(EXIT_FAILURE);
  }

  int right_fd = open(out_file, O_RDWR | O_CREAT, 0644);
  int userout_fd = open(user_out_file, O_RDWR | O_CREAT, 0644);
  check_diff(right_fd, userout_fd, &result.status);
  print_result(&result);
  return 0;
}
