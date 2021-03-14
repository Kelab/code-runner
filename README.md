# code-runner

一个运行用户程序的工具。

根据判题数据，判定用户程序的运行结果以及获取用户程序运行时间和内存消耗。

判题结果会以 JSON 格式输出到标准输出中。

```json
{
  "status": 0,
  "cpu_time_used": 2,
  "cpu_time_used_us": 1870,
  "real_time_used": 15,
  "real_time_used_us": 14849,
  "memory_used": 1516,
  "error_code": 0,
  "signal_code": 0,
  "exit_code": 0
}
```

## 文档

查看文档：<https://code-runner.kelab.dev>

## 致谢

项目中使用到的开源库链接：

- [rxi/log.c](https://github.com/rxi/log.c)

项目开发过程中参考的项目：

- [1510460325/judge-runner](https://github.com/1510460325/judge-runner)
- [dojiong/Lo-runner](https://github.com/dojiong/Lo-runner/)
- [QingdaoU/Judger](https://github.com/QingdaoU/Judger)

对以上项目表示感谢。
