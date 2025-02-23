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

#### 封装寄存器类型信息
- 封装寄存器信息到registers_info.hpp，用宏定义（书里叫X-macro的宏）的方式将寄存器信息统一定义到registers.inc文件中，在register_info.hpp中直接include该文件来完成定义
- 除此之外，registers.inc中还定义了一些计算寄存器偏移的函数，以便后续读写寄存器时使用
- 在register_info.hpp中的g_register_infos数组全局保存了所有寄存器信息，可以通过register_info_by_xx函数按特定方式查询所需寄存器信息
- 新增types.hpp

#### 封装寄存器读写操作
- 与寄存器读写相关的库定义接口在sys/user.h中，其中struct user定义的寄存器的接口，我理解就是库会通过该结构体提供我们所需要的寄存器信息，我们的读写都需要通过该结构体定义来完成。struct user中有三个成员变量需要我们关注，regs/i387/u_debugreg分别封装了通用寄存器、浮点寄存器、debug专用寄存器的数据。
- 新增registers.hpp封装寄存器读写操作到registers类中，该类有一个user成员变量，用于保存当前user结构数据状态，读写都需更新到该user变量中。
- 集成registers到process类中：process类中新增registers成员完成寄存器读写，读写通过后面说到的ptrace请求来完成数据读写。
  - 读寄存器: 在process处于暂停状态时（此时主进程在waite_on_signal中），读取所有寄存器的信息。通过ptrace请求分别读取通用寄存器、浮点寄存器、debug寄存器数据，保存到register的user成员变量中。
  - 写寄存器：调用regs_write()完成写指定寄存器，需要注意处理以下issue：
    1. 写ah/bh/ch/dh等寄存器时会抛异常，原因是PTRACE_POKEUSER要求寄存器读写地址必须是8字节对齐，一次直接写8个字节。而这些高位寄存器是4字节对齐，所以ptrace请求抛异常。我们需要增加对齐处理，扩充成8字节读写。
    2. 写x87寄存器时会抛异常。作者说是PTRACE_POKEUSER/PTRACE_PEEKUSER在x86上不支持读写x87寄存器。所以改用PTRACE_SETFPREGS类型请求实现。
    3. 写的数据size小于寄存大小时需要特殊处理高位数据，和第1个问题有点类似，也是需要进行扩充数据处理。特别是当我们需要写入的数据是带符号的时候，需要注意高位要进行符号扩展。

#### 与寄存器管理相关的ptrace请求类型
- PTRACE_GETREGS & PTRACE_SETREGS：读/写通用寄存器，sys/user.h中的user_regs_struct定义
- PTRACE_GETFPREGS & PTRACE_SETFPREGS：读/写浮点寄存器(x87/MMX/SSE)，sys/user.h中的user_fpregs_struct定义
- PTRACE_POKEUSER: 写debug寄存器，或者写通用寄存器（准确定义其实是写用户空间数据，不过我们可以用来读写寄存器值）
- - PTRACE_PEEKUSER: 读debug寄存器



#### C++用法

- inline关键字：有些时候，当需要将函数/变量实现放在头文件中（比如register_info.hpp中的g_register_infos和register_info_by_xx），我们需要在定义前面加inline关键字，避免重定义错误（编译单元即使多次include头文件，也只会生成一次inline的函数/变量）
- std::variant：C++17引入，可以看作的类型安全的union，该类型保存值实际类型，当通过错误的类型方式访问时会抛出异常，而普通的union却不会。我们使用该类型封装寄存器读写操作返回类型。
- std::copy & std::memcopy主要区别：前者类型安全，后者不是。当需要拷贝的是非POD类型时，只能使用std::copy。
- 在构造函数中使用this指针：本章在process类中新增了一个register成员regs_，在process的构造函数中需要构造regs_，这里有一个有意思的细节，就是我们需要使用this指针来构造regs_，但是此时我们process还没有初始化完成，那么这里使用this指针是安全的吗？~~废话，都这么用了肯定安全~~ but why? because: 我们只是使用this来获取引用，而没有解引用this指针。前者安全是因为：此时已经完成process的内存分配，内存地址的是已经确定了的；后者不安全是因为：这块process的内存还未初始化，解引用访问内存内容是不安全的。总结：只是在构造函数中使用this指针或者引用是安全的，但是解引用访问还未初始化的内存内存就是不安全的。
- 编译的时候遇到一个的问题：process.cpp和registers.cpp相互include对方的头文件就会报错，但是registers和process有需要相互的类型定义，如何解决呢？其实registres的实现并不需要process类型的完整定义（只需要一个process引用），所以不用include process.hpp，只需要前向声明`class process;` 即可。
- if constexpr (std::is_floating_point_v<T>): [todo]