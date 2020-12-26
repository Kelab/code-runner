#include <stdlib.h>

#include "run.h"
#include "utils.h"

int main(int argc, char *argv[])
{
  char *cmd[20];
  split(cmd, argv[1], "@");
  run(cmd, atoi(argv[2]), atoi(argv[3]), argv[4], argv[5], argv[6]);
  return 0;
}
