#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "cli.h"
#include "constants.h"
#include "diff.h"
#include "log.h"
#include "run.h"
#include "utils.h"

char result_message[1000];

void init_result(struct Result *_result)
{
  _result->status = PENDING;
  _result->cpu_time_used = _result->cpu_time_used_us = 0;
  _result->real_time_used = _result->real_time_used_us = 0;
  _result->memory_used = 0;
  _result->signal = _result->exit_code = 0;
}

void init_config(struct Config *config)
{
  config->run_only = config->check_only = 0;
  *config->cmd = '\0';
  config->time_limit = config->memory_limit = 0;
  config->log_file = config->in_file = config->out_file = config->user_out_file = '\0';
}

FILE *set_logger(struct Config *config)
{
  FILE *log_fp = NULL;
  log_set_quiet(true);
  if (config->log_file != '\0')
  {
    log_fp = fopen(config->log_file, "a");
    if (log_fp == NULL)
    {
      fprintf(stderr, "can not open log file\n");
      fprintf(stderr, "log_file: %s\n", config->log_file);
    }
    else
    {
      log_add_fp(log_fp, 0);
    }
  }
  return log_fp;
}

void log_config(struct Config *config)
{
  log_debug("run_only %d", config->run_only);
  log_debug("check_only %d", config->check_only);
  log_debug("cmd %s", *config->cmd);
  log_debug("time_limit %d", config->time_limit);
  log_debug("memory_limit %d", config->memory_limit);
  log_debug("in_file %s", config->in_file);
  log_debug("out_file %s", config->out_file);
  log_debug("user_out_file %s", config->user_out_file);
  log_debug("log_file %s", config->log_file);
}

void print_result(struct Result *_result)
{
  format_result(result_message, _result);
  printf("%s\n", result_message);
  log_info(result_message);
}

int main(int argc, char *argv[])
{
  struct Config config;
  init_config(&config);
  parse_argv(argc, argv, &config);
  FILE *log_fp = set_logger(&config);
  log_config(&config);
  struct Result result;
  init_result(&result);

  if (config.check_only == 1)
  {
    int status = 0;
    diff(&config, &status);
    printf("%d\n", status);
  }
  else
  {
    run(&config, &result);
    // 运行失败的话，直接输出结果。
    if (result.exit_code || result.signal)
    {
      print_result(&result);
      exit(EXIT_FAILURE);
    }

    if (config.run_only == 1)
    {
      print_result(&result);
      exit(EXIT_SUCCESS);
    }

    diff(&config, &result.status);
    print_result(&result);
  }
  fclose(log_fp);
  return 0;
}
