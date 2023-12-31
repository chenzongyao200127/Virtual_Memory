# Address Spaces
# 地址空间

地址空间（addressspace）是一个非负整数地址的有序集合：
`{0,1,2, ... }`

如果地址空间中的整数是连续的，那么我们说它是一个线性地址空间（`linear address space`）。
为了简化讨论，我们总是假设使用的是线性地址空间。在一个带虚拟内存的系统中，CPU 从一个有 $N = 2^n$ 个地址的地址空间中生成虚拟地址，这个地址空间称为虚拟地址空间（`virtual addres sspace`）：
`{0,1,2, ... , N-1}`

一个地址空间的大小是由表示最大地址所需要的位数来描述的。例如，一个包含个$N = 2^n$地址的虚拟地址空间就叫做一个 *n* 位地址空间。现代系统通常支持 32 位或者 64 位虚拟地址空间。

一个系统还有一个物理地址空间（physical address space），对应于系统中物理内存的 M 个字节：
*{0,1,2, ... , M-1}*

M 不要求是 2 的幕，但是为了简化讨论，我们假设 `M = 2 ^ m`。

地址空间的概念是很重要的，因为它清楚地区分了数据对象（字节）和它们的属性（地址）。
一旦认识到了这种区别，那么我们就可以将其推广，允许每个数据对象有多个独立的地址，其中每个地址都选自一个不同的地址空间。这就是虚拟内存的基本思想。主存中的每字节都有一个选自虚拟地址空间的虚拟地址和一个选自物理地址空间的物理地址。

====================================================================================================
地址空间（Address Space）是一个计算机科学的概念，它指的是一个进程在内存中可用于寻址的所有地址的集合。在不同的上下文中，地址空间可以有不同的含义：

1. **物理地址空间**：
   - 这是指计算机物理内存中所有可用地址的集合。物理地址空间的大小由计算机的物理内存大小和寻址能力决定。例如，32位系统通常有4GB的物理地址空间。

2. **虚拟地址空间**：
   - 在现代操作系统中，每个进程通常都有自己的虚拟地址空间。这意味着进程看到的内存地址（虚拟地址）与实际的物理内存地址是分开的。
   - 虚拟地址空间通过操作系统和内存管理单元（MMU）的协作映射到物理地址空间。这样，每个进程都好像在使用它自己的独立内存一样，增加了安全性和稳定性。
   - 在32位系统中，虚拟地址空间通常也是4GB，而在64位系统中，虚拟地址空间可以非常大，远远超过实际物理内存的大小。

3. **用户空间和内核空间**：
   - 在虚拟地址空间中，通常会有用户空间和内核空间的划分。用户空间是用户级应用程序运行的地方，而内核空间是操作系统内核运行的地方。
   - 这种划分提高了系统的安全性和稳定性，因为用户程序不能直接访问或干扰内核空间。

4. **地址空间的管理**：
   - 地址空间的管理是操作系统内存管理的一个重要方面。操作系统负责分配和回收地址空间，管理进程之间的地址空间隔离，以及虚拟地址到物理地址的映射。

5. **地址空间的意义**：
   - 地址空间为进程提供了一种方法，使其能够在不与其他进程冲突的情况下访问内存。每个进程都有自己的地址空间，这意味着即使两个进程中的两个不同变量具有相同的虚拟内存地址，它们也会映射到不同的物理地址。

地址空间是操作系统设计中的一个核心概念，对于实现进程隔离、内存保护和有效的内存管理至关重要。
====================================================================================================