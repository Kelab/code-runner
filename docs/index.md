# 概览

这个程序是程序设计判题中的一个步骤，也就是运行用户程序(已经编译好的程序)，获取用户运行的结果。

## 快速开始

先看几个例子吧。

- `./judge -l echo.log -u echo.out echo 123`  
  运行 `echo 123` 这个程序，中途生成的日志写到 `echo.log` 这个文件中，运行结果输出到 `echo.out` 这个文件中。 
  ![运行结果](https://i.loli.net/2021/03/14/tPcryONFHsfJWmi.png)
  ![日志文件](https://i.loli.net/2021/03/14/8kHslmaZJiywEB2.png)

- `./judge -l ls.log -u ls.out -- ls -al`  
  跟上一句的意思差不多，这次执行的是 `ls -al`。  
  这次的需要执行的命令带有一个前置 `-`，所以我们要把命令放在 `--` 后，
  这样 `-` 就不会被认为是 `judge` 的参数了。
  ![运行结果](https://i.loli.net/2021/03/19/8jBZuodeKMzaEbi.png)

## 解释

可以看到，这个程序就是执行你给出的运行命令，然后输出一个 JSON 格式的结果，并且可以重定向你运行的命令的输入输出。

runner 的所有参数可见：[选项](./opts.md)

接下来我们定义一个名次：待判程序 -> 被运行的程序。

输出的 JSON 里有一些运行结果。

比如待判程序的 `exit_code`：

![exit code](https://i.loli.net/2021/03/19/SMOzWy9fIF47kw6.png)

比如 runner 内部检测到的本次运行出现的错误原因 `error_code`：

![error code](https://i.loli.net/2021/03/19/FCGcNsmTRk6zQte.png)

`error_code` 为 1 的时候就是说你要运行的命令在环境变量 `PATH` 里找不到。

全部 `error_code` 的值可见：[Code 一览 - error_code](/every-code/#error_code)

然后还有本次判题的结果：`status`，这个值的意思就是你的程序运行是对是错了。
还有一些收集到的信息，运行时间，使用的内存等。

好了，大概的东西都介绍完了，你可以点击下一页了。
