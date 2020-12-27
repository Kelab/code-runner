#ifndef __RUN_HEADER
#define __RUN_HEADER

struct result
{
  int status;
  int cpu_time_used;
  long cpu_time_used_us;
  int memory_used;
  long memory_used_b;
  int signal;
  int exit_code;
  int err_number;
};

int run(char *args[], int timeLimit, int memoryLimit, char *in_file, char *out_file, char *result_file);

#endif
