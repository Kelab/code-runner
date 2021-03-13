#define _POSIX_SOURCE
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
#include <pthread.h>
#include <signal.h>

#include "constants.h"
#include "run.h"
#include "utils.h"

/**
 * run the specific process
 */
void child_process(struct Config *config)
{
  int input_fd = -1;
  int output_fd = -1;
  int err_fd = -1;

  FILE *re_out = NULL;
  FILE *re_err = NULL;

  if (config->cpu_time_limit != RESOURCE_UNLIMITED)
  {
    // CPU time limit in seconds.
    log_debug("set cpu_time_limit");
    struct rlimit max_time_rl;
    max_time_rl.rlim_cur = max_time_rl.rlim_max = (rlim_t)((config->cpu_time_limit + 1000) / 1000);
    if (setrlimit(RLIMIT_CPU, &max_time_rl))
      CHILD_ERROR_EXIT("set RLIMIT_CPU failure");
  }

  // 注意，设置 memory_limit 会导致有些程序 crash，比如 python, node
  if (config->memory_limit != RESOURCE_UNLIMITED)
  {
    if (config->memory_check_only == 0)
    {
      // The maximum size of the process's virtual memory (address space) in bytes.
      log_debug("set memory_limit");

      struct rlimit max_memory_rl;
      // 为了避免代码是正确的，但是因为超过内存 oom 而被判定为 re。
      // 如果程序占用低于两倍，最后再重新检查内存占用和配置的关系，就可以判定为超内存而不是 re，如果超过两倍，那就真的 re 了（可能会被 kill）。
      max_memory_rl.rlim_max = max_memory_rl.rlim_cur = config->memory_limit * 1024 * 2;
      if (setrlimit(RLIMIT_AS, &max_memory_rl))
        CHILD_ERROR_EXIT("set RLIMIT_AS failure");
    }
  }

  // log_debug("set max_output_size");
  // // set max output size limit
  // struct rlimit max_output_size;
  // max_output_size.rlim_cur = max_output_size.rlim_max = MAX_OUTPUT;
  // if (setrlimit(RLIMIT_FSIZE, &max_output_size) != 0)
  // {
  //   CHILD_ERROR_EXIT("set RLIMIT_FSIZE failure");
  // }

  // 重定向 标准输出IO 到相应的文件中
  input_fd = open(config->in_file, O_RDONLY | O_CREAT, 0700);
  if (input_fd != -1)
  {
    log_debug("open in_file");
    if (dup2(input_fd, fileno(stdin)) == -1)
    {
      CHILD_ERROR_EXIT("input_fd dup error");
    }
  }
  else
  {
    log_error("error open in_file");
  }
  output_fd = open(config->user_out_file, O_WRONLY | O_CREAT | O_TRUNC, 0700);
  if (output_fd != -1)
  {
    log_debug("open user_out_file");
    if (dup2(output_fd, fileno(stdout)) == -1)
    {
      CHILD_ERROR_EXIT("output_fd dup error");
    }
  }
  else
  {
    log_error("error open user_out_file, redirect to /dev/null");
    re_out = freopen("/dev/null", "r", stdout);
  }
  err_fd = output_fd;
  if (err_fd != -1)
  {
    if (dup2(err_fd, fileno(stderr)) == -1)
    {
      CHILD_ERROR_EXIT("err_fd");
    }
  }
  else
  {
    log_error("error open err_fd, redirect to /dev/null");
    re_err = freopen("/dev/null", "r", stderr);
  }

  log_debug("exec %s", config->cmd[0]);
  char *envp[] = {NULL};

  execvpe(config->cmd[0], config->cmd, envp);
  CHILD_ERROR_EXIT("exec cmd error");
}

void log_rusage(struct rusage *ru)
{
  log_debug("rusage: user time used tv_sec %ld s", ru->ru_utime.tv_sec);
  log_debug("rusage: user time used tv_usec %ld us", ru->ru_utime.tv_usec);
  log_debug("rusage: system time tv_sec %ld s", ru->ru_stime.tv_sec);
  log_debug("rusage: system time tv_usec %ld us", ru->ru_stime.tv_usec);
  log_debug("rusage: maximum resident set size %ld kb", ru->ru_maxrss);
  log_debug("rusage: page reclaims %ld", ru->ru_minflt);
  log_debug("rusage: page faults %ld", ru->ru_majflt);
  log_debug("rusage: block input operations %ld", ru->ru_inblock);
  log_debug("rusage: block output operations %ld", ru->ru_oublock);
  log_debug("rusage: voluntary context switches %ld", ru->ru_nvcsw);
  log_debug("rusage: involuntary context switches %ld", ru->ru_nivcsw);
}

struct killer_parameter
{
  pid_t pid;
  int timeout;
};

int kill_pid(pid_t pid)
{
  return kill(pid, SIGKILL);
}

