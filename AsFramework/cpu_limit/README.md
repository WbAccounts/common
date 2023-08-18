## CPU限速
***

### 功能说明
控制CPU使用率
***

### 说明

1. actor.h & process.h & actor.cpp & process.cpp 不再使用；
2. 但为兼容以前版本的相关编译工作，不对这几个文件做相关处理；
3. 由此引申出一个讨论，工具类要以一种什么方式进行集成开发？
4. 我们已形成结论，但是需要在实践工程过程中至少保证两点：
   （1）屏蔽外层调用对底层实现细节的直接操作；
   （2）上层封装的向下兼容性设计；

### 使用方法

#### 参数输入
```
input argv: ./CPULimitTest 
 -n <thread concurrence numbers> 
 -s <speed mode>
```

#### 速度调控
通过发送信号进行速度调控：（1）SIGUSR1 -> 100% （2）SIGSEGV -> 25% （3）SIGUSR2 -> 50%

***
### CHANGELOG
1. 2020.08.13重构cpu_limit，去除Actor和ProcessEx的无用设计；