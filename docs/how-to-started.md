# 编译及使用

## 编译

需要在 GCC(7 以上) 环境下编译使用。

可以设置 `CC` 变量以置顶自己的编译器版本，如：`export CC=gcc-9`。

```bash
make runner
```

会在当前目录编译出一个可执行文件 `runner`。

```bash
make librunner
```

会在当前目录编译出一个共享库 `librunner.so`。


### 编译报错

编译报错请检查是否装了 `build-essential`：

在 Ubuntu 下可以安装：

```sh
sudo apt-get update
sudo apt-get -y install build-essential
```

### 在 alpine 下编译

```sh
apk update && apk add --no-cache git gcc musl-dev make argp-standalone
```

需要注意的是需要安装 `argp-standalone`，然后 make 打包的时候需要添加 `/usr/lib/libargp.a`。

## 运行

只需要执行 `./runner [选项...] <命令> [参数...]` 即可，比如：

```sh
./runner -l node.log -t 1000 -m 2048 --mco -i ./tests/node/1.in -o ./tests/node/1.out -u node.tmp.out -- node ./tests/node/main.js
```

各参数意义：

- `-l, --log_file` Log 日志路径
- `-t, --cpu_time_limit` CPU 时间限制 ，单位是 ms，用于判断是否超过 CPU 时间限制。
- `-m, --memory_limit` 内存限制，单位是 kb，用于判断是否超过内存限制。
- `--memory_check_only, --mco` 只进行内存限制的检查（要确认判题结果），而不进行真正的内存限制，因为在执行 Node.js 时，限制内存会导致 `node` 程序无法正常执行，会报段错误。
- `-i, --system_input` 从该文件读入数据当成待判程序的标准输入，如果不设置，也可以直接向程序使用 pipe 的方式输入数据。
- `-o, --system_output` 判题数据的输出，用于比对程序是否运行正确。
- `-u, --user_output` 将待判程序的标准输出写入该文件。

最后的 `node ./tests/node/main.js` 就是要执行的命令。

更多选项可以输入 `./runner --help` 查看帮助，或者查看 [选项](./opts.md)。

如果执行待判程序的命令的参数中需要使用到 `-`（如想用 runner 运行： `python --version`），那你需要将这个参数放在 runner 参数里的 `--` 后，如：

```bash
./runner [选项...] <命令> [参数...] -- [放在这里...]
# 比如
./runner -t 2000 -- python --version
```

反正只要是「想传给待判程序的，并且以 `-` 开头的的参数」，就要放在 `--` 后面。
放在 `--` 前面的话，会被认为是 runner 的参数。

## 读取运行结果

读取程序的标准输出，输出的是 JSON 格式的结果，用解析 JSON 的包解析一下就好了。

### 判断 runner 自身是否运行成功

看 runner 进程的退出值。  
如果 runner 自身都运行出错了，那这次判题就失败了。

### 解析结果

以下的值是 runner 提供的有关运行待判程序的信息：

```json
  "error_code": 0,
  "signal_code": 0,
  "exit_code": 0
```

其实只需要关注的就是 `error_code` 就好，代表 runner 告诉你为什么错了。其他两个值都是为了调试方便展示出来的，

`signal_code` 是导致子程序异常退出的信号值。
`exit_code` 是子程序的正常退出值（比如 exit(1））。

`error_code` 是根据 `signal_code` 和 `exit_code` 总结出来的错误，

比如如果 `error_code` 为 1，就表明没有找到要运行的用户程序。你就可以发现，当 `error_code` 为 1 的时候，`signal_code` 为 12，`exit_code` 为 0。

如果 `error_code` 为 0，大概率就说明本次判题执行成功了，这时候你就可以使用 status 的值当成本次的判题结果。
如果这几个 `*_code` 有不为 0 的，可以在判题日志中查看更多信息，看看到底是哪里错了。

这些 Code 的相关请看：[Code 一览](/every-code)

判题结果 status 含义请看：[Code 一览 - status](/every-code/#staus)

## 一些数据的单位

### 时间相关

其中 `cpu_time_used` 和 `real_time_used` 单位都是毫秒(ms)。
`cpu_time_used_us` 和 `real_time_used_us` 单位是微秒(us)。

### 内存相关

`memory_used` 单位是 kb。

log 中的也有一些数据，一般都把单位带上了再输出的。


## 在其他语言中调用

可以以启动子进程的方式来调用 `runner`，然后捕获控制台的输出即可。  

### Python

### Node.js

### Rust
