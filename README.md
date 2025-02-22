# xdb
my xdebugger
参考《Building a Debugger (Early Access) by Sy Brand》一书，实现调试器

## chapter 1

本章本质上是教我们如何写现代化的cmake，可以作为使用cmake管理项目的模版进行学习

项目目录组织为：
- xdb/include/libdb: 公共头文件目录，里面包含提供给库用户的头文件
- xdb/src: 库实现目录
- xdb/src/include: 库实现内部头文件，这些头文件是库内部实现，不需要暴露给用户
- xdb/test: 测试目录
- xdb/tools: 命令行程序目录

#### 各目录中CMakefile主要写法
- xdb: add_subdirectory添加子目录
- xdb/src: 
  - add_library用于构建库
  - target_include_directories用于指定目标需要包含的头文件路径，这里有public和private关键字修饰的区别，private的含义是说只在当前直接目标target里添加需要包含头文件路径，而public则具有传递性，所有依赖当前目标的间接目标也会包含该头文件路径。
  具体来说，实际用法就是，暴露给用户的头文件路径需要是public的，而库内部实现的头文件路径需要是private。
  然后又介绍了 generator expression 的用法来优雅地处理用户通过库下载和源码编译两种方式需要包含的公共头文件位置不同的问题，通过 INSTALL_INTERFACE 指定当用户通过下载库的方式时需要包含的头文件路径，通过 BUILD_INTERFACE 指定用户通过源码编译的方式时需要包含的头文件路径
- xdb/test: 
  - 链接libxdb和Catch2
- xdb/tool: 
  - 链接libxdb和libedit

通过使用vcpkg的清单模式进行依赖库管理
- 参考文档：https://learn.microsoft.com/zh-cn/vcpkg/consume/manifest-mode?tabs=msbuild%2Cbuild-MSBuild


## chapter 2
  介绍编译调试的相关基础知识，无代码实现

## chapter 3

#### 主要系统调用

- ptrace: 是实现调试器的所依赖的最最重要的系统调用，它允许一个进程（通常是调试器）控制另一个进程的执行，读取其内存、寄存器等状态，甚至修改它们。
ptrace有多种类型，对应不同动作，当前需要用到一下几种：
  - ptrace(PTRACE_TRACEME, 0, nullptr, nullptr): 设置当前进程为被调试状态
  - ptrace(PTRACE_ATTACH, pid, nullptr, nullptr): 设置指定进程为被调试状态
  - ptrace(PTRACE_CONT, pid, nullptr, nullptr): 恢复被调试进程继续执行
- waitpid：阻塞当前进程，等待指定的子进程状态变化。每次需要改变被调试进程状态时（无论是暂停运行还是恢复运行），主进程都应该调用该函数等待被调试进程完成状态变化。

- kill: 给指定进程发送特定类型信号
当前用到的相关信号的作用：
  - kill(pid, SIGSTOP)：暂停指定进程执行
  - kill(pid, SIGCONT)：恢复指定进程执行，即继续之前暂停的操作
  - kill(pid, SIGKILL)：终止指定进程
- fork 和 exec

#### 调试主要流程
两种方式：
1. launch方式：启动调试时才创建被调试进程（xdb <binary path>）
调用fork创建子进程，在子进程中先调用ptrace(PTRACE_TRACEME)，然后再调用execlp运行被调试进程，此时进程启动时会进入到暂停运行状态
2. attach：调试一个已经启动的进程（xdb -p <pid>）
调用调用ptrace(PTRACE_ATTACH)设置允许向指定进程发送ptrace请求，同时会发送一个SIGSTOP给该进程，使其暂停运行

#### continue命令
被调试进程启动时，一开始都处于暂停运行状态，我们实现continue命令来完成恢复执行，简单来说就是通过调用ptrace(PTRACE_CONT)实现

#### Handling Errors
通过exception进行错误处理，继承std::runtime_error构造自定义exception

## chapter 4

为了方便测试：
1. 封装pipe类，通过pipe方式将被调试进程异常传回主进程，完成抛出异常的测试
2. 修改launch调试方式的实现，增加一个参数，用于测试被调试进程还未被trace的状态
3. 通过访问/proc/$(pid)/stat，获取进程的当前状态信息

## chapter 5

#### 本章实现
- 新增registers.hpp封装寄存器读写操作

- 新增registers_info.hpp封装寄存器信息
  - 用宏（书里叫X-macro的宏）来封装寄存器的定义，统一放到registers.inc文件中，然后register_info.hpp中直接通过#include的方式引用，这样不用每次都枚举一遍所有寄存器。
  - 除此之外，registers.inc中还定义了一些计算寄存器偏移的函数，以便后续读写寄存器时使用
  - 在register_info.hpp中的g_register_infos数组全局保存了所有寄存器信息，可以通过register_info_by_xx函数按特定方式查询所需寄存器信息
- 新增types.hpp

与寄存器管理相关的ptrace请求类型:
- PTRACE_GETREGS & PTRACE_SETREGS：读/写通用寄存器，sys/user.h中的user_regs_struct定义
- PTRACE_GETFPREGS & PTRACE_SETFPREGS：读/写浮点寄存器(x87/MMX/SSE)，sys/user.h中的user_fpregs_struct定义
- PTRACE_PEEKUSER: 读debug寄存器
- PTRACE_POKEUSER: 写debug寄存器，或者写通用寄存器



#### C++用法

#### inline用法
- 有些时候，当需要将函数/变量实现放在头文件中（比如register_info.hpp中的g_register_infos和register_info_by_xx），我们需要在定义前面加inline关键字，避免重定义错误（编译单元即使多次include头文件，也只会生成一次inline的函数/变量）
#### std::variant
- 使用该类型封装寄存器读写操作返回类型
