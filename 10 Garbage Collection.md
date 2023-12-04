# Garbage Collection
# 垃圾收集

在诸如 C malloc 包这样的显式分配器中，应用通过调用 malloc 和 free 来分配和释放堆块。应用要负责释放所有不再需要的已分配块。

未能释放已分配的块是一种常见的编程错误。例如，考虑下面的 C 函数，作为处理的一部分，它分配一块临时存储：

~~~c
void garbage()
{
    int *p = (int *)Malloc(15213);
    return; /* Array p is garbage at this point */
}
~~~

因为程序不再需要 p，所以在 garbage 返回前应该释放 p。
不幸的是，程序员忘了释放这个块。它在程序的生命周期内都保持为已分配状态，毫无必要地占用着本来可以用来满足后面分配请求的堆空间。

垃圾收集器（`garbage collector`）是一种动态内存分配器，它自动释放程序不再需要的已分配块。
这些块被称为垃圾（garbage）（因此术语就称之为垃圾收集器）。自动回收堆存储的过程叫做垃圾收集（garbagecollection）。
在一个支持垃圾收集的系统中，应用显式分配堆块，但是从不显示地释放它们。在 C 程序的上下文中，应用调用 malloc，但是从不调用 free。
反之，垃圾收集器定期识别垃圾块，并相应地调用 free，将这些块放回到空闲链表中。

垃圾收集可以追溯到 John McCarthy 在 20 世纪 60 年代早期在 MIT 开发的 Lisp 系统。它是诸如 Java、ML、Perl 和 Mathematica 等现代语言系统的一个重要部分，而且它仍然是一个重要而活跃的研究领域。有关文献描述了大量的垃圾收集方法，其数量令人吃惊。

我们的讨论局限于 McCarthy 独创的 `Mark&Sweep`（标记 & 清除）算法，这个算法很有趣，因为它可以建立在已存在的 malloc 包的基础之上，为 C 和 C++ 程序提供垃圾收集。

====================================================================================================
想象一下，你的房间里摆满了玩具，有些是你经常玩的，有些则很久没有碰过。McCarthy 独创的“标记 & 清除”（Mark & Sweep）算法就像是一种特殊的房间打扫方法，用来帮助你整理这些玩具。

1. **标记过程**：首先，你需要检查每个玩具，看看哪些是你最近使用过的。对于每个你发现的、经常玩的玩具，你就在上面贴一个标签。在计算机的世界里，这就像是“标记”阶段，其中程序会检查所有的内存对象，标记那些仍然被程序使用的对象。

2. **清除过程**：接下来，你会查看所有没有标签的玩具。这些没有标签的玩具就是你很久没有玩的，可能已经不再需要的。然后，你会把这些玩具收起来，腾出空间。在“清除”阶段，计算机会做类似的事情：它会查找所有未被标记的内存对象，并释放它们所占用的内存空间。

3. **结果**：最终，你的房间里只剩下那些有标签的、你仍然需要的玩具。其余的都被清理掉了，房间也因此变得更加整洁。在计算机中，经过标记和清除后，内存中只留下了仍在使用的对象，而不再需要的内存空间被清理出来，可以用于新的对象。

“标记 & 清除”算法的优点是它相对简单，而且不需要移动对象，这意味着对象的内存地址可以保持不变。不过，它的缺点是执行过程中可能需要暂停程序的运行（称为“`停止世界`”），并且可能会导致内存中出现碎片。尽管如此，这种方法是垃圾回收技术中的一个重要基石，对现代编程语言的内存管理产生了深远的影响。
====================================================================================================

## 垃圾收集器的基本知识

垃圾收集器将内存视为一张有向可达图（`reachability graph`），其形式如图 9-49 所示。该图的节点被分成一组根节点（root node）和一组堆节点（heap node）。
每个堆节点对应于堆中的一个已分配块。有向边 p→q 意味着块 p 中的某个位置指向块 q 中的某个位置。
根节点对应于这样一种不在堆中的位置，它们中包含指向堆中的指针。这些位置可以是寄存器、栈里的变量，或者是虚拟内存中读写数据区域内的全局变量。

当存在一条从任意根节点出发并到达 p 的有向路径时，我们说节点 p 是可达的（reachable）。在任何时刻，不可达节点对应于垃圾，是不能被应用再次使用的。垃圾收集器的角色是维护可达图的某种表示，并通过释放不可达节点且将它们返回给空闲链表，来定期地回收它们。

