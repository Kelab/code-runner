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

void init_result(struct Result *result)
{
  result->status = PENDING;
  result->cpu_time_used = result->cpu_time_used_us = 0;
  result->real_time_used = result->real_time_used_us = 0;
  result->memory_used = 0;
  result->signal_code = result->exit_code = 0;
  result->error_code = 0;
}

void init_config(struct Config *config)
{
  config->memory_check_only = 0;
  config->cpu_time_limit = config->real_time_limit = config->memory_limit = 0;
  config->std_in = config->std_out = config->std_err = 0;
  config->save_file = '\0';
  config->in_file = config->out_file = '\0';
  config->log_file = config->stdout_file = config->stderr_file = '\0';
}

void log_config(struct Config *config)
{
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
  log_debug("config: stdout_file %s", config->stdout_file);
  log_debug("config: log_file %s", config->log_file);
}

void show_result(struct Result *result, struct Config *config)
{
  format_result(result_message, result);
  if (config->save_file)
  {
    log_debug("save result into file %s", config->save_file);
    write_file(config->save_file, result_message);
  }
  else
  {
    log_debug("print result_message");
    printf("%s\n", result_message);
  }
  log_info(result_message);
}

int main(int argc, char *argv[])
{
  log_set_quiet(true);

  struct Config config;
  struct Result result;

  init_config(&config);
  init_result(&result);

  parse_argv(argc, argv, &config);

  FILE *log_fp = NULL;
  if (config.log_file)
  {
    log_fp = fopen(config.log_file, "a");
    if (log_fp != NULL)
    {
      log_add_fp(log_fp, LOG_DEBUG);
    }
  }
  log_config(&config);

  run(&config, &result);

  // 子程序运行失败的话，直接输出结果。
  if (result.exit_code || result.signal_code)
  {
    show_result(&result, &config);
    CLOSE_FP(log_fp);
    exit(EXIT_FAILURE);
  }
  if (result.status <= ACCEPTED)
    diff(&config, &result);
  show_result(&result, &config);
  CLOSE_FP(log_fp);
  return 0;
}
