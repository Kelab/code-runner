
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

#define RAISE_EXIT(err)      \
  {                          \
    printf("err %s\n", err); \
    _exit(EXIT_FAILURE);     \
  }

/**
 * set the process limit(cpu and memory)
 */
void setProcessLimit(int timelimit, int memory_limit)
{
  struct rlimit rl;

  rl.rlim_cur = timelimit / 1000;
  rl.rlim_max = rl.rlim_cur + 1;
  // CPU time limit in seconds.
  if (setrlimit(RLIMIT_CPU, &rl))
    RAISE_EXIT("set RLIMIT_CPU failure");

  rl.rlim_cur = memory_limit * 1024;
  rl.rlim_max = rl.rlim_cur + 1024;
  // The maximum size of the process's data segment (initialized data, uninitialized data, and heap)
  if (setrlimit(RLIMIT_DATA, &rl))
    RAISE_EXIT("set RLIMIT_DATA failure");

  rl.rlim_cur = memory_limit * 1024 * 2;
  // setrlimit(maxrss) may cause some crash issues
  rl.rlim_max = rl.rlim_cur + 1024;
  // The maximum size of the process's virtual memory (address space) in bytes.
  if (setrlimit(RLIMIT_AS, &rl))
    RAISE_EXIT("set RLIMIT_AS failure");

  rl.rlim_cur = 64 * 1024 * 1024;
  rl.rlim_max = rl.rlim_cur + 1024;
  // The maximum size of the process stack, in bytes.
  // Upon reaching this limit, a SIGSEGV signal is generated.
  if (setrlimit(RLIMIT_STACK, &rl))
    RAISE_EXIT("set RLIMIT_STACK failure");
}

/**
 * run the specific process
 */
void runCmd(char *args[], long timeLimit, long memoryLimit, char *in, char *out)
{
  // 重定向 标准输出IO 到相应的文件中
  int newstdin = open(in, O_RDWR | O_CREAT, 0644);
  int newstdout = open(out, O_RDWR | O_CREAT, 0644);
  setProcessLimit(timeLimit, memoryLimit);
  if (newstdout != -1 && newstdin != -1)
  {
    dup2(newstdout, fileno(stdout));
    dup2(newstdin, fileno(stdin));
    if (execvp(args[0], args) == -1)
    {
      _exit(EXIT_FAILURE);
    }
    close(newstdin);
    close(newstdout);
    _exit(EXIT_SUCCESS);
  }
  _exit(EXIT_FAILURE);
}

/*
get data of the user process
*/
void getResult(struct rusage *ru, struct result *rest)
{
  rest->cpu_time_used = ru->ru_utime.tv_sec * 1000 + ru->ru_utime.tv_usec / 1000 + ru->ru_stime.tv_sec * 1000 + ru->ru_stime.tv_usec / 1000;
  rest->cpu_time_used_us = ru->ru_utime.tv_sec * 1000 * 1000 + ru->ru_utime.tv_usec + ru->ru_stime.tv_sec * 1000 * 1000 + ru->ru_stime.tv_usec;
  rest->memory_used = ru->ru_maxrss;
  rest->memory_used_b = ru->ru_maxrss * 1024;
}

/**
 * monitor the user process
 */
