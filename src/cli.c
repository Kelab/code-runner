#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <argp.h>

#include "constants.h"
#include "utils.h"

const char *argp_program_version = "judge 0.1.0";

static char args_doc[] = "<command> [args for command]";

static char doc[] =
    "judge -- made with hard work and 🧡\
\n\
\ne.g. `judge node main.js -t 1000 --mco` \
\v\
\nIf you want to pass a option(has a leading `-`) to <command> , you need to put them after the \
`--` argument(which prevents anything following being interpreted as an option).\
\n  e.g. \
\n    - judge -t 1000 --mco python main.py -- -OO \
\n    - judge node -t 1000 -- --version \
\n    - judge -t 1000 -- node --version \
\nThat's all.";

/* Keys for options without short-options. */
#define OPT_MEMORY_CHECK_ONLY 3

#define OPT_CPU_TIME_LIMIT 't'
#define OPT_MEMORY_LIMIT 'm'
#define OPT_SYSTEM_INPUT 'i'
#define OPT_SYSTEM_OUTPUT 'o'
#define OPT_USER_OUTPUT 'u'
#define OPT_LOG_FILE 'l'
#define OPT_REAL_TIME_LIMIT 'r'

static struct argp_option options[] = {
    {"cpu_time_limit", OPT_CPU_TIME_LIMIT, "MS", 0, "cpu_time limit (default 0) ms, when 0, not check", 1},
    {"memory_limit", OPT_MEMORY_LIMIT, "KB", 0, "memory limit (default 0) kb, when 0, not check", 1},
    {"system_input", OPT_SYSTEM_INPUT, "FILE", 0, "system_input path", 2},
    {"system_output", OPT_SYSTEM_OUTPUT, "FILE", 0, "system_output path", 2},
    {"user_output", OPT_USER_OUTPUT, "FILE", 0, "user out -> file path", 2},

    {0, 0, 0, 0, "Optional options:"},
    {"real_time_limit", OPT_REAL_TIME_LIMIT, "MS", 0, "real_time_limit (default 0) ms"},
    {"memory_check_only", OPT_MEMORY_CHECK_ONLY, 0, OPTION_ARG_OPTIONAL, "not set memory limit in run, (default not check)"},
    {"mco", OPT_MEMORY_CHECK_ONLY, 0, OPTION_ALIAS},
    {"log_file", OPT_LOG_FILE, "FILE", 0, "log file path, (default not output)"},
    {0},
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
  /* input 是之前我们调用 argp_parse 传入的第四个参数 */
  struct Config *config = state->input;
  switch (key)
  {
  case OPT_CPU_TIME_LIMIT:
    config->cpu_time_limit = arg ? atoi(arg) : 0;
    break;
  case OPT_MEMORY_LIMIT:
    config->memory_limit = arg ? atoi(arg) : 0;
    break;
  case OPT_SYSTEM_INPUT:
    config->in_file = arg;
    break;
  case OPT_SYSTEM_OUTPUT:
    config->out_file = arg;
    break;
  case OPT_USER_OUTPUT:
    config->user_out_file = arg;
    break;
  case OPT_REAL_TIME_LIMIT:
    config->real_time_limit = arg ? atoi(arg) : 5000;
    break;
  case OPT_MEMORY_CHECK_ONLY:
    config->memory_check_only = 1;
    break;
  case OPT_LOG_FILE:
    config->log_file = arg;
    break;
  case ARGP_KEY_NO_ARGS:
    argp_usage(state);
  case ARGP_KEY_ARG:
    config->cmd = &state->argv[state->next - 1];
    /* by setting state->next to the end
         of the arguments, we can force argp to stop parsing here and
         return. */
    state->next = state->argc;
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp judge_argp = {options, parse_opt, args_doc, doc};

int parse_argv(int argc, char **argv, struct Config *config)
{
  /* Parse our arguments; every option seen by parse_opt will be
     reflected in arguments. */
  argp_parse(&judge_argp, argc, argv, 0, 0, config);
  return 0;
}
