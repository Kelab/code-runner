# Code 一览

## status

| status | code                  | 解释                       |
| ------ | --------------------- | -------------------------- |
| -1     | PENDING               | 还未执行答案检查           |
| 0      | ACCEPTED              | 答案正确                   |
| 1      | PRESENTATION_ERROR    | 换行问题，输出的换行符有误 |
| 2      | TIME_LIMIT_EXCEEDED   | 超时                       |
| 3      | MEMORY_LIMIT_EXCEEDED | 超内存限制                 |
| 4      | WRONG_ANSWER          | 答案错误                   |
| 5      | RUNTIME_ERROR         | 用户的程序运行时发生错误   |
| 6      | COMPILE_ERROR         | 编译错误                   |
| 7      | SYSTEM_ERROR          | 判题系统发生错误           |


## error_code

可以通过 `error_code` 判断判题哪里出错。

| error_code | 解释                                    |
| ---------- | --------------------------------------- |
| 1          | 要执行的程序没有找到。command not found |

## signal

<https://man7.org/linux/man-pages/man7/signal.7.html>

| signal | code    | 解释                                                                                     |
| ------ | ------- | ---------------------------------------------------------------------------------------- |
| 9      | SIGKILL | 可能是 real_time_limit 超过后被终止。                                                    |
| 10     | SIGUSR1 | runner 程序发出的表示程序结束的 signal，有可能是没找到要运行的程序。                      |
| 12     | SIGUSR2 | 没找到要运行的子程序。                                                                   |
| 11     | SIGSEGV | 段错误，程序出现空指针。如果是在执行**需要运行时**的程序的时候，可以试试不设置内存限制。 |

## exit_code

| exit_code | 解释                                                                             |
| --------- | -------------------------------------------------------------------------------- |
| 127       | shell 没找到命令。比如执行 `sh -c "command-not-found"` 的时候 exit_code 会是这个 |
