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
  config->judge_mode = 1;
  config->run_mode = config->check_mode = 0;
  config->memory_check_only = 0;
  config->cpu_time_limit = config->real_time_limit = config->memory_limit = 0;
  config->log_file = config->in_file = config->out_file = config->user_out_file = '\0';
}

FILE *set_logger(struct Config *config)
{
  FILE *log_fp = NULL;
  log_set_quiet(true);
  if (config->log_file)
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
  log_debug("config: run_mode %d", config->run_mode);
  log_debug("config: check_mode %d", config->check_mode);
  log_debug("config: judge_mode %d", config->judge_mode);
  int i = 0;
  while ((config->cmd)[i])
  {
    log_debug("config: cmd part %d: %s", i, (config->cmd)[i]);
    i++;
  };
  log_debug("config: cpu_time_limit %d ms", config->cpu_time_limit);
  log_debug("config: real_time_limit %d ms", config->real_time_limit);
  log_debug("config: memory_limit %d kb", config->memory_limit);
  log_debug("config: memory_check_only %d", config->memory_check_only);
  log_debug("config: in_file %s", config->in_file);
  log_debug("config: out_file %s", config->out_file);
  log_debug("config: user_out_file %s", config->user_out_file);
  log_debug("config: log_file %s", config->log_file);
}

void print_result(struct Result *_result)
{
  format_result(result_message, _result);
  printf("%s\n", result_message);
  log_info(result_message);
}

#define CLOSE_LOGGER_FILE() \
  {                         \
    if (log_fp != NULL)     \
    {                       \
      fclose(log_fp);       \
    }                       \
  }

int main(int argc, char *argv[])
{
  struct Config config;
  struct Result result;

  init_config(&config);
  parse_argv(argc, argv, &config);
  FILE *log_fp = set_logger(&config);
  log_config(&config);
  init_result(&result);

  if (config.check_mode == 1)
  {
    int status = -1;
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
      CLOSE_LOGGER_FILE();
      exit(EXIT_FAILURE);
    }
    if (result.status == PENDING || result.status == ACCEPTED)
    {
      if (config.judge_mode == 1)
      {
        diff(&config, &result.status);
      }
    }
    print_result(&result);
  }
  CLOSE_LOGGER_FILE();
  return 0;
}
