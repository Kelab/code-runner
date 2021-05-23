#define _GNU_SOURCE

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <sys/utsname.h>
#include <sched.h>
#include <stdint.h>
#include <sys/mman.h>

#include "child.h"
#include "constants.h"
#include "run.h"
#include "utils.h"

struct rlimit _rl;
#define SET_LIMIT(FIELD, VALUE)                  \
  log_debug("set %s : %ld", #FIELD, VALUE);      \
  _rl.rlim_cur = _rl.rlim_max = (rlim_t)(VALUE); \
  if (setrlimit(FIELD, &_rl))                    \
    CHILD_ERROR_EXIT("set" #FIELD "failure");

/**
 * run the specific process
 */
void child_process()
{
  int input_fd = -1;
  int output_fd = -1;
  int err_fd = -1;
  int null_fd = open("/dev/null", O_RDWR);

  if (runner_config.cpu_time_limit != RESOURCE_UNLIMITED)
  {
    // CPU time limit in seconds.
    SET_LIMIT(RLIMIT_CPU, (runner_config.cpu_time_limit + 1000) / 1000);
  }

  // 注意，设置 memory_limit 会导致有些程序 crash，比如 python, node
  if (runner_config.memory_limit != RESOURCE_UNLIMITED)
  {
    if (runner_config.memory_check_only == 0)
    {
      // The maximum size of the process's virtual memory (address space) in bytes.
      // 为了避免代码是正确的，但是因为超过内存 oom 而被判定为 re。
      // 如果程序占用低于两倍，最后再重新检查内存占用和配置的关系，就可以判定为超内存而不是 re，如果超过两倍，那就真的 re 了（可能会被 kill）。
      SET_LIMIT(RLIMIT_AS, runner_config.memory_limit * 1024 * 2);
    }
  }

  // 其他安全项
  // 设置同时能打开的最大文件描述符数为
  SET_LIMIT(RLIMIT_NOFILE, LIMITS_MAX_FD);
  // 最大输出
  SET_LIMIT(RLIMIT_FSIZE, LIMITS_MAX_OUTPUT);
  // 重定向 标准输出IO 到相应的文件中
  if (runner_config.in_file)
  {
    input_fd = open(runner_config.in_file, O_RDONLY | O_CREAT, 0700);
    if (input_fd != -1)
    {
      log_debug("open in_file");
      if (dup2(input_fd, STDIN_FILENO) == -1)
      {
        CHILD_ERROR_EXIT("input_fd dup error");
      }
    }
    else
    {
      CHILD_ERROR_EXIT("error open in_file");
    }
  }
  else
  {
    log_info("in_file is not set");
    if (runner_config.std_in == 0)
    {
      log_info("redirected stdin to /dev/null");
      dup2(null_fd, STDIN_FILENO);
    }
    else
    {
      log_info("use stdin");
    }
  }

  if (runner_config.stdout_file)
  {
    output_fd = open(runner_config.stdout_file, O_WRONLY | O_CREAT | O_TRUNC, 0700);
    if (output_fd != -1)
    {
      log_debug("open stdout_file");
      if (dup2(output_fd, STDOUT_FILENO) == -1)
      {
        CHILD_ERROR_EXIT("output_fd dup error");
      }
    }
    else
    {
      CHILD_ERROR_EXIT("error open stdout_file");
    }
  }
  else
  {
    log_info("stdout_file is not set");
    if (runner_config.std_out == 0)
    {
      log_info("redirected stdout to /dev/null");
      dup2(null_fd, STDOUT_FILENO);
    }
    else
    {
      log_info("use stdout");
    }
  }

  if (runner_config.stderr_file)
  {
    err_fd = open(runner_config.stderr_file, O_WRONLY | O_CREAT | O_TRUNC, 0700);
    if (err_fd != -1)
    {

      if (dup2(err_fd, STDERR_FILENO) == -1)
      {
        CHILD_ERROR_EXIT("err_fd");
      }
    }
    else
    {
      CHILD_ERROR_EXIT("error open err_fd");
    }
  }
  else
  {
    log_info("err_file is not set");
    if (runner_config.std_err == 0)
    {
      log_info("redirected stderr to /dev/null");
      dup2(null_fd, STDERR_FILENO);
    }
    else
    {
      log_info("use stderr");
    }
  }

  log_debug("exec %s", runner_config.cmd[0]);
  execvp(runner_config.cmd[0], runner_config.cmd);
  CHILD_ERROR_EXIT("exec cmd error");
}

int sandbox_proxy(void *_arg)
{
  struct utsname uts;
  char *hostname = "runner";
  /* Change hostname in UTS namespace of child. */
  if (sethostname(hostname, strlen(hostname)) == -1)
    INTERNAL_ERROR_EXIT("sethostname");

  /* Retrieve and display hostname. */
  if (uname(&uts) == -1)
    INTERNAL_ERROR_EXIT("uname");

  pid_t inside_pid = fork();

  if (inside_pid < 0)
  {
    INTERNAL_ERROR_EXIT("Cannot run process, fork failed");
  }
  else if (!inside_pid)
  {
    child_process();
    _exit(42); // We should never get here
  }
  int stat;
  pid_t p = waitpid(inside_pid, &stat, 0);

  if (p < 0)
    INTERNAL_ERROR_EXIT("Proxy waitpid() failed");
  return 0;
}
