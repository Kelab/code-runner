#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "argv.h"
#include "constants.h"
#include "diff.h"
#include "log.h"
#include "run.h"
#include "utils.h"

char result_message[1000];

void init_result()
{
  runner_result.status = PENDING;
  runner_result.cpu_time_used = runner_result.cpu_time_used_us = 0;
  runner_result.real_time_used = runner_result.real_time_used_us = 0;
  runner_result.memory_used = 0;
  runner_result.signal_code = runner_result.exit_code = 0;
  runner_result.error_code = 0;
}

void init_config()
{
  runner_config.memory_check_only = 0;
  runner_config.cpu_time_limit = runner_config.real_time_limit = runner_config.memory_limit = 0;
  runner_config.std_in = runner_config.std_out = runner_config.std_err = 0;
  runner_config.in_file = runner_config.out_file = '\0';
  runner_config.save_file = '\0';
  runner_config.stdout_file = '\0';
  runner_config.stderr_file = '\0';
  runner_config.log_file = "runner.log";
}

void log_config()
{
  int i = 0;
  while ((runner_config.cmd)[i])
  {
    log_debug("config: cmd part %d: %s", i, (runner_config.cmd)[i]);
    i++;
  };
  log_debug("config: cpu_time_limit %d ms", runner_config.cpu_time_limit);
  log_debug("config: real_time_limit %d ms", runner_config.real_time_limit);
  log_debug("config: memory_limit %d kb", runner_config.memory_limit);
  log_debug("config: memory_check_only %d", runner_config.memory_check_only);
  log_debug("config: attach: STDIN %d | STDOUT %d | STDERR %d", runner_config.std_in, runner_config.std_out, runner_config.std_err);
  log_debug("config: in_file %s", runner_config.in_file);
  log_debug("config: out_file %s", runner_config.out_file);
  log_debug("config: stdout_file %s", runner_config.stdout_file);
  log_debug("config: log_file %s", runner_config.log_file);
}

void show_result()
{
  format_result(result_message);
  if (runner_config.save_file)
  {
    log_debug("save result into file %s", runner_config.save_file);
    write_file(runner_config.save_file, result_message);
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
  init_config();
  init_result();
  parse_argv(argc, argv);

  FILE *log_fp = NULL;
  if (runner_config.log_file)
  {
    log_fp = fopen(runner_config.log_file, "a");
    if (log_fp != NULL)
    {
      log_add_fp(log_fp, LOG_DEBUG);
    }
  }

  log_config();

  run();

  // 子程序运行失败的话，直接输出结果。不需要进行后面的 diff 了
  if (runner_result.exit_code || runner_result.signal_code)
  {
    show_result();
    CLOSE_FP(log_fp);
    return 0;
  }
  if (runner_result.status <= ACCEPTED)
  {
    diff();
  }
  show_result();
  CLOSE_FP(log_fp);
  return 0;
}
