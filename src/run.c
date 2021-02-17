
#define _GNU_SOURCE

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <errno.h>

#include "constants.h"
#include "run.h"
#include "utils.h"

/**
 * run the specific process
 */
void child_process(struct Config *_config)
{
  int input_fd = -1;
  int output_fd = -1;
  int err_fd = -1;

  if (_config->time_limit != RESOURCE_UNLIMITED)
  {
    // CPU time limit in seconds.
    log_debug("set time_limit");
    struct rlimit max_time_rl;
    max_time_rl.rlim_cur = max_time_rl.rlim_max = (rlim_t)((_config->time_limit + 1000) / 1000);
    if (setrlimit(RLIMIT_CPU, &max_time_rl))
      CHILD_ERROR_EXIT("set RLIMIT_CPU failure");
  }

  // 注意，设置 memory_limit 会导致有些程序 crash，比如 python, node
  if (_config->memory_limit != RESOURCE_UNLIMITED)
  {
    // The maximum size of the process's virtual memory (address space) in bytes.
    log_debug("set memory_limit");

    struct rlimit max_memory_rl;
    // 为了避免代码是正确的，但是因为超过内存 oom 而被判定为 re。
    // 如果程序占用低于两倍，最后再重新检查内存占用和配置的关系，就可以判定为超内存而不是 re，如果超过两倍，那就真的 re 了（可能会被 kill）。
    max_memory_rl.rlim_max = max_memory_rl.rlim_cur = _config->memory_limit * 1024 * 2;
    if (setrlimit(RLIMIT_AS, &max_memory_rl))
      CHILD_ERROR_EXIT("set RLIMIT_AS failure");
  }

  log_debug("set max_output_size");
  // set max output size limit
  struct rlimit max_output_size;
  max_output_size.rlim_cur = max_output_size.rlim_max = MAX_OUTPUT;
  if (setrlimit(RLIMIT_FSIZE, &max_output_size) != 0)
  {
    CHILD_ERROR_EXIT("set RLIMIT_FSIZE failure");
  }

  // 重定向 标准输出IO 到相应的文件中
  input_fd = open(_config->in_file, O_RDONLY | O_CREAT, 0700);
  if (input_fd != -1)
  {
    log_debug("open in_file");
    if (dup2(input_fd, fileno(stdin)) == -1)
    {
      CHILD_ERROR_EXIT("input_fd");
    }
  }
  output_fd = open(_config->user_out_file, O_WRONLY | O_CREAT | O_TRUNC, 0700);
  if (output_fd != -1)
  {
    log_debug("open user_out_file");
    if (dup2(output_fd, fileno(stdout)) == -1)
    {
      CHILD_ERROR_EXIT("output_fd");
    }
  }
  // err_fd = output_fd;
  // if (err_fd != -1)
  // {
  //   if (dup2(err_fd, fileno(stderr)) == -1)
  //   {
  //     CHILD_ERROR_EXIT("err_fd");
  //   }
  // }

  log_debug("exec %s", _config->cmd[0]);
  char *envp[] = {NULL};

  execve(_config->cmd[0], _config->cmd, envp);
  CHILD_ERROR_EXIT("exec cmd error");
}

void log_rusage(struct rusage *ru)
{
  log_debug("rusage: user time used us %ld", ru->ru_utime.tv_usec);
  log_debug("rusage: user time used s %ld", ru->ru_utime.tv_sec);
  log_debug("rusage: system time used us %ld", ru->ru_stime.tv_usec);
  log_debug("rusage: system time used s %ld", ru->ru_stime.tv_sec);
  log_debug("rusage: maximum resident set size %ld", ru->ru_maxrss);
  log_debug("rusage: page reclaims %ld", ru->ru_minflt);
  log_debug("rusage: page faults %ld", ru->ru_majflt);
  log_debug("rusage: block input operations %ld", ru->ru_inblock);
  log_debug("rusage: block output operations %ld", ru->ru_oublock);
  log_debug("rusage: voluntary context switches %ld", ru->ru_nvcsw);
  log_debug("rusage: involuntary context switches %ld", ru->ru_nivcsw);
}

