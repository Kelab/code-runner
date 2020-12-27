# Judger

Origin: [1510460325/judge-runner](https://github.com/1510460325/judge-runner)

程序还参考了：

- [dojiong/Lo-runner](https://github.com/dojiong/Lo-runner/)
- [QingdaoU/Judger](https://github.com/QingdaoU/Judger)

获取程序运行时间和内存消耗。

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
  "exit_code": 0,
  "err_number": 0
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

time_limit 时间单位是 ms  
memory_limit 内存单位是 kb

```bash
./judge ./process time_limit memory_limit input_file_path user_output_file_path result_file_path
```

举个例子：

```bash
./judge ./test 1000 2048 1.in 1.tmp.out 1.result
```
