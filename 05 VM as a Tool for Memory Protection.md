# VM as a Tool for Memory Protection
# 虚拟内存作为内存保护的工具

任何现代计算机系统必须为操作系统提供手段来控制对内存系统的访问。不应该允许一个用户进程修改它的只读代码段。而且也不应该允许它读或修改任何内核中的代码和数据结构。
不应该允许它读或者写其他进程的私有内存，并且不允许它修改任何与其他进程共享的虚拟页面，除非所有的共享者都显式地允许它这么做（通过调用明确的进程间通信系统调用）。
====================================================================================================
当然，让我通过一些具体的例子来说明这些进程间通信（IPC）方法的使用：

### 1. 管道（Pipes）

假设你有两个程序，一个生成数据，另一个处理这些数据。你可以通过管道将这两个程序连接起来。在 Unix-like 系统中，这可以通过 shell 命令实现：

```bash
# 使用管道将 'ls' 命令的输出传递给 'grep' 命令
ls | grep "example"
```

这个命令会列出当前目录下的文件，并将输出传递给 `grep` 命令来筛选包含 "example" 的文件名。

### 2. 消息队列（Message Queues）

假设你正在开发一个系统，其中一个进程负责收集数据，另一个进程负责数据处理。你可以使用消息队列来传递数据。
这样，数据收集进程可以将数据作为消息发送到队列，而数据处理进程可以从队列中读取这些消息。

```c
// C语言中使用消息队列的简化示例
// 这需要包含适当的头文件并链接相关的库
msgsnd(queue_id, &some_data, sizeof(some_data), 0); // 发送消息
msgrcv(queue_id, &some_data, sizeof(some_data), 0, 0); // 接收消息
```

### 3. 信号量（Semaphores）

在多个进程需要访问同一个资源（如打印机）时，可以使用信号量来确保每次只有一个进程可以使用该资源。

```c
// C语言中使用信号量的简化示例
sem_wait(&semaphore); // 等待信号量
// 执行对共享资源的操作
sem_post(&semaphore); // 释放信号量
```

### 4. 共享内存（Shared Memory）

如果你有一个大的数据集，需要由多个进程进行处理，可以使用共享内存。这样，所有进程都可以直接访问这个数据集，而不是通过慢速的进程间通信机制。

```c
// C语言中使用共享内存的简化示例
shared_memory_id = shmget(key, size, IPC_CREAT | 0666); // 创建或获取共享内存
shared_memory_ptr = shmat(shared_memory_id, NULL, 0); // 映射共享内存
// 使用 shared_memory_ptr 作为指向共享内存的指针
```

### 5. 套接字（Sockets）

可以在不同主机上运行的两个进程间使用套接字进行通信。例如，一个客户端程序可以通过套接字向服务器发送请求，并接收响应。

```c
// C语言中使用套接字的简化示例
sockfd = socket(AF_INET, SOCK_STREAM, 0); // 创建套接字
connect(sockfd, &server_address, sizeof(server_address)); // 连接到服务器
write(sockfd, "request", strlen("request")); // 向服务器发送请求
read(sockfd, buffer, sizeof(buffer)); // 读取来自服务器的响应
```

这些例子提供了基本的概念，但实际上的实现会更复杂，涉及到错误处理、资源管理等问题。
这些 IPC 方法是现代操作系统和网络通信中不可或缺的组成部分。
====================================================================================================

在这个示例中，每个 PTE 中已经添加了三个许可位。
`SUP` 位表示进程是否必须运行在内核（超级用户）模式下才能访问该页。运行在内核模式中的进程可以访问任何页面，但是运行在用户模式中的进程只允许访问那些 SUP 为 0 的页面。
`READ` 位和 `WRITE` 位控制对页面的读和写访问。
例如，如果进程 i 运行在用户模式下，那么它有读 VP 0 和读写 VP 1 的权限。然而，不允许它访问 VP 2。

如果一条指令违反了这些许可条件，那么 CPU 就触发一个一般保护故障，将控制传递给一个内核中的异常处理程序。Linux shell 一般将这种异常报告为“段错误（*segmentation fault*）”。

====================================================================================================
在页表项（`Page Table Entry, PTE`）中，有几个位是专门用于访问控制的。这些位决定了对应内存页的访问权限和行为。
以下是一些常见的与访问控制相关的位：

1. **有效位（Present/Valid Bit）**：
   - 这个位指示相关的页是否在物理内存中。如果该位为0，则表示该页不在物理内存中，可能在磁盘上（即页面未装载或已被交换出去）。

2. **读/写位（Read/Write Bit）**：
   - 这个位指示页是否可写。如果设置为1，表示页面可以被读取和写入；如果为0，则页面只能被读取。

3. **用户/超级用户位（User/Supervisor Bit）**：
   - 这个位控制着不同特权级别的访问权限。如果设置为1，表示用户模式和内核模式的进程都可以访问该页面；如果为0，则只有内核模式的进程（即操作系统）可以访问。

4. **访问位（Accessed Bit）**：
   - 操作系统使用这个位来跟踪页面是否被访问过。这个信息可以用于页面替换算法，如最近最少使用（LRU）算法。

5. **脏位（Dirty Bit）**：
   - 这个位指示页面自从被加载进物理内存以来是否被写入过。这对于页面写回策略很重要，因为只有“脏”的页面在被替换出物理内存时才需要被写回到磁盘。

6. **执行禁止位（No-Execute Bit）或执行保护位（Execute-Protection Bit）**：
   - 在一些体系结构中，这个位用于防止代码在非执行页面上运行，作为安全特性来防止诸如缓冲区溢出攻击的利用。

不同的操作系统和硬件架构可能会有些许差异，在具体的实现和命名上可能会有所不同。这些位在内存管理单元（MMU）处理虚拟地址到物理地址的转换时发挥着关键作用，同时也是操作系统实施内存保护和控制的基础。
