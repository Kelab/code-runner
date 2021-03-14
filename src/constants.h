#ifndef CONSTANTS_HEADER
#define CONSTANTS_HEADER

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

struct Result
{
  int status;
  int cpu_time_used;
  long cpu_time_used_us;
  int real_time_used;
  long real_time_used_us;
  int memory_used;
  int signal;
  int exit_code;
  int error_code;
};

#define CMD_MAX_LENGTH 20

struct Config
{
  char **cmd;
  char *log_file;
  int cpu_time_limit;
  int real_time_limit;
  int memory_limit;
  int memory_check_only;
  char *in_file;
  char *out_file;
  char *stdout_file;
  char *stderr_file;
};

#endif