像 ML 和 Java 这样的语言的垃圾收集器，对应用如何创建和使用指针有很严格的控制，能够维护可达图的一种精确的表示，因此也就能够回收所有垃圾。
然而，诸如 C 和 C++ 这样的语言的收集器通常不能维持可达图的精确表示。这样的收集器也叫做*保守的垃圾收集器（conservative garbage collector）*。
从某种意义上来说它们是保守的，即每个可达块都被正确地标记为可达了，而一些不可达节点却可能被错误地标记为可达。(Soundness)
收集器可以按需提供它们的服务，或者它们可以作为一个和应用并行的独立线程，不断地更新可达图和回收垃圾。
例如，考虑如何将一个 C 程序的保守的收集器加入到已存在的 malloc 包中，如图 9-50 所示。

https://hansimov.gitbook.io/csapp/part2/ch09-virtual-memory/9.10-garbage-collection

无论何时需要堆空间时，应用都会用通常的方式调用 malloc。如果 malloc 找不到一个合适的空闲块，那么它就调用垃圾收集器，希望能够回收一些垃圾到空闲链表。收集器识别出垃圾块，并通过调用 free 函数将它们返回给堆。
**关键的思想是收集器代替应用去调用 free。**
当对收集器的调用返回时，malloc 重试，试图发现一个合适的空闲块。如果还是失败了，那么它就会向操作系统要求额外的内存。
最后，malloc 返回一个指向请求块的指针（如果成功）或者返回一个空指针（如果不成功）。

## Mark&Sweep 垃圾收集器

Mark&Sweep 垃圾收集器由标记（mark）阶段和清除（sweep）阶段组成，标记阶段标记出根节点的所有可达的和已分配的后继，而后面的清除阶段释放每个未被标记的已分配块。块头部中空闲的低位中的一位通常用来表示这个块是否被标记了。

我们对 Mark&Sweep 的描述将假设使用下列函数，其中 ptr 定义为 `typedef void* ptr`：

- `ptr isPtr (ptr p)` 如果 p 指向一个已分配块中的某个字，那么就返回一个指向这个块的起始位置的指针 b。否则返回 NULL。
- `int blockMarked(ptr b)` 如果块 b 是已标记的，那么就返回 true。
- `int blockAllocated (ptr b)` 如果块 b 是已分配的，那么就返回 true。
- `void markBlock (ptr b)` 标记块 b。
- `int length (b)` 返回块 b 的以字为单位的长度(不包括头部)
- `void unmarkBlock (ptr b)` 将块 b 的状态由已标记的改为未标记的。
- `ptr nextBlock (ptr b)` 返回堆中块 b 的后继。

标记阶段为每个根节点调用一次图 9-51a 所示的 mark 函数。如果 p 不指向一个已分配并且未标记的堆块，mark 函数就立即返回。否则，它就标记这个块，并对块中的每个字递归地调用它自己。每次对 mark 函数的调用都标记某个根节点的所有未标记并且可达的后继节点。在标记阶段的末尾，任何未标记的已分配块都被认定为是不可达的，是垃圾，可以在清除阶段回收。

~~~c
// mark 函数
void mark(ptr p) {
    if ((b = isPtr(p)) == NULL)
        return;
    if (blockMarked(b))
        return;
    markBlock(b);
    len = length(b);
    for (i = 0; i < len; i++)
        mark(b[i]);
    return;
}
~~~

清除阶段是对图 9-51b 所示的 sweep 函数的一次调用。sweep 函数在堆中每个块上反复循环，释放它所遇到的所有未标记的已分配块（也就是垃圾）。

~~~c
void sweep(ptr b, ptr end) {
    while (b < end) {
        if (blockMarked(b))
            unmarkBlock(b);
        else if (blockAllocated(b))
            free(b);
        b = nextBlock(b);
    }
    return;
}
~~~

##  C 程序的保守 `Mark&Sweep`
Mark&Sweep 对 C 程序的垃圾收集是一种合适的方法，因为它可以就地工作，而不需要移动任何块。
然而，C 语言为 `isPtr` 函数的实现造成了一些有趣的挑战。

> - `ptr isPtr (ptr p)` 如果 p 指向一个已分配块中的某个字，那么就返回一个指向这个块的起始位置的指针 b。否则返回 NULL。

