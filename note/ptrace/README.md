## 内容简介
文件夹主要介绍ptrace，主要源于对debugger的兴趣，参考如下链接：
1. [调试器工作原理](https://abcdxyzk.github.io/blog/2013/11/29/debug-debuger-1/)
2. [How Debuggers Works](https://eli.thegreenplace.net/2011/01/23/how-debuggers-work-part-1)
3. [Playing with ptrace](https://www.linuxjournal.com/article/6100)

## 文件介绍
1. [simple_tracer.c](./simple_tracer.c)是主程序，使用如下命令编译：
```shell
gcc -g -Wall -pedantic --std=c99 simple_tracer.c -o simple_tracer
```
2. [traced_hello.c](./traced_hello.c)是tracee，被追踪的程序，由于c程序需要系统依赖，使用主程序运行时，大部分显示libc里面的命令，使用如下命令编译：
```shell
gcc -g -Wall -pedantic --std=c99 traced_hello.c -o traced_hello
```
3. 由于上面的问题，使用汇编语言实现tracee，[traced_hello.s](./traced_hello.s)，使用如下命令编译：
```shell
nasm -f elf64 -o traced_hello.o traced_hello.s
ld traced_hello.o -o traced_hello
```

## ptrace
```c
#include <sys/ptrace.h>
long ptrace(enum __ptrace_rquest request, pid_t pid, void *addr, void *data);
```
## signal
```c
#include <sys/types.h>
#include <sys/wait.h>

pid_t wait(int *wstatus);
pid_t waitpid(pid_t pid, int *wstatus, int options);
int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);

// 使用如下宏判断wstatus
WIFEXITED(wstatus) // returns true if the child terminated normally, that is, by calling exit or _exit, or by returning from main()
WEXITSTATUS(wstatus)

WIFSIGNALED(wstatus) // returns true if the child process was terminated by a signal
WTERMSIG(wstatus)

WCOREDUMP(wstatus) // returns true if the child produced a core dump. This micro should be employed only if WIFSIGNALED returned true

WIFSTOPPED(wstatus) // 比如pstrace中，子进程使用pstrace的TRACE_ME，会触发TRACED信号给父进程，父进程使用这类宏
WSTOPSIG(wstatus)

WIFCONTINUED(wstatus) // returns true if the child process was resumed by delivery of SIGCONT
```
signal操作和修改默认动作，见[linux singal](https://man7.org/linux/man-pages/man7/signal.7.html)