void monitor(pid_t child_pid, int timeLimit, int memoryLimit, struct result *rest)
{
  // 获取子进程的退出状态
  int status;
  struct rusage ru;
  if (wait4(child_pid, &status, 0, &ru) == -1)
  {
    exit(EXIT_FAILURE);
  }

  getResult(&ru, rest);

  // 以下判断运行状态逻辑参考了：QingdaoU/Judger，dojiong/Lo-runner

  // 若此值为非0 表明进程被信号终止，说明子进程非正常结束
  if (WIFSIGNALED(status) != 0)
  {
    printf("非正常结束");
    // 此时可通过 WTERMSIG(status) 获取使得进程退出的信号编号
    rest->signal = WTERMSIG(status);
    switch (rest->signal)
    {
    case SIGUSR1:
      rest->status = SYSTEM_ERROR;
      break;
    case SIGSEGV:
      if (rest->memory_used > memoryLimit)
        rest->status = MEMORY_LIMIT_EXCEEDED;
      else
        rest->status = RUNTIME_ERROR;
      break;
    case SIGALRM:
    case SIGXCPU:
      rest->status = TIME_LIMIT_EXCEEDED;
      break;
    default:
      // 可能是超过资源限制之后的被 SIGKILL 杀掉
      if (rest->memory_used > memoryLimit)
        rest->status = MEMORY_LIMIT_EXCEEDED;
      else if (rest->cpu_time_used > timeLimit)
        rest->status = TIME_LIMIT_EXCEEDED;
      else
        rest->status = RUNTIME_ERROR;
      break;
    }
  }
  else
  {
    // 程序正常结束，此时可通过WEXITSTATUS(status)获取进程退出状态(exit时参数)
    rest->exit_code = WEXITSTATUS(status);

    if (rest->exit_code != 0)
    {
      int errsv = errno;
      printf("child process exited with error: %s\n", strerror(errsv));
      rest->status = RUNTIME_ERROR;
      rest->err_number = errsv;
    }
    else
    {
      if (rest->cpu_time_used > timeLimit)
        rest->status = TIME_LIMIT_EXCEEDED;
      else if (rest->memory_used > memoryLimit)
        rest->status = MEMORY_LIMIT_EXCEEDED;
    }
  }
}

void write_file(char *data, char *filename)
{
  FILE *_filename = fopen(filename, "w+");
  fprintf(_filename, data);
  fclose(_filename);
}

void init_result(struct result *_result)
{
  _result->status = AC;
  _result->cpu_time_used = _result->cpu_time_used_us = 0;
  _result->memory_used = _result->memory_used_b = 0;
  _result->signal = _result->exit_code = _result->err_number = 0;
}

int run(char *args[], int timeLimit, int memoryLimit, char *in_file, char *out_file, char *result_file)
{
  // use `_exit` to abort the child program
  // 使用 vfork 时，当子进程运行失败时，要调用 _exit 让出进程控制权
  // 子进程与父进程共享数据段
  pid_t child_pid = vfork();
  if (child_pid < 0)
  {
    // The fork() function shall fail if:
    //   The system lacked the necessary resources to create another process,
    //   or the system - imposed limit on the total number of processes under execution system - wide or by a single user{CHILD_MAX} would be exceeded.
    // The fork() function may fail if:
    //   Insufficient storage space is available.
    printf("error in fork!\n");
    exit(EXIT_FAILURE);
  }
  else if (child_pid == 0)
  {
    // child process
    runCmd(args, timeLimit, memoryLimit, in_file, out_file);
  }
  else
  {
    // parent process
    // vfork 保证子进程先运行，在子进程调用 exec 或 exit 之后父进程才可能被调度运行
    struct result rest;
    init_result(&rest);
    monitor(child_pid, timeLimit, memoryLimit, &rest);
    char result[1000];
    sprintf(result, "{\n"
                    "  \"status\": %d,\n"
                    "  \"cpu_time_used\": %d,\n"
                    "  \"cpu_time_used_us\": %ld,\n"
                    "  \"memory_used\": %d,\n"
                    "  \"memory_used_b\": %ld,\n"
                    "  \"signal\": %d,\n"
                    "  \"exit_code\": %d,\n"
                    "  \"err_number\": %d\n"
                    "}",
            rest.status,
            rest.cpu_time_used,
            rest.cpu_time_used_us,
            rest.memory_used,
            rest.memory_used_b,
            rest.signal,
            rest.exit_code,
            rest.err_number);
    printf("%s\n", result);
    write_file(result, result_file);
  }
  return 0;
}
