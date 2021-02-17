#define _GNU_SOURCE
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

void get_opts_str(char str[], char *opts[], int length)
{
  char *tmp = NULL;

  for (int i = 0; i < length; i++)
  {
    if (asprintf(&tmp, "<%s> ", opts[i]) < 0)
      break;
    if (i == 0)
    {
      strcpy(str, tmp);
    }
    else
    {
      strcat(str, tmp);
    }
    if (tmp != NULL)
    {
      free(tmp);
      tmp = NULL;
    }
  }
}

#define JUDGE_OPTS_LENGTH 6

char *judge_opts[JUDGE_OPTS_LENGTH] = {
    "command",
    "time_limit",
    "memory_limit",
    "testdata_input_path",
    "testdata_output_path",
    "tmp_output_path",
};

void judge_usage()
{
  char str[80];
  get_opts_str(str, judge_opts, JUDGE_OPTS_LENGTH);
  printf("Usage: judge judge %s [options]\n", str);
  printf("\n");
  printf("e.g. judge process with input data file and tmp output path, and log path.\n");
  printf("\t./judge judge ./main 1000 2048 ./tests/1/1.in ./tests/1/1.out 1.tmp.out -l 1.log\n");
  printf("\n");
  common_options();
}

#define RUN_OPTS_LENGTH 5

char *run_opts[RUN_OPTS_LENGTH] = {
    "command",
    "time_limit",
    "memory_limit",
    "testdata_input_path",
    "tmp_output_path",
};

void run_usage()
{
  char str[80];
  get_opts_str(str, run_opts, RUN_OPTS_LENGTH);
  printf("Usage: judge run %s [options]\n", str);
  printf("\n");
  printf("e.g. Run process with input data file and tmp output path, and log path.\n");
  printf("\t./judge run ./main 1000 2048 ./tests/1/1.in 1.tmp.out -l 1.log\n");
  printf("\n");
  common_options();
}

#define CHECK_OPTS_LENGTH 2

char *check_opts[CHECK_OPTS_LENGTH] = {
    "testdata_output_path",
    "tmp_output_path",
};

void check_usage()
{
  char str[80];
  get_opts_str(str, check_opts, CHECK_OPTS_LENGTH);
  printf("Usage: judge check %s [options]\n", str);
  printf("\n");
  printf("e.g. Judge answers with <testdata_output_path> and <tmp_output_path>.\n");
  printf("\t./judge check ./tests/1/1.out 1.tmp.out -l 1.log\n");
  printf("\n");
  common_options();
}

int last_optind = 0;
char *next(int argc, char *argv[])
{
  last_optind++;
  if (optind + last_optind >= argc)
  {
    printf("缺少参数。\n");
    exit(EXIT_FAILURE);
  }
  return argv[optind + last_optind];
}

#define NEXT_ARG next(argc, argv)

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
    process_cmd(config->cmd, NEXT_ARG);
    config->time_limit = atoi(NEXT_ARG);
    config->memory_limit = atoi(NEXT_ARG);
    config->in_file = NEXT_ARG;
    config->out_file = NEXT_ARG;
    config->user_out_file = NEXT_ARG;
  }
  else if (strcmp(mode, "run") == 0)
  {
    config->run_mode = 1;
    process_cmd(config->cmd, NEXT_ARG);
    config->time_limit = atoi(NEXT_ARG);
    config->memory_limit = atoi(NEXT_ARG);
    config->in_file = NEXT_ARG;
    config->user_out_file = NEXT_ARG;
  }
  else if (strcmp(mode, "check") == 0)
  {
    config->check_mode = 1;
    config->out_file = NEXT_ARG;
    config->user_out_file = NEXT_ARG;
  }
}