第一，C 不会用任何类型信息来标记内存位置。因此，对 isPtr 没有一种明显的方式来判断它的输入参数 p 是不是一个指针。
第二，即使我们知道 p 是一个指针，对 isPtr 也没有明显的方式来判断 p 是否指向一个已分配块的有效载荷中的某个位置。

对后一问题的解决方法是将已分配块集合维护成一棵`平衡二叉树`，这棵树保持着这样一个属性：左子树中的所有块都放在较小的地址处，而右子树中的所有块都放在较大的地址处。
如图 9-53 所示，这就要求每个已分配块的头部里有两个附加字段（left 和 right）。每个字段指向某个已分配块的头部。
`isPtr(ptr p)` 函数用树来执行对已分配块的二分查找。在每一步中，它依赖于块头部中的大小字段来判断 p 是否落在这个块的范围之内。

~~~shell
    已分配块头部
[size|Left|Right| 块剩余部分]
      /        \
~~~

平衡树方法保证会标记所有从根节点可达的节点，从这个意义上来说它是正确的。这是一个必要的保证，因为应用程序的用户当然不会喜欢把他们的已分配块过早地返回给空闲链表。然而，这种方法从某种意义上而言又是保守的，因为它可能不正确地标记实际上不可达的块，因此它可能不会释放某些垃圾。虽然这并不影响应用程序的正确性，但是这可能导致不必要的外部碎片。

C 程序的 Mark&Sweep 收集器必须是保守的，其根本原因是 C 语言不会用类型信息来标记内存位置。
因此，像 int 或者 float 这样的标量可以伪装成指针。例如，假设某个可达的已分配块在它的有效载荷中包含一个 int，其值碰巧对应于某个其他已分配块 b 的有效载荷中的一个地址。
对收集器而言，是没有办法推断出这个数据实际上是 int 而不是指针。因此，分配器必须保守地将块 b 标记为可达，尽管事实上它可能是不可达的。

====================================================================================================
Java的垃圾回收（Garbage Collection, GC）机制是一种自动内存管理系统。它的主要任务是识别和回收程序不再使用的对象，以释放和重用内存。Java的GC实现相当复杂，因为它需要在释放不再使用的内存和最小化对程序性能影响之间找到平衡。以下是一个详细且专业的介绍：

### 基本概念

1. **堆内存**：Java中，大部分对象都在堆内存中分配。堆是一个运行时数据区域，从中分配所有类实例和数组。

2. **垃圾回收对象**：任何不再被应用程序中的任何对象引用的对象都被视为垃圾。

### 垃圾回收过程

Java的GC过程可以分为几个阶段：

1. **标记**：GC首先标记所有从根集合（包括活动线程、静态字段、本地变量等）可达的对象。可达对象被认为是“活着的”，而不可达对象被视为垃圾。

2. **清除**：在标记阶段之后，GC将清除所有未标记的对象。

3. **压缩**（可选）：为了解决内存碎片问题，某些GC算法会进行压缩阶段，移动对象以使得空闲内存空间连续。

### GC算法

Java提供了多种GC算法，每种都有其特点：

1. **串行垃圾回收器（Serial GC）**：适用于小型应用和有限的资源。它在单个线程中执行，适合单核处理器。

2. **并行垃圾回收器（Parallel GC）**：默认的GC方式。并行处理GC任务，提高了性能，但在垃圾回收时，会暂停所有应用程序线程（Stop-The-World）。

3. **并发标记清除（CMS）**：旨在减少GC引起的停顿。它在应用程序线程运行的同时执行大部分GC工作，但其缺点是会产生更多的内存碎片。

4. **G1垃圾回收器（Garbage-First）**：面向大型堆内存和多处理器机器，尝试平衡吞吐量和停顿时间。G1将堆分成多个区域，并优先回收价值最大的区域。

5. **ZGC和Shenandoah**：这些是相对较新的GC算法，设计用于减少停顿时间，适用于大型堆和多核心机器。

### 性能考虑

在选择和配置GC算法时，需要考虑以下因素：

- **停顿时间 vs 吞吐量**：减少GC停顿时间（使应用程序更平滑）和提高吞吐量（最大化应用程序运行时间）之间的权衡。

- **堆大小和可用内存**：堆的大小直接影响GC的性能。

- **应用程序特性**：不同的应用程序有不同的内存分配和回收模式。

Java的垃圾回收机制不断进化，以适应不断变化的应用需求和硬件条件。理解和正确配置GC对于优化Java应用程序的性能至关重要。
====================================================================================================