void *timeout_killer(void *killer_para)
{
  // this is a new thread, kill the process if timeout
  pid_t pid = ((struct killer_parameter *)killer_para)->pid;
  int timeout = ((struct killer_parameter *)killer_para)->timeout;
  // On success, pthread_detach() returns 0; on error, it returns an error number.
  if (pthread_detach(pthread_self()) != 0)
  {
    kill_pid(pid);
    return NULL;
  }
  // this may sleep longer that expected, but we will have a check at the end
  if (sleep((unsigned int)((timeout + 500) / 1000)) != 0)
  {
    log_debug("timeout");
    kill_pid(pid);
    return NULL;
  }
  if (kill_pid(pid) != 0)
  {
    return NULL;
  }
  return NULL;
}

/**
 * monitor the user process
 */
void monitor(pid_t child_pid, struct Config *config, struct Result *result, struct timeval *start_time, struct timeval *end_time)
{
  // create new thread to monitor process running time
  pthread_t tid = 0;
  if (config->real_time_limit != RESOURCE_UNLIMITED)
  {
    struct killer_parameter para;

    para.timeout = config->real_time_limit;
    para.pid = child_pid;
    if (pthread_create(&tid, NULL, timeout_killer, (void *)(&para)) != 0)
    {
      kill_pid(child_pid);
      INTERNAL_ERROR_EXIT("pthread create error");
    }
  }
  // 获取子进程的退出状态
  int status;
  struct rusage ru;
  if (wait4(child_pid, &status, 0, &ru) == -1)
  {
    INTERNAL_ERROR_EXIT("wait4 error");
  }
  gettimeofday(end_time, NULL);
  log_rusage(&ru);
  const struct timeval real_time_tv = {
      (end_time->tv_sec - start_time->tv_sec),
      (end_time->tv_usec - start_time->tv_usec),
  };
  result->real_time_used = tv_to_ms(&real_time_tv);
  result->real_time_used_us = tv_to_us(&real_time_tv);
  const struct timeval cpu_time_tv = {
      ((&ru.ru_utime)->tv_sec + (&ru.ru_stime)->tv_sec),
      ((&ru.ru_utime)->tv_usec + (&ru.ru_stime)->tv_usec),
  };
  result->cpu_time_used = tv_to_ms(&cpu_time_tv);
  result->cpu_time_used_us = tv_to_us(&cpu_time_tv);
  // 在 linux, ru_maxrss 单位是 kb
  result->memory_used = ru.ru_maxrss;
  // process exited, we may need to cancel timeout killer thread
  if (config->real_time_limit != RESOURCE_UNLIMITED)
  {
    if (pthread_cancel(tid) != 0)
    {
      // todo logging
    };
  }
  // 若此值为非0 表明进程被信号终止，说明子进程非正常结束
  if (WIFSIGNALED(status) != 0)
  {
    // TODO: fix SIGUSR1
    // 此时可通过 WTERMSIG(status) 获取使得进程退出的信号编号
    result->signal = WTERMSIG(status);
    log_debug("child process exit abnormal, signal: %d", result->signal);
    log_info("strsignal: %s", strsignal(result->signal));
    switch (result->signal)
    {
    case SIGUSR1:
      result->status = SYSTEM_ERROR;
      break;
    case SIGSEGV:
      if (result->memory_used > config->memory_limit)
        result->status = MEMORY_LIMIT_EXCEEDED;
      else
        result->status = RUNTIME_ERROR;
      break;
    case SIGALRM:
    case SIGXCPU:
      result->status = TIME_LIMIT_EXCEEDED;
      break;
    case SIGKILL:
    default:
      if (config->cpu_time_limit != RESOURCE_UNLIMITED && result->cpu_time_used > config->cpu_time_limit)
        result->status = TIME_LIMIT_EXCEEDED;
      else if (config->real_time_limit != RESOURCE_UNLIMITED && result->real_time_used > config->real_time_limit)
        result->status = TIME_LIMIT_EXCEEDED;
      else if (config->memory_limit != RESOURCE_UNLIMITED && result->memory_used > config->memory_limit)
        result->status = MEMORY_LIMIT_EXCEEDED;
      else
        result->status = RUNTIME_ERROR;
      break;
    }
  }
  else
  {
    // 程序正常结束，此时可通过WEXITSTATUS(status)获取进程退出状态(exit时参数)
    result->exit_code = WEXITSTATUS(status);
    log_debug("child process exit_code %d", result->exit_code);

    if (result->exit_code != 0)
    {
      result->status = RUNTIME_ERROR;
    }
    else
    {
      if (config->cpu_time_limit != RESOURCE_UNLIMITED && result->cpu_time_used > config->cpu_time_limit)
        result->status = TIME_LIMIT_EXCEEDED;
      else if (config->real_time_limit != RESOURCE_UNLIMITED && result->real_time_used > config->real_time_limit)
        result->status = TIME_LIMIT_EXCEEDED;
      else if (config->memory_limit != RESOURCE_UNLIMITED && result->memory_used > config->memory_limit)
        result->status = MEMORY_LIMIT_EXCEEDED;
    }
  }
}

int run(struct Config *config, struct Result *result)
{
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
    INTERNAL_ERROR_EXIT("error in fork");
  }
  else if (child_pid == 0)
  {
    child_process(config);
  }
  else
  {
    monitor(child_pid, config, result, &start_time, &end_time);
  }
  return 0;
}
