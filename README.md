# Judge

根据判题数据，判定用户程序的运行结果以及获取用户程序运行时间和内存消耗。

判题结果会输出为 JSON 格式到标准输出中。

结果：

```json
{
  "status": 0,
  "cpu_time_used": 0,
  "cpu_time_used_us": 462,
  "memory_used": 1544,
  "memory_used_b": 1581056,
  "signal": 0,
  "exit_code": 0
}
```

## 编译

```bash
make judge
```

会在当前目录编译出一个可执行文件 `judge`。

## 测试

```bash
make test1 # 运行程序并判题
make test1c # 只检查答案结果，输出判题状态
make test1r # 只运行程序以及记录程序输出结果
make cleantest1 # 清除 test1 例子相关的输出
```

会运行 `tests/1/` 这个例子并输出。

## 运行

```bash
❯ ./judge

Usage: judge <command> [<args>]

Available commands are:

judge   Run then compare.
run     Run the specified command only, do not check the result.
check   Compare the user's output and right answer to get the result.

Type 'judge help <command>' to get help for a specific command.
```

程序有三种命令模式：

- judge
  完整判题模式
- run
  只根据输入运行用户程序
- check
  给入题目答案数据和用户输出的数据，输出一个判题结果。

### judge 模式

```bash
❯ ./judge help judge

Usage: judge judge <command> <time_limit> <memory_limit> <testdata_input_path> <testdata_output_path> <tmp_output_path> [options]

e.g. judge process with input data file and tmp output path, and log path.
        ./judge judge ./main 1000 2048 ./tests/1/1.in ./tests/1/1.out 1.tmp.out -l 1.log

Options:

  -l    Path of the log file
```

- command 用户程序地址
- time_limit 时间单位是 ms
- memory_limit 内存单位是 kb
- input_path 判题的标准输入文件位置
- output_path 判题的标准输出文件位置
- tmp_output_path 用户程序执行的标准输出位置（用于判断答案是否正确）
- -l 参数可以传入一个 log 文件的地址，会把判题 log 写入该文件，方便调试。

之所以要多传入一个 `<tmp_output_path>`(`1.tmp.out`) 是因为可以：

1. 方便多步对程序执行结果进行判断。
2. 不用把判题输出保留在内存中。

### run 模式

```bash
❯ ./judge help run

Usage: judge run <command> <time_limit> <memory_limit> <testdata_input_path> <tmp_output_path> [options]

e.g. Run process with input data file and tmp output path, and log path.
        ./judge run ./main 1000 2048 ./tests/1/1.in 1.tmp.out -l 1.log

Options:

  -l    Path of the log file
```

### check 模式

```bash
❯ ./judge help check

Usage: judge check <testdata_output_path> <tmp_output_path> [options]

e.g. Judge answers with <testdata_output_path> and <tmp_output_path>.
        ./judge check ./tests/1/1.out 1.tmp.out -l 1.log

Options:

  -l    Path of the log file% 
```

## 使用

首先要判断 judge 程序是否运行成功，看进程的退出值。

然后以 JSON 形式读取程序的标准输出，看以下三个值：

```json
  "signal": 0,
  "exit_code": 0
```

`signal` 是导致程序退出的信号值。  
`exit_code` 是执行判题的程序的退出值。

如果都为 0，则说明本次判题执行成功。  
如果有不为 0 的值，可以在判题日志中查看更多信息。

status 是判题结果：

```c
#define PENDING -1 // 还未执行答案检查
#define ACCEPTED 0
#define PRESENTATION_ERROR 1
#define TIME_LIMIT_EXCEEDED 2
#define MEMORY_LIMIT_EXCEEDED 3
#define WRONG_ANSWER 4
#define RUNTIME_ERROR 5
#define OUTPUT_LIMIT_EXCEEDED 6
#define COMPILE_ERROR 7
#define SYSTEM_ERROR 8
```

如果是 `check` 模式的话，只会输出一个判题值，如：

```bash
❯ ./judge check -l 1.log ./tests/1/1.out 1.tmp.out
0
```

说明答案正确，AC。

### 在其他语言中调用

#### Python 例子

```python
import json
import subprocess

judge_path = "./judge"


def judge(proc_args):
    proc = subprocess.Popen(proc_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = proc.communicate()
    if err:
        raise ValueError("Error occurred while calling judge: {}".format(err))

    return json.loads(out.decode("utf-8"))


proc_args = [
    judge_path,
    "judge",
    "./main",
    "1000",
    "2048",
    "./tests/1/1.in",
    "./tests/1/1.out",
    "1.tmp.out",
    "-l",
    "1.log",
]

result = judge(proc_args)
print("result: ", result)
# result:  {'status': 0, 'cpu_time_used': 0, 'cpu_time_used_us': 638, 'memory_used': 1528, 'memory_used_b': 1564672, 'signal': 0, 'exit_code': 0}

proc_args = [
    judge_path,
    "run",
    "./main",
    "1000",
    "2048",
    "./tests/1/1.in",
    "1.tmp.out",
    "-l",
    "1.log",
]
result = judge(proc_args)
print("result: ", result)
# result:  {'status': -1, 'cpu_time_used': 0, 'cpu_time_used_us': 509, 'memory_used': 1568, 'memory_used_b': 1605632, 'signal': 0, 'exit_code': 0}

proc_args = [
    judge_path,
    "check",
    "./tests/1/1.out",
    "1.tmp.out",
    "-l",
    "1.log",
]
result = judge(proc_args)
print("result: ", result)
# result:  0
```

捕获控制台的输出即可。

## FAQ

### rusage 的 `ru_utime` 和 `ru_stime` 有什么区别？

[来源](https://www.reddit.com/r/cs50/comments/553okd/difference_between_ru_utime_and_ru_stime/)

操作系统的目的就是为了在同时运行许多程序时共享对硬件的访问。

CPU 有时候的时间花费是在运行程序上，而有时候的时间花费则是代表你在执行操作（比如从磁盘或键盘读取数据）。

运行程序的时间被标记为「用户时间」下，而你执行操作的时间被标记为「系统时间」。这就分别是 `utime` 和 `stime`。

所有涉及访问硬件的事情都是在称为内核模式(Kernel Mode)的特殊模式下完成的。
您的程序不允许直接接触硬件（例如磁盘）；它必须请求操作系统来执行此操作。
而且，你的程序也不被允许直接进入内核模式。你必须通过向操作系统询请求它准备提供的特定服务(如从磁盘读取)来输入它。
这就是操作系统如何调节硬件的使用。

## 开源致谢

项目中使用到的开源库链接：

- [rxi/log.c](https://github.com/rxi/log.c)

项目开发过程中参考的项目：

- [1510460325/judge-runner](https://github.com/1510460325/judge-runner)
- [dojiong/Lo-runner](https://github.com/dojiong/Lo-runner/)
- [QingdaoU/Judger](https://github.com/QingdaoU/Judger)

对以上项目表示感谢。
