# Judger

Origin: [1510460325/judge-runner](https://github.com/1510460325/judge-runner)

程序还参考了：

- [dojiong/Lo-runner](https://github.com/dojiong/Lo-runner/)
- [QingdaoU/Judger](https://github.com/QingdaoU/Judger)

获取程序运行时间和内存消耗。

程序运行结果会输出到指定的文件中。

结果：

```json
{ "status": "8", "timeUsed": "0", "memoryUsed": "2020" }
```

待实现 Feature：

- [ ] 优化获取真实执行时间和CPU执行时间

## 编译

```bash
make judge
```

## 测试

```bash
make test1
```

运行格式：

时间单位是 ms  
内存单位是 kb

```bash
./judge ./process time_limit_ms memory_limit_b input_file_path process_output_file_path result_file_path
```

举个例子：

```bash
./judge ./test 1000 2048 1.in 1.tmp.out 1.result
```
