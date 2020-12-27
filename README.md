# Judger

根据判题数据，判定用户程序的运行结果以及获取用户程序运行时间和内存消耗。

程序运行结果会输出到指定的文件中。

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

## 测试

```bash
make test1
```

## 运行

运行格式：

```bash
./judge process_path time_limit memory_limit input_path output_path tmp_output_path log_path
```

- process_path 用户程序地址
- time_limit 时间单位是 ms
- memory_limit 内存单位是 kb
- input_path 判题的标准输入文件位置
- output_path 判题的标准输出文件位置
- tmp_output_path 用户程序执行的标准输出位置（用于判断答案是否正确）
- log_path 日志文件位置

举个例子：

```bash
./judge ./test 1000 2048 1.in 1.out 1.tmp.out 1.log
```

之所以要多传入一个 `1.tmp.out` 是因为可以：1. 方便对程序执行结果进行判断，2. 不用把判题输出保留在内存中。

`1.log` 是本次判题的日志，方便调试。

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
#define AC 0 // Accepted
#define PRESENTATION_ERROR 1
#define TIME_LIMIT_EXCEEDED 2
#define MEMORY_LIMIT_EXCEEDED 3
#define WRONG_ANSWER 4
#define RUNTIME_ERROR 5
#define OUTPUT_LIMIT_EXCEEDED 6
#define CE 7           // Compile Error
#define SYSTEM_ERROR 8 // System Error
```

## 开源致谢

项目中使用到的开源库链接：

- [rxi/log.c](https://github.com/rxi/log.c)

项目开发过程中参考的项目：

- [1510460325/judge-runner](https://github.com/1510460325/judge-runner)
- [dojiong/Lo-runner](https://github.com/dojiong/Lo-runner/)
- [QingdaoU/Judger](https://github.com/QingdaoU/Judger)
