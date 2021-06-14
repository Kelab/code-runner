#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <argp.h>

#include "constants.h"
#include "utils.h"

extern struct Config runner_config;
extern struct Result runner_result;

const char *argp_program_version = "v0.1.0";

static char args_doc[] = "COMMAND [ARG...] [-- <COMMAND_FLAG>...]";

static char doc[] =
    "runner -- made with 🧡\
\n\
\ne.g. `runner node main.js -t 1000 --mco` \
\v\
\nIf you want to pass a argument(has a leading `-`) to <command> , you need to put it after the \
`--` argument(which prevents anything following being interpreted as an option).\
\n  e.g. \
\n    - runner -t 1000 --mco python main.py -- -OO \
\n    - runner node -t 1000 -- --version \
\n    - runner -t 1000 -- node --version \
\nThat's all.";

#define OPT_SHARE_NET 1
#define OPT_MEMORY_CHECK_ONLY 4

#define OPT_CPU_TIME_LIMIT 't'
#define OPT_MEMORY_LIMIT 'm'
#define OPT_STDIN 'i'
#define OPT_STDOUT 'u'
#define OPT_STDERR 'e'
#define OPT_TESTDATA_OUTPUT 'o'
#define OPT_LOG_FILE 'l'
#define OPT_REAL_TIME_LIMIT 'r'
#define OPT_SAVE_RESULT 's'
#define OPT_ATTACH 'a'

static struct argp_option options[] = {
    {"cpu_time_limit", OPT_CPU_TIME_LIMIT, "TIME", 0, "cpu_time limit (default 0) ms, when 0, not check", 1},
    {"memory_limit", OPT_MEMORY_LIMIT, "SIZE", 0, "memory limit (default 0) kb, when 0, not check", 1},
    {"stdin", OPT_STDIN, "FILE", 0, "Redirect standard input from file. Otherwise, standard input is disabled.", 2},
    {"stdout", OPT_STDOUT, "FILE", 0, "Redirect standard output to file. Otherwise, standard output is disabled.", 2},
    {"stderr", OPT_STDERR, "FILE", 0, "Redirect standard error output to file. Otherwise, standard error is disabled.", 2},
    {"attach", OPT_ATTACH, "NAME", 0, "Attach to STDIN, STDOUT or STDERR", 3},
    {"testdata_output", OPT_TESTDATA_OUTPUT, "FILE", 0, "testdata output path", 3},
    {"save", OPT_SAVE_RESULT, "FILE", 0, "save result to file", 4},
    {0, 0, 0, 0, "Other options:"},
    {"real_time_limit", OPT_REAL_TIME_LIMIT, "MS", 0, "real_time_limit (default 0) ms"},
    {"shart_net", OPT_SHARE_NET, 0, OPTION_ARG_OPTIONAL, "runner will create a new network namespace default, This prevents the program from communicating with the outside world."},
    {"memory_check_only", OPT_MEMORY_CHECK_ONLY, 0, OPTION_ARG_OPTIONAL, "not set memory limit in run"},
    {"mco", OPT_MEMORY_CHECK_ONLY, 0, OPTION_ALIAS},
    {"log_file", OPT_LOG_FILE, "FILE", 0, "log file path, (default ./runner.log)"},
    {0},
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
  switch (key)
  {
  case OPT_CPU_TIME_LIMIT:
    runner_config.cpu_time_limit = arg ? atoi(arg) : 0;
    break;
  case OPT_MEMORY_LIMIT:
    runner_config.memory_limit = arg ? atoi(arg) : 0;
    break;
  case OPT_STDIN:
    runner_config.stdin_file = arg;
    break;
  case OPT_TESTDATA_OUTPUT:
    runner_config.testdata_out = arg;
    break;
  case OPT_STDOUT:
    runner_config.stdout_file = arg;
    break;
  case OPT_STDERR:
    runner_config.stderr_file = arg;
    break;
  case OPT_SAVE_RESULT:
    runner_config.save_file = arg;
    break;
  case OPT_REAL_TIME_LIMIT:
    runner_config.real_time_limit = arg ? atoi(arg) : 5000;
    break;
  case OPT_MEMORY_CHECK_ONLY:
    runner_config.memory_check_only = 1;
    break;
  case OPT_LOG_FILE:
    runner_config.log_file = arg;
    break;
  case OPT_ATTACH:
    if (str_equal(arg, "STDIN"))
    {
      runner_config.attach_stdin = 1;
    }
    else if (str_equal(arg, "STDOUT"))
    {
      runner_config.attach_stdout = 1;
    }
    else if (str_equal(arg, "STDERR"))
    {
      runner_config.attach_stderr = 1;
    }
    break;
  case OPT_SHARE_NET:
    runner_config.share_net = 1;
    break;
  case ARGP_KEY_NO_ARGS:
    argp_usage(state);
    break;
  case ARGP_KEY_ARG:
    runner_config.cmd = &state->argv[state->next - 1];
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

static struct argp runner_argp = {options, parse_opt, args_doc, doc};

int parse_argv(int argc, char **argv)
{
  /* Parse our arguments; every option seen by parse_opt will be
     reflected in arguments. */
  return argp_parse(&runner_argp, argc, argv, 0, 0, 0);
}
