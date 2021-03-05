# Judge

根据判题数据，判定用户程序的运行结果以及获取用户程序运行时间和内存消耗。

判题结果会以 JSON 格式输出到标准输出中。

结果：

```json
{
  "status": 0,
  "cpu_time_used": 0,
  "cpu_time_used_us": 479,
  "real_time_used": 2,
  "real_time_used_us": 1966,
  "memory_used": 1556,
  "signal": 0,
  "exit_code": 0
}
```

## 编译

需要安装 `gcc-9`，然后设置 CC 变量：`export CC=gcc-9`。

```bash
make judge
```

会在当前目录编译出一个可执行文件 `judge`。

```bash
make libjudge
```

会在当前目录编译出一个共享库 `libjudge.so`。

## 开发

你可以使用 VSCode 提供的 VSCode Dev Container 在 Dokcer 容器中开发。
使用 VSCode 打开本文件夹即能看到提示。

## 运行

下面简单介绍一个例子：

![image](https://user-images.githubusercontent.com/13938334/109407241-1baafa00-79ba-11eb-8f14-51fa0ee23d27.png)

判本仓库根目录下 `tests/node/` 的题。

只需要执行 `./judge [选项...] <命令> [参数...]` 即可，比如：

```sh
./judge -l node.log -t 1000 -m 2048 --mco -i ./tests/node/1.in -o ./tests/node/1.out -u node.tmp.out -- node ./tests/node/main.js
```

各参数意义：

- `-l, --log_file` Log 日志路径
- `-t, --cpu_time_limit` CPU 时间限制 ，单位是 ms，用于判断是否超过 CPU 时间限制。
- `-m, --memory_limit` 内存限制，单位是 kb，用于判断是否超过内存限制。
- `--memory_check_only, --mco` 只进行内存限制的检查（要确认判题结果），而不进行真正的内存限制，因为在执行 Node.js 时，限制内存会导致 `node` 程序无法正常执行，会报段错误。
- `-i, --system_input` 从该文件读入数据当成待判程序的标准输入，如果不设置，也可以直接向程序使用 pipe 的方式输入数据。
- `-o, --system_output` 判题数据的输出，用于比对程序是否运行正确。
- `-u, --user_output` 将待判程序的标准输出写入该文件。

更多选项可以输入 `./judge -?` 查看帮助。

如果执行待判程序的命令的参数中需要使用到 `-`（如想用判题程序执行： `python --version`），那你需要将这个参数放在 `--` 后，如：

```bash
./judge [选项...] <命令> [参数...] -- [放在这里...]
# 比如
./judge -t 2000 -- python --version
```

反正只要是「想传给待判程序的，并且以 `-` 开头的的参数」，就要放在 `--` 后面，只要是放在 `--` 前面任意位置的以 `-` 开头的的参数就会认为是判题程序的参数。

## 运行结果

首先要判断 judge 程序是否运行成功，看进程的退出值。

这个退出值是判题程序判题是否成功的标识。

然后以 JSON 形式读取判题程序的标准输出（JSON 格式），看以下两个值：

```json
  "signal": 0,
  "exit_code": 0
```

`signal` 是导致子程序退出的信号值。
`exit_code` 是子程序的退出值。

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

注意：如果仅执行了 `run` 模式，并且程序运行没有错误或者超过资源限制的话，输出结果的 `status` 应该为: `-1`。

### 输出单位

其中 `cpu_time_used` 和 `real_time_used` 单位都是毫秒(ms)。
`cpu_time_used_us` 和 `real_time_used_us` 单位是微秒(us)。

`cpu_time` 的意思是用户在程序中用到的 CPU 计算所消耗的时间，不包括 IO 或者挂起时间。
`real_time` 是用户程序真实运行的时间。

`memory_used` 在 linux 下单位是 kb。

## FAQ

### rusage 的 `ru_utime` 和 `ru_stime` 有什么区别？

[来源](https://www.reddit.com/r/cs50/comments/553okd/difference_between_ru_utime_and_ru_stime/)

操作系统出现的原因就是为了在同时运行多个程序时可以共享对硬件的访问。

CPU 时间有时候花费是在运行用户的程序上，而有时候花费在进行系统调用(syscall)上（比如从磁盘或键盘读取数据）。

运行用户程序的时间被标记为「用户时间」，进行系统调用的时间被标记为「系统时间」。二者分别是 `rusage` 的 `ru_utime` 和 `ru_stime` 。

所有涉及访问硬件的行为都是在名为内核模式(Kernel Mode)的特殊模式下完成的。

你的程序不允许直接接触硬件（例如磁盘）；你必须通过请求操作系统来完成接触硬件的操作。

而且，你的程序也不被允许直接进入内核模式。如果你想从磁盘读取文件，那你必须通过请求操作系统提供的特定服务来完成这个行为。

这就是操作系统如何调节硬件的使用。

### 常见 `signal` 和 `exit_code`

`signal`(<https://man7.org/linux/man-pages/man7/signal.7.html>):

| signal | code    | 解释                                                                                     |
| ------ | ------- | ---------------------------------------------------------------------------------------- |
| 10     | SIGUSR1 | judge 程序发出的表示程序结束的 signal                                                    |
| 11     | SIGSEGV | 段错误，程序出现空指针，如果是在执行**需要运行时**的程序的时候，可以试试不设置内存限制。 |

`exit_code`:

| exit_code | 解释                               |
| --------- | ---------------------------------- |
| 127       | shell 报出来的 `command not found` |

### 在其他语言中调用

可以以启动子进程的方式来调用 `judge`，然后捕获控制台的输出即可。  

## 开源致谢

项目中使用到的开源库链接：

- [rxi/log.c](https://github.com/rxi/log.c)

项目开发过程中参考的项目：

- [1510460325/judge-runner](https://github.com/1510460325/judge-runner)
- [dojiong/Lo-runner](https://github.com/dojiong/Lo-runner/)
- [QingdaoU/Judger](https://github.com/QingdaoU/Judger)

对以上项目表示感谢。