/**
 * monitor the user process
 */
void monitor(pid_t child_pid, struct Config *_config, struct Result *_result, struct timeval *start_time, struct timeval *end_time)
{
  // 获取子进程的退出状态
  int status;
  struct rusage ru;
  if (wait4(child_pid, &status, 0, &ru) == -1)
  {
    LOG_INTERNAL_ERROR("wait4 error");
    exit(EXIT_FAILURE);
  }
  gettimeofday(end_time, NULL);
  log_rusage(&ru);
  _result->real_time_used = ((end_time->tv_sec - start_time->tv_sec) * 1000 + (end_time->tv_usec - start_time->tv_usec) / 1000);
  _result->real_time_used_us = (end_time->tv_sec * 1000 * 1000 + end_time->tv_usec - start_time->tv_sec * 1000 * 1000 - start_time->tv_usec);
  _result->cpu_time_used = ru.ru_utime.tv_sec * 1000 + ru.ru_utime.tv_usec / 1000;
  _result->cpu_time_used_us = ru.ru_utime.tv_sec * 1000 * 1000 + ru.ru_utime.tv_usec;
  // 在 linux, ru_maxrss 单位是 kb
  _result->memory_used = ru.ru_maxrss;

  // 若此值为非0 表明进程被信号终止，说明子进程非正常结束
  if (WIFSIGNALED(status) != 0)
  {
    // TODO: fix SIGUSR1
    // 此时可通过 WTERMSIG(status) 获取使得进程退出的信号编号
    _result->signal = WTERMSIG(status);
    log_debug("child process exit abnormal");
    log_info("strsignal: %s", strsignal(_result->signal));
    switch (_result->signal)
    {
    case SIGUSR1:
      _result->status = SYSTEM_ERROR;
      break;
    case SIGSEGV:
      if (_result->memory_used > _config->memory_limit)
        _result->status = MEMORY_LIMIT_EXCEEDED;
      else
        _result->status = RUNTIME_ERROR;
      break;
    case SIGALRM:
    case SIGXCPU:
      _result->status = TIME_LIMIT_EXCEEDED;
      break;
    default:
      // 可能是超过资源限制之后的被 SIGKILL 杀掉
      if (_result->memory_used > _config->memory_limit)
        _result->status = MEMORY_LIMIT_EXCEEDED;
      else if (_result->cpu_time_used > _config->time_limit)
        _result->status = TIME_LIMIT_EXCEEDED;
      else
        _result->status = RUNTIME_ERROR;
      break;
    }
  }
  else
  {
    log_debug("child process exit normal");
    // 程序正常结束，此时可通过WEXITSTATUS(status)获取进程退出状态(exit时参数)
    _result->exit_code = WEXITSTATUS(status);

    if (_result->exit_code != 0)
    {
      _result->status = RUNTIME_ERROR;
    }
    else
    {
      if (_result->cpu_time_used > _config->time_limit)
        _result->status = TIME_LIMIT_EXCEEDED;
      else if (_result->memory_used > _config->memory_limit)
        _result->status = MEMORY_LIMIT_EXCEEDED;
    }
  }
}

int run(struct Config *_config, struct Result *_result)
{
  // record real running time: end_time - start_time
  struct timeval start_time, end_time;

  gettimeofday(&start_time, NULL);
  pid_t child_pid = fork();
  if (child_pid < 0)
  {
    // The fork() function shall fail if:
    //   The system lacked the necessary resources to create another process,
    //   or the system - imposed limit on the total number of processes under execution system - wide or by a single user{CHILD_MAX} would be exceeded.
    // The fork() function may fail if:
    //   Insufficient storage space is available.
    LOG_INTERNAL_ERROR("error in fork");
    exit(EXIT_FAILURE);
  }
  else if (child_pid == 0)
  {
    child_process(_config);
  }
  else
  {
    monitor(child_pid, _config, _result, &start_time, &end_time);
  }
  return 0;
}
