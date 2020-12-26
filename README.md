# Judger

Origin: [1510460325/judge-runner](https://github.com/1510460325/judge-runner)

程序还参考了：

- [Certseeds/CS309_dboj_judge_System](https://github.com/Certseeds/CS309_dboj_judge_System/blob/master/mysql/judge.cpp)
- [MrEdge123/Team-programming](https://github.com/MrEdge123/Team-programming/blob/main/webh/judgeModel/judgeCore.cpp)

获取程序运行时间和内存消耗。

程序运行结果会输出到指定的文件中。

结果：

```json
{ "status": "8", "timeUsed": "0", "memoryUsed": "2020" }
```

待实现 Feature：

- [ ] 优化获取结果的方式
- [ ] 优化 log
- [ ] 能获取到程序运行报错

## 编译

```bash
make judge
```

## 测试

```bash
make test1
```

运行格式：

```bash
./judge ./process time_limit memory_limit input_file_path process_output_file_path result_file_path
```

举个例子：

```bash
./judge ./test 100 2048 1.in 1.tmp.out 1.result
```
