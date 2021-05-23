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

void log_rusage(struct rusage *ru)
{
  log_debug("rusage: user time used tv_sec %ld s", ru->ru_utime.tv_sec);
  log_debug("rusage: user time used tv_usec %ld us", ru->ru_utime.tv_usec);
  log_debug("rusage: system time tv_sec %ld s", ru->ru_stime.tv_sec);
  log_debug("rusage: system time tv_usec %ld us", ru->ru_stime.tv_usec);
  // log_debug("rusage: maximum resident set size %ld kb", ru->ru_maxrss);
  // log_debug("rusage: page reclaims %ld", ru->ru_minflt);
  // log_debug("rusage: page faults %ld", ru->ru_majflt);
  // log_debug("rusage: block input operations %ld", ru->ru_inblock);
  // log_debug("rusage: block output operations %ld", ru->ru_oublock);
  // log_debug("rusage: voluntary context switches %ld", ru->ru_nvcsw);
  // log_debug("rusage: involuntary context switches %ld", ru->ru_nivcsw);
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
void monitor(pid_t child_pid, struct timeval *start_time, struct timeval *end_time)
{
  // create new thread to monitor process running time
  pthread_t tid = 0;
  if (runner_config.real_time_limit != RESOURCE_UNLIMITED)
  {
    struct killer_parameter para;

    para.timeout = runner_config.real_time_limit;
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
  runner_result.real_time_used = tv_to_ms(&real_time_tv);
  runner_result.real_time_used_us = tv_to_us(&real_time_tv);
  const struct timeval cpu_time_tv = {
      ((&ru.ru_utime)->tv_sec + (&ru.ru_stime)->tv_sec),
      ((&ru.ru_utime)->tv_usec + (&ru.ru_stime)->tv_usec),
  };
  runner_result.cpu_time_used = tv_to_ms(&cpu_time_tv);
  runner_result.cpu_time_used_us = tv_to_us(&cpu_time_tv);
  // 在 linux, ru_maxrss 单位是 kb
  runner_result.memory_used = ru.ru_maxrss;
  // process exited, we may need to cancel timeout killer thread
  if (runner_config.real_time_limit != RESOURCE_UNLIMITED)
  {
    if (pthread_cancel(tid) != 0)
    {
      // todo logging
    };
  }
  if (WIFSIGNALED(status))
  {
    // TODO: fix SIGUSR1
    // the child process was terminated by a signal.
    // 此时可通过 WTERMSIG(status) 获取信号编号
    runner_result.signal_code = WTERMSIG(status);
    log_debug("child process exit abnormal, signal: %d, strsignal: %s", runner_result.signal_code, strsignal(runner_result.signal_code));
    switch (runner_result.signal_code)
    {
    case SIGUSR1:
      runner_result.status = SYSTEM_ERROR;
      break;
    case SIGUSR2:
      runner_result.status = SYSTEM_ERROR;
      runner_result.error_code = COMMAND_NOT_FOUND;
      break;
    case SIGSEGV:
      if (runner_result.memory_used > runner_config.memory_limit)
        runner_result.status = MEMORY_LIMIT_EXCEEDED;
      else
        runner_result.status = RUNTIME_ERROR;
      break;
    case SIGALRM:
    case SIGXCPU:
      runner_result.status = TIME_LIMIT_EXCEEDED;
      break;
    case SIGKILL:
    default:
      if (runner_config.cpu_time_limit != RESOURCE_UNLIMITED && runner_result.cpu_time_used > runner_config.cpu_time_limit)
        runner_result.status = TIME_LIMIT_EXCEEDED;
      else if (runner_config.real_time_limit != RESOURCE_UNLIMITED && runner_result.real_time_used > runner_config.real_time_limit)
        runner_result.status = TIME_LIMIT_EXCEEDED;
      else if (runner_config.memory_limit != RESOURCE_UNLIMITED && runner_result.memory_used > runner_config.memory_limit)
        runner_result.status = MEMORY_LIMIT_EXCEEDED;
      else
        runner_result.status = RUNTIME_ERROR;
      break;
    }
  }
  else
  {
    // 程序正常结束(通过调用 return 或者 exit 结束)
    // 此时可通过WEXITSTATUS(status)获取进程退出状态(exit时参数)
    runner_result.exit_code = WEXITSTATUS(status);
    log_debug("child process exit_code %d", runner_result.exit_code);

    if (runner_result.exit_code != 0)
    {
      runner_result.status = RUNTIME_ERROR;
    }
    else
    {
      if (runner_config.cpu_time_limit != RESOURCE_UNLIMITED && runner_result.cpu_time_used > runner_config.cpu_time_limit)
        runner_result.status = TIME_LIMIT_EXCEEDED;
      else if (runner_config.real_time_limit != RESOURCE_UNLIMITED && runner_result.real_time_used > runner_config.real_time_limit)
        runner_result.status = TIME_LIMIT_EXCEEDED;
      else if (runner_config.memory_limit != RESOURCE_UNLIMITED && runner_result.memory_used > runner_config.memory_limit)
        runner_result.status = MEMORY_LIMIT_EXCEEDED;
    }
  }
}

#define STACK_SIZE (1024 * 1024) /* Stack size for cloned child */

int run()
{
  struct timeval start_time, end_time;
  gettimeofday(&start_time, NULL);
  /* Allocate memory to be used for the stack of the child. */
  char *stack;    /* Start of stack buffer */
  char *stackTop; /* End of stack buffer */

  stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
               MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
  if (stack == MAP_FAILED)
    INTERNAL_ERROR_EXIT("mmap err");

  stackTop = stack + STACK_SIZE; /* Assume stack grows downward */
  pid_t proxy_pid = clone(
      sandbox_proxy, // Function to execute as the body of the new process
      stackTop,      // Pass our stack, aligned to 16-bytes
      SIGCHLD | CLONE_NEWIPC | (runner_config.share_net ? 0 : CLONE_NEWNET) | CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWUTS,
      0); // Pass the arguments

  if (proxy_pid < 0)
    INTERNAL_ERROR_EXIT("Cannot run proxy, clone failed");
  if (!proxy_pid)
    INTERNAL_ERROR_EXIT("Cannot run proxy, clone returned 0");

  monitor(proxy_pid, &start_time, &end_time);
  return 0;
}
