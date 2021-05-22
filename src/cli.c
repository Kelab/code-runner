#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <argp.h>

#include "constants.h"
#include "utils.h"

const char *argp_program_version = "runner 0.1.0";

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

/* Keys for options without short-options. */
#define OPT_ENABLE_STDIN 1
#define OPT_ENABLE_STDOUT 2
#define OPT_ENABLE_STDERR 3
#define OPT_MEMORY_CHECK_ONLY 4

#define OPT_CPU_TIME_LIMIT 't'
#define OPT_MEMORY_LIMIT 'm'
#define OPT_SYSTEM_INPUT 'i'
#define OPT_SYSTEM_OUTPUT 'o'
#define OPT_USER_OUTPUT 'u'
#define OPT_USER_ERROR 'e'
#define OPT_LOG_FILE 'l'
#define OPT_REAL_TIME_LIMIT 'r'
#define OPT_SAVE_RESULT 's'
#define OPT_ATTACH 'a'

static struct argp_option options[] = {
    {"cpu_time_limit", OPT_CPU_TIME_LIMIT, "TIME", 0, "cpu_time limit (default 0) ms, when 0, not check", 1},
    {"memory_limit", OPT_MEMORY_LIMIT, "SIZE", 0, "memory limit (default 0) kb, when 0, not check", 1},
    {"system_input", OPT_SYSTEM_INPUT, "FILE", 0, "system_input path", 2},
    {"system_output", OPT_SYSTEM_OUTPUT, "FILE", 0, "system_output path", 2},
    {"user_output", OPT_USER_OUTPUT, "FILE", 0, "user outputs -> file path", 3},
    {"user_err", OPT_USER_ERROR, "FILE", 0, "user error output -> file path", 3},
    {"save", OPT_SAVE_RESULT, "FILE", 0, "save result to file", 4},

    {0, 0, 0, 0, "Optional options:"},
    {"real_time_limit", OPT_REAL_TIME_LIMIT, "MS", 0, "real_time_limit (default 0) ms"},
    {"memory_check_only", OPT_MEMORY_CHECK_ONLY, 0, OPTION_ARG_OPTIONAL, "not set memory limit in run, (default not check)"},
    {"mco", OPT_MEMORY_CHECK_ONLY, 0, OPTION_ALIAS},
    {"attach", OPT_ATTACH, "NAME", 0, "Attach to STDIN, STDOUT or STDERR"},
    {"stdin", OPT_ENABLE_STDIN, 0, OPTION_ARG_OPTIONAL, "use stdin"},
    {"stdout", OPT_ENABLE_STDOUT, 0, OPTION_ARG_OPTIONAL, "use stdout"},
    {"stderr", OPT_ENABLE_STDERR, 0, OPTION_ARG_OPTIONAL, "use stderr"},
    {"log_file", OPT_LOG_FILE, "FILE", 0, "log file path, (default ./runner.log)"},
    {0},
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
  // An arbitrary pointer passed in from the user.
  struct Config *_config = state->input;
  switch (key)
  {
  case OPT_CPU_TIME_LIMIT:
    _config->cpu_time_limit = arg ? atoi(arg) : 0;
    break;
  case OPT_MEMORY_LIMIT:
    _config->memory_limit = arg ? atoi(arg) : 0;
    break;
  case OPT_SYSTEM_INPUT:
    _config->in_file = arg;
    break;
  case OPT_SYSTEM_OUTPUT:
    _config->out_file = arg;
    break;
  case OPT_USER_OUTPUT:
    _config->stdout_file = arg;
    break;
  case OPT_USER_ERROR:
    _config->stderr_file = arg;
    break;
  case OPT_SAVE_RESULT:
    _config->save_file = arg;
    break;
  case OPT_REAL_TIME_LIMIT:
    _config->real_time_limit = arg ? atoi(arg) : 5000;
    break;
  case OPT_MEMORY_CHECK_ONLY:
    _config->memory_check_only = 1;
    break;
  case OPT_ENABLE_STDIN:
    _config->std_in = 1;
    break;
  case OPT_ENABLE_STDOUT:
    _config->std_out = 1;
    break;
  case OPT_ENABLE_STDERR:
    _config->std_err = 1;
    break;
  case OPT_LOG_FILE:
    _config->log_file = arg;
    break;
  case OPT_ATTACH:
    if (equalStr(arg, "STDIN"))
    {
      _config->std_in = 1;
    }
    else if (equalStr(arg, "STDOUT"))
    {
      _config->std_out = 1;
    }
    else if (equalStr(arg, "STDERR"))
    {
      _config->std_err = 1;
    }
    break;
  case ARGP_KEY_NO_ARGS:
    argp_usage(state);
    break;
  case ARGP_KEY_ARG:
    _config->cmd = &state->argv[state->next - 1];
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

int parse_argv(int argc, char **argv, struct Config *config)
{
  /* Parse our arguments; every option seen by parse_opt will be
     reflected in arguments. */
  argp_parse(&runner_argp, argc, argv, 0, 0, config);
  return 0;
}
