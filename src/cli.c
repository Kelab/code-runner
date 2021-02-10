#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include "constants.h"
#include "utils.h"

extern char *optarg;
extern int optind;

void usage()
{
  printf("Usage: judge <command> [<args>]\n");
  printf("\n");
  printf("Available commands are:\n");
  printf("\n");
  printf("judge\tRun then compare.\n");
  printf("run\tRun the specified command only, do not check the result.\n");
  printf("check\tCompare the user's output and right answer to get the result.\n");
  printf("\n");
  printf("Type 'judge help <command>' to get help for a specific command.\n");
}

void common_options()
{
  printf("Options:\n");
  printf("\n");
  //  --log-file
  printf("  -l\tPath of the log file");
}

void judge_usage()
{
  printf("Usage: judge judge <command> <time_limit> <memory_limit> <testdata_input_path> <testdata_output_path> <tmp_output_path> [options]\n");
  printf("\n");
  printf("e.g. judge process with input data file and tmp output path, and log path.\n");
  printf("\t./judge judge ./main 1000 2048 ./tests/1/1.in ./tests/1/1.out 1.tmp.out -l 1.log\n");
  printf("\n");
  common_options();
}

void run_usage()
{
  printf("Usage: judge run <command> <time_limit> <memory_limit> <testdata_input_path> <tmp_output_path> [options]\n");
  printf("\n");
  printf("e.g. Run process with input data file and tmp output path, and log path.\n");
  printf("\t./judge run ./main 1000 2048 ./tests/1/1.in 1.tmp.out -l 1.log\n");
  printf("\n");
  common_options();
}

void check_usage()
{
  printf("Usage: judge check <testdata_output_path> <tmp_output_path> [options]\n");
  printf("\n");
  printf("e.g. Judge answers with <testdata_output_path> and <tmp_output_path>.\n");
  printf("\t./judge check ./tests/1/1.out 1.tmp.out -l 1.log\n");
  printf("\n");
  common_options();
}

int last_optind = 0;
char *next(char *argv[])
{
  last_optind++;
  return argv[optind + last_optind];
}

void parse_argv(int argc, char *argv[], struct Config *config)
{
  if (argc == 1)
  {
    usage();
    exit(EXIT_SUCCESS);
  }

  int opt;
  char *string = "l:";

  while ((opt = getopt(argc, argv, string)) != -1)
  {
    switch (opt)
    {
    case 'l':
      config->log_file = optarg;
      break;
    case '?':
      printf("Unknown option: %c\n", (char)optopt);
    }
  }

  char *mode = argv[optind];

  if (strcmp(mode, "help") == 0)
  {
    // 只有 judge help，展示 usage
    if (argc == 2)
    {
      usage();
    }
    else
    {
      char *command = argv[optind + 1];
      if (strcmp(command, "run") == 0)
      {
        run_usage();
      }
      else if (strcmp(command, "check") == 0)
      {
        check_usage();
      }
      else if (strcmp(command, "judge") == 0)
      {
        judge_usage();
      }
      else
      {
        printf("Unknown command: %s\n\n", command);
        usage();
      }
    }
    exit(EXIT_SUCCESS);
  }
  else if (strcmp(mode, "judge") == 0)
  {
    config->judge_mode = 1;
    process_cmd(config->cmd, next(argv));
    config->time_limit = atoi(next(argv));
    config->memory_limit = atoi(next(argv));
    config->in_file = next(argv);
    config->out_file = next(argv);
    config->user_out_file = next(argv);
  }
  else if (strcmp(mode, "run") == 0)
  {
    config->run_mode = 1;
    process_cmd(config->cmd, next(argv));
    config->time_limit = atoi(next(argv));
    config->memory_limit = atoi(next(argv));
    config->in_file = next(argv);
    config->user_out_file = next(argv);
  }
  else if (strcmp(mode, "check") == 0)
  {
    config->check_mode = 1;
    config->out_file = next(argv);
    config->user_out_file = next(argv);
  }
}
