
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

#define AC 0  // Accepted
#define PE 1  // Presentation Error
#define TLE 2 // Time Limit Exceeded
#define MLE 3 // Memory Limit Exceeded
#define WA 4  // Wrong Answer
#define RE 5  // Runtime Error
#define OLE 6
#define CE 7 // Compile Error
#define SE 8 // System Error

struct result
{
  int status;
  int timeUsed;
  int memoryUsed;
};

/*
set the process limit(cpu and memory)
*/
void setProcessLimit(int timelimit, int memory_limit)
{
  struct rlimit rl;
  /* set the time_limit (second)*/
  rl.rlim_cur = timelimit / 1000;
  rl.rlim_max = rl.rlim_cur + 1;
  setrlimit(RLIMIT_CPU, &rl);

  /* set the memory_limit (b)*/
  rl.rlim_cur = memory_limit * 1024;
  rl.rlim_max = rl.rlim_cur + 1024;
  setrlimit(RLIMIT_DATA, &rl);
}

/*
 run the user process
*/
void runCmd(char *args[], long timeLimit, long memoryLimit, char *in, char *out)
{

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
void getResult(struct rusage *ru, int data[2])
{
  data[0] = ru->ru_utime.tv_sec * 1000 + ru->ru_utime.tv_usec / 1000 + ru->ru_stime.tv_sec * 1000 + ru->ru_stime.tv_usec / 1000;
  data[1] = ru->ru_maxrss;
}

/*
monitor the user process
*/
void monitor(pid_t pid, int timeLimit, int memoryLimit, struct result *rest)
{
  // 获取子进程的退出状态
  int status;
  struct rusage ru;
  if (wait4(pid, &status, 0, &ru) == -1)
  {
    int errsv = errno;
    printf("wait4 error %s\n", strerror(errsv));
    exit(EXIT_FAILURE);
  }

  rest->timeUsed = ru.ru_utime.tv_sec * 1000 + ru.ru_utime.tv_usec / 1000 + ru.ru_stime.tv_sec * 1000 + ru.ru_stime.tv_usec / 1000;
  rest->memoryUsed = ru.ru_maxrss;

  // 若此值为非0 表明进程被信号终止，非正常结束
  if (WIFSIGNALED(status))
  {
    // 此时可通过 WTERMSIG(status) 获取使得进程退出的信号编号
    switch (WTERMSIG(status))
    {
    case SIGSEGV:
      if (rest->memoryUsed > memoryLimit)
        rest->status = MLE;
      else
        rest->status = RE;
      break;
    case SIGALRM:
    case SIGXCPU:
      rest->status = TLE;
      break;
    default:
      rest->status = RE;
      break;
    }
  }
  else
  {
    // 程序正常结束，此时可通过WEXITSTATUS(status)获取进程退出状态(exit时参数)
    int exit_code = WEXITSTATUS(status);
    if (exit_code == 0)
    {
      if (rest->timeUsed > timeLimit)
        rest->status = TLE;
      else if (rest->memoryUsed > memoryLimit)
        rest->status = MLE;
      else
        rest->status = AC;
    }
    else
    {
      int errsv = errno;
      printf("%s\n", strerror(errsv));
      rest->status = SE;
    }
  }
}

void write_result(char *result, char *log_file)
{
  FILE *_log_file = fopen(log_file, "w+");
  fprintf(_log_file, result);
  fclose(_log_file);
}

int run(char *args[], int timeLimit, int memoryLimit, char *in, char *out, char *log_file)
{
  // use `_exit` to abort the child program
  // 使用 vfork 时，当子进程运行失败时，要调用 _exit 让出进程控制权
  pid_t pid = vfork();
  if (pid < 0)
  {
    // The fork() function shall fail if:
    //   The system lacked the necessary resources to create another process,
    //   or the system - imposed limit on the total number of processes under execution system - wide or by a single user{CHILD_MAX} would be exceeded.
    // The fork() function may fail if:
    //   Insufficient storage space is available.
    printf("error in fork!\n");
    exit(EXIT_FAILURE);
  }
  else if (pid == 0)
  {
    // child process
    // 子进程与父进程共享数据段
    runCmd(args, timeLimit, memoryLimit, in, out);
  }
  else
  {
    // parent process
    // vfork 保证子进程先运行，在子进程调用 exec 或 exit 之后父进程才可能被调度运行
    struct result rest;
    monitor(pid, timeLimit, memoryLimit, &rest);
    char result[1000];
    sprintf(result, "{\"status\":\"%d\",\"timeUsed\":\"%d\",\"memoryUsed\":\"%d\"}", rest.status, rest.timeUsed, rest.memoryUsed);
    printf("%s\n", result);
    write_result(result, log_file);
  }
}

void split(char **arr, char *str, const char *del)
{
  char *s = NULL;
  s = strtok(str, del);
  while (s != NULL)
  {
    *arr++ = s;
    s = strtok(NULL, del);
  }
  *arr++ = NULL;
}

int main(int argc, char *argv[])
{
  char *cmd[20];
  split(cmd, argv[1], "@");
  run(cmd, atoi(argv[2]), atoi(argv[3]), argv[4], argv[5], argv[6]);
  return 0;
}
