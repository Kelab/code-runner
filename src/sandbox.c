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

#include "constants.h"
#include "utils.h"
#include "diff.h"

extern struct Config runner_config;
extern struct Result runner_result;

static int result_pipes[2];
static int write_result_to_fd;
static int read_result_from_fd;

static int status_pipes[2];

static pid_t box_pid;
static pid_t proxy_pid;

// 100 MB
#define STACK_SIZE (100 * 1024 * 1024) /* Stack size(bytes) for cloned child */

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
void monitor(pid_t child_pid)
{
  struct timeval start_time, end_time;
  gettimeofday(&start_time, NULL);
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
  if (write(status_pipes[1], &status, sizeof(status)) != sizeof(status))
    INTERNAL_ERROR_EXIT("Proxy write to pipe failed");

  gettimeofday(&end_time, NULL);
  log_rusage(&ru);
  const struct timeval real_time_tv = {
      (end_time.tv_sec - start_time.tv_sec),
      (end_time.tv_usec - start_time.tv_usec),
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
  log_debug("child process memory_used %d", runner_result.memory_used);
  log_debug("child process real_time_used %d", runner_result.real_time_used);
  log_debug("child process cpu_time_used %d", runner_result.cpu_time_used);
}

static int do_write_result_to_fd()
{
  if (write_result_to_fd)
  {
    // We are inside the box, have to use error pipe for error reporting.
    // We hope that the whole error message fits in PIPE_BUF bytes.
    char buf[1024];
    int n = format_result(buf);
    return write(write_result_to_fd, buf, n);
  }
  return 0;
}

static int sandbox_proxy(void *_arg)
{
  write_result_to_fd = result_pipes[1];
  close(result_pipes[0]);
  close(status_pipes[0]);

  pid_t inside_pid = fork();

  if (inside_pid < 0)
  {
    INTERNAL_ERROR_EXIT("Cannot run process, fork failed");
  }
  else if (!inside_pid)
  {
    close(status_pipes[1]);
    child_process();
    _exit(42); // We should never get here
  }

  if (write(status_pipes[1], &inside_pid, sizeof(inside_pid)) != sizeof(inside_pid))
    INTERNAL_ERROR_EXIT("Proxy write to pipe failed");

  monitor(inside_pid);

  // 子程序运行失败的话，直接输出结果。不需要进行后面的 diff 了
  if (runner_result.exit_code || runner_result.signal_code)
  {
    return do_write_result_to_fd();
  }

  if (runner_result.status <= ACCEPTED)
  {
    diff();
  }

  return do_write_result_to_fd();
}

static void find_box_pid(void)
{
  /*
   *  The box keeper process wants to poll status of the inside process,
   *  so it needs to know the box_pid. However, it is not easy to obtain:
   *  we got the PID from the proxy, but it is local to the PID namespace.
   *  Instead, we ask /proc to enumerate the children of the proxy.
   *
   *  CAVEAT: The timing is tricky. We know that the inside process was
   *  already started (passing the PID from the proxy to us guarantees it),
   *  but it might already have exited and be reaped by the proxy. Therefore
   *  it is correct if we fail to find anything.
   */

  char namebuf[256];
  snprintf(namebuf, sizeof(namebuf), "/proc/%d/task/%d/children", (int)proxy_pid, (int)proxy_pid);
  FILE *f = fopen(namebuf, "r");
  if (!f)
    return;

  int child;
  if (fscanf(f, "%d", &child) != 1)
  {
    fclose(f);
    return;
  }
  box_pid = child;

  if (fscanf(f, "%d", &child) == 1)
    INTERNAL_ERROR_EXIT("Error parsing %s: unexpected children found", &namebuf);

  fclose(f);
}

static void
box_keeper(void)
{
  read_result_from_fd = result_pipes[0];
  close(result_pipes[1]);
  close(status_pipes[1]);

  struct rusage rus;
  int stat;
  pid_t p;
  p = wait4(proxy_pid, &stat, 0, &rus);
  if (p < 0)
  {
    INTERNAL_ERROR_EXIT("wait4");
  }
  if (p != proxy_pid)
    INTERNAL_ERROR_EXIT("wait4: unknown pid %d exited!", p);
  proxy_pid = 0;

  // Check error pipe if there is an internal error passed from inside the box
  char interr[1024];
  int n = read(read_result_from_fd, interr, sizeof(interr) - 1);
  if (n > 0)
  {
    interr[n] = 0;
    if (runner_config.save_file)
    {
      log_debug("save result into file %s", runner_config.save_file);
      write_file(runner_config.save_file, interr);
    }
    else
    {
      log_debug("print result_message");
      printf("%s\n", interr);
    }
    log_debug("result  %s", interr);
  }

  // Check status pipe if there is an exit status reported by the proxy process
  n = read(status_pipes[0], &stat, sizeof(stat));
  if (n != sizeof(stat))
    INTERNAL_ERROR_EXIT("Did not receive exit status from proxy");
  log_debug("exit status  %d", stat);
}

int run_in_sandbox()
{
  /* Allocate memory to be used for the stack of the child. */
  char *stack; /* Start of stack buffer */

  // Stacks grow downward on all
  // processors that run Linux (except the HP PA processors), so stack
  // usually points to the topmost address of the memory space set up
  // for the child stack.
  stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
               MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
  if (stack == MAP_FAILED)
    INTERNAL_ERROR_EXIT("Cannot run proxy, mmap err");

  setup_pipe(result_pipes, 1);
  setup_pipe(status_pipes, 0);

  proxy_pid = clone(
      sandbox_proxy,
      stack + STACK_SIZE,
      SIGCHLD | CLONE_NEWIPC | (runner_config.share_net ? 0 : CLONE_NEWNET) | CLONE_NEWNS | CLONE_NEWPID,
      0);
  if (proxy_pid < 0)
    INTERNAL_ERROR_EXIT("Cannot run proxy, clone failed");
  if (!proxy_pid)
    INTERNAL_ERROR_EXIT("Cannot run proxy, clone returned 0");

  pid_t box_pid_inside_ns;
  int n = read(status_pipes[0], &box_pid_inside_ns, sizeof(box_pid_inside_ns));
  if (n != sizeof(box_pid_inside_ns))
    INTERNAL_ERROR_EXIT("Proxy failed before it passed box_pid");

  find_box_pid();
  log_debug("Started proxy_pid=%d box_pid=%d box_pid_inside_ns=%d", (int)proxy_pid, (int)box_pid, (int)box_pid_inside_ns);
  box_keeper();
  return 0;
}
