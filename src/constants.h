#ifndef CONSTANTS_HEADER
#define CONSTANTS_HEADER

#include <string.h>
#include <signal.h>

#include "log.h"
#include "utils.h"

// 还未执行答案检查
#define PENDING -1
// 答案正确
#define ACCEPTED 0
// 换行问题
#define PRESENTATION_ERROR 1
// 超时
#define TIME_LIMIT_EXCEEDED 2
// 超内存限制
#define MEMORY_LIMIT_EXCEEDED 3
// 答案错误
#define WRONG_ANSWER 4
// 用户的程序运行时发生错误
#define RUNTIME_ERROR 5
// 编译错误
#define COMPILE_ERROR 6
// 判题系统发生错误
#define SYSTEM_ERROR 7

#define CALLS_MAX 400

// error_code 的错误代码
// 要运行的命令没有找到
#define COMMAND_NOT_FOUND 1

// 资源相关
#define RESOURCE_UNLIMITED 0
#define CMD_MAX_LENGTH 20

#define LIMITS_MAX_OUTPUT 128 * 1024 * 1024
#define LIMITS_MAX_FD 100

struct Result
{
  int status;
  int cpu_time_used;
  long cpu_time_used_us;
  int real_time_used;
  long real_time_used_us;
  int memory_used;
  int signal_code;
  int exit_code;
  int error_code;
};

struct Config
{
  char **cmd;
  char *log_file;
  int cpu_time_limit;
  int real_time_limit;
  int memory_limit;
  int memory_check_only;
  int std_in;
  int std_out;
  int std_err;
  int share_net;
  char *in_file;
  char *out_file;
  char *stdout_file;
  char *stderr_file;
  char *save_file;
};

#define CHILD_ERROR_EXIT(message)                                                               \
  {                                                                                             \
    int _errno = errno;                                                                         \
    log_fatal("child error: %s, errno: %d, strerror: %s; ", message, _errno, strerror(_errno)); \
    CLOSE_FD(input_fd);                                                                         \
    CLOSE_FD(output_fd);                                                                        \
    CLOSE_FD(err_fd);                                                                           \
    CLOSE_FD(null_fd);                                                                          \
    if (_errno == 2)                                                                            \
      raise(SIGUSR2);                                                                           \
    else                                                                                        \
      raise(SIGUSR1);                                                                           \
    exit(_errno);                                                                               \
  }

#define INTERNAL_ERROR_EXIT(message, arg...)                                       \
  {                                                                                \
    log_fatal("Interlnal Error: errno: %d, strerror: %s", errno, strerror(errno)); \
    log_fatal(message, ##arg);                                                     \
    exit(EXIT_FAILURE);                                                            \
  }

#endif
