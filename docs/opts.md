# 选项

## 查看帮助

```shell
$ ./judge --help
Usage: judge [OPTION...] <command> [args for command]
judge -- made with hard work and 🧡

e.g. `judge node main.js -t 1000 --mco` 

  -m, --memory_limit=KB      memory limit (default 0) kb, when 0, not check
  -t, --cpu_time_limit=MS    cpu_time limit (default 0) ms, when 0, not check
  -i, --system_input=FILE    system_input path
  -o, --system_output=FILE   system_output path
  -u, --user_output=FILE     user out -> file path

 Optional options:
  -l, --log_file=FILE        log file path, (default not output)
      --memory_check_only, --mco   not set memory limit in run, (default not
                             check)
  -r, --real_time_limit=MS   real_time_limit (default 0) ms

  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.


If you want to pass a option(has a leading `-`) to <command> , you need to put
them after the `--` argument(which prevents anything following being
interpreted as an option).
  e.g. 
    - judge -t 1000 --mco python main.py -- -OO 
    - judge node -t 1000 -- --version 
    - judge -t 1000 -- node --version 
That's all.
```

## -m, --memory_limit=KB

内存限制。

## -t, --cpu_time_limit=MS

CPU 时间限制。

## -i, --system_input=FILE

要 stdin 给用户程序的文件。

## -o, --system_output=FILE

用户程序应该输出的内容，用于结果对比。

## -u, --user_output=FILE

用户程序实际 stdout 的内容，被重定向到这个文件中。

## -l, --log_file=FILE

日志文件。

## --memory_check_only, --mco

不真正的限制内存，只根据用户的输入值进行判题结果检查，看是否超内存。

## -r, --real_time_limit=MS

真实时间限制。
