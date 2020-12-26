#ifndef __RUN_HEADER
#define __RUN_HEADER

struct result
{
  int status;
  int timeUsed;
  int memoryUsed;
};

int run(char *args[], int timeLimit, int memoryLimit, char *in_file, char *out_file, char *result_file);

#endif
