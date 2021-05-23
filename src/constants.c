#include "constants.h"

struct Result runner_result = {
    .status = PENDING,
    .cpu_time_used = 0,
    .cpu_time_used_us = 0,
    .real_time_used = 0,
    .real_time_used_us = 0,
    .memory_used = 0,
    .signal_code = 0,
    .exit_code = 0,
    .error_code = 0,
};

struct Config runner_config = {
    .memory_check_only = 0,
    .cpu_time_limit = 0,
    .real_time_limit = 0,
    .memory_limit = 0,
    .std_in = 0,
    .std_out = 0,
    .std_err = 0,
    .in_file = '\0',
    .out_file = '\0',
    .save_file = '\0',
    .stdout_file = '\0',
    .stderr_file = '\0',
    .log_file = "runner.log",
};
