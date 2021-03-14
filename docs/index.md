# code-runner

这个程序是程序设计判题中的一个步骤，也就是编译好了用户提交的代码之后，要运行用户程序获取用户运行的结果。

## 快速开始

先看几个例子吧。

- `./judge -l echo.log -u echo.out echo 123`  
  运行 `echo 123` 这个程序，中途生成的日志写到 `echo.log` 这个文件中，运行结果输出到 `echo.out` 这个文件中。 
  ![运行结果](https://i.loli.net/2021/03/14/tPcryONFHsfJWmi.png)
  ![日志文件](https://i.loli.net/2021/03/14/8kHslmaZJiywEB2.png)

- `./judge -l ls.log -u ls.out -- ls -al`  
  跟上一句的意思差不多，这次执行的是 `ls -al`，这次的需要执行的命令带有一个前置 `-`，所以我们要把命令放在 `--` 后，
  这样 `-` 就不会被认为是 `judge` 的参数了。
