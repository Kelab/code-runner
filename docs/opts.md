# 选项

## 查看帮助

执行 `./runner --help`：

就能看到居多 options 了。。。

```shell
  -m, --memory_limit=KB      memory limit (default 0) kb, when 0, not check
  -t, --cpu_time_limit=MS    cpu_time limit (default 0) ms, when 0, not check
  -i, --system_input=FILE    system_input path
  -o, --system_output=FILE   system_output path
  -e, --user_err=FILE        user error output -> file path
  -u, --user_output=FILE     user outputs -> file path
  -s, --save=FILE            save result to file

 Optional options:
  -l, --log_file=FILE        log file path, (default not output)
      --memory_check_only, --mco   not set memory limit in run, (default not
                             check)
  -r, --real_time_limit=MS   real_time_limit (default 0) ms
      --stderr               use stderr
      --stdin                use stdin
      --stdout               use stdout

  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version
```

## 限制类

### -m, --memory_limit=KB

内存限制。

### -t, --cpu_time_limit=MS

CPU 时间限制。

`cpu_time` 的意思是用户在程序中用到的 CPU 计算所消耗的时间，不包括 IO 或者挂起时间。
`real_time` 是用户程序真实运行的时间。

## 输入输出文件相关

### -i, --system_input=FILE

要 stdin 给用户程序的文件。

### -o, --system_output=FILE

用户程序应该输出的内容，用于结果对比。

### -e, --user_err=FILE

用户的程序的 stderr 输出，不开就啥都不输出。

### -u, --user_output=FILE

用户程序实际 stdout 的内容，被重定向到这个文件中。

### -s, --save=FILE

把那个 json 文件保存起来，不输出。

## 一些增强项

### 允许 stdio

```txt
  --stderr
  --stdin
  --stdout
```

因为 runner 只会输出自己的执行结果，所以如果不设置输出到文件的情况下，用户输出的数据就会被重定向到 `/dev/null`。
这个时候你可以使用这三个选项来激活用户程序的标准输入输出。

### -l, --log_file=FILE

日志文件。

## --memory_check_only, --mco

不真正的限制内存，只根据用户的输入值进行判题结果检查，看是否超内存。

## -r, --real_time_limit=MS

真实时间限制。
