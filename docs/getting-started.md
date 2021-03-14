# 开始使用

## 编译

需要在 GCC 环境下编译使用（Mac 可以使用 VSCode 提供的 VSCode Dev Container 在容器中开发）。

需要设置 CC 变量，如：`export CC=gcc-9`，不设置则会使用系统默认的编译器。

```bash
make judge
```

会在当前目录编译出一个可执行文件 `judge`。

```bash
make libjudge
```

会在当前目录编译出一个共享库 `libjudge.so`。

编译报错请检查是否装了 `build-essential`：

```sh
sudo apt-get update
sudo apt-get -y install build-essential
```

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

```cpp
// 还未执行答案检查
#define PENDING -1
// 答案正确
#define ACCEPTED 0
// 换行问题
#define PRESENTATION_ERROR 1
// 超时
#define TIME_LIMIT_EXCEEDED 2
// 超内存限制
#define MEMORY_LIMIT_EXCEEDED 3
// 答案错误
#define WRONG_ANSWER 4
// 用户的程序运行时发生错误
#define RUNTIME_ERROR 5
// 编译错误
#define COMPILE_ERROR 6
// 判题系统发生错误
#define SYSTEM_ERROR 7
```

注意：如果仅执行了 `run` 模式，并且程序运行没有错误或者超过资源限制的话，输出结果的 `status` 应该为: `-1`。

### 输出单位

其中 `cpu_time_used` 和 `real_time_used` 单位都是毫秒(ms)。
`cpu_time_used_us` 和 `real_time_used_us` 单位是微秒(us)。

`cpu_time` 的意思是用户在程序中用到的 CPU 计算所消耗的时间，不包括 IO 或者挂起时间。
`real_time` 是用户程序真实运行的时间。

`memory_used` 在 linux 下单位是 kb。
