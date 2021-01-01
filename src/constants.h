#ifndef CONSTANTS_HEADER
#define CONSTANTS_HEADER

#define PENDING -1 // 还未执行答案检查
#define ACCEPTED 0
#define PRESENTATION_ERROR 1
#define TIME_LIMIT_EXCEEDED 2
#define MEMORY_LIMIT_EXCEEDED 3
#define WRONG_ANSWER 4
#define RUNTIME_ERROR 5
#define OUTPUT_LIMIT_EXCEEDED 6
#define COMPILE_ERROR 7
#define SYSTEM_ERROR 8

#define CALLS_MAX 400
#define MAX_OUTPUT 100000000

struct Result
{
  int status;
  int cpu_time_used;
  long cpu_time_used_us;
  int memory_used;
  long memory_used_b;
  int signal;
  int exit_code;
};

struct Config
{
  int run_only;
  int check_only;
  char *cmd[20];
  char *log_file;
  int time_limit;
  int memory_limit;
  char *in_file;
  char *out_file;
  char *user_out_file;
};

#endif
