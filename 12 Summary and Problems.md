# Summary
# 小结

虚拟内存是对主存的一个抽象。支持虚拟内存的处理器通过使用一种叫做虚拟寻址的间接形式来引用主存。处理器产生一个虚拟地址，在被发送到主存之前，这个地址被翻译成一个物理地址。从虚拟地址空间到物理地址空间的地址翻译要求硬件和软件紧密合作。专门的硬件通过使用页表来翻译虚拟地址，而页表的内容是由操作系统提供的。

虚拟内存提供三个重要的功能。
 - 第一，它在主存中自动缓存最近使用的存放磁盘上的虚拟地址空间的内容。虚拟内存缓存中的块叫做页。对磁盘上页的引用会触发缺页，缺页将控制转移到操作系统中的一个缺页处理程序。缺页处理程序将页面从磁盘复制到主存缓存，如果必要，将写回被驱逐的页。
 - 第二，虚拟内存简化了内存管理，进而又简化了链接、在进程间共享数据、进程的内存分配以及程序加载。
 - 最后，虚拟内存通过在每条页表条目中加入保护位，从而了简化了内存保护。

地址翻译的过程必须和系统中所有的硬件缓存的操作集成在一起。大多数页表条目位于 L1 高速缓存中，但是一个称为 TLB 的页表条目的片上高速缓存，通常会消除访问在 L1 上的页表条目的开销。

现代系统通过将虚拟内存片和磁盘上的文件片关联起来，来初始化虚拟内存片，这个过程称为内存映射。内存映射为共享数据、创建新的进程以及加载程序提供了一种高效的机制。
应用可以使用 `mmap` 函数来手工地创建和删除虚拟地址空间的区域。

然而，大多数程序依赖于动态内存分配器，例如 malloc，它管理虚拟地址空间区域内一个称为堆的区域。
动态内存分配器是一个感觉像系统级程序的应用级程序，它直接操作内存，而无需类型系统的很多帮助。分配器有两种类型。显式分配器要求应用显式地释放它们的内存块。
隐式分配器（垃圾收集器）自动释放任何未使用的和不可达的块。

对于 C 程序员来说，管理和使用虚拟内存是一件困难和容易出错的任务。
常见的错误示例包括：间接引用坏指针，读取未初始化的内存，允许栈缓冲区溢岀，假设指针和它们指向的对象大小相同，引用指针而不是它所指向的对象，误解指针运算，引用不存在的变量，以及引起内存泄漏。


====================================================================================================

## 9.14
假设有一个输入文件 hello.txt，由字符串 “Hello, world!\n” 组成，编写一个 C 程序，使用 mmap 将 hello.txt 的内容改变为 “Jello, world! \n”。
~~~c
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>    // Include for file control constants
#include <unistd.h>   // Include for POSIX operating system API
#include <sys/mman.h> // Include for memory management declarations
#include <sys/stat.h> // Include for file status constants
#include <sys/types.h>

#define DEF_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH) // Define default file permissions
#define DEF_UMASK (S_IWGRP | S_IWOTH) // Define default umask value

// Function to test if the content of a file matches the expected content
void test(char* filename, char* content) {
  int fd;
  char buf[20];
  fd = open(filename, O_RDONLY); // Open the file in read-only mode
  read(fd, buf, strlen(content)); // Read the file content into buffer
  assert(!strncmp(buf, content, strlen(content))); // Assert that the content matches
  close(fd); // Close the file descriptor
}

// Function to create a new file or truncate an existing one and write content to it
int touch(char* filename, char* content) {
  int fd;
  umask(DEF_UMASK); // Set the file creation mask
  fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, DEF_MODE); // Open the file, create if not exist, and truncate to zero length
  write(fd, content, strlen(content)); // Write content to the file
  close(fd); // Close the file descriptor
}

int main(int argc, char* argv[]) {
  touch("hello.txt", "Hello, world!"); // Create 'hello.txt' with initial content
  test("hello.txt", "Hello, world!"); // Test the initial content of 'hello.txt'

  struct stat statbuf;
  int fd;
  char* bufp;
  size_t size;

  fd = open("hello.txt", O_RDWR); // Open 'hello.txt' in read-write mode
  fstat(fd, &statbuf); // Get file status
  size = statbuf.st_size; // Store the size of the file

  bufp = mmap(NULL, size, PROT_WRITE, MAP_SHARED, fd, 0); // Map file to memory
  *bufp = 'J'; // Modify the first character of the mapped file
  munmap(bufp, size); // Unmap the file from memory

  close(fd); // Close the file descriptor

  test("hello.txt", "Jello, world!"); // Test the modified content of 'hello.txt'
  return 0;
}
~~~


## 9.19
下面给出了三组关于内存管理和垃圾收集的陈述。在每一组中，只有一句陈述是正确的。你的任务就是判断哪一句是正确的。

a）在一个伙伴系统中，最高可达 50% 的空间可以因为内部碎片而被浪费了。√
b）首次适配内存分配算法比最佳适配算法要慢一些（平均而言）。×
c）只有当空闲链表按照内存地址递增排序时，使用边界标记来回收才会快速。×
d）伙伴系统只会有内部碎片，而不会有外部碎片。×

a）在按照块大小递减顺序排序的空闲链表上，使用首次适配算法会导致分配性能很低，但是可以避免外部碎片。×
b）对于最佳适配方法，空闲块链表应该按照内存地址的递增顺序排序。×
c）最佳适配方法选择与请求段匹配的最大的空闲块。×
d）在按照块大小递增的顺序排序的空闲链表上，使用首次适配算法与使用最佳适配算法等价。√

----------------------------------------------------------------------------------------------------
让我们逐一分析这四个陈述：

a）在按照块大小递减顺序排序的空闲链表上，使用首次适配算法会导致分配性能很低，但是可以避免外部碎片。【错误】

- 原因：首次适配算法在查找空闲块时选择第一个能满足需求的块。如果空闲链表是按块大小递减排序的，首次适配算法可能会快速找到一个足够大的块，但这并不一定导致低效的分配性能。此外，任何动态内存分配算法都无法完全避免外部碎片。

b）对于最佳适配方法，空闲块链表应该按照内存地址的递增顺序排序。【错误】

- 原因：最佳适配算法的目的是找到最小的足够大的空闲块，以最大程度减少浪费。这种方法的链表排序通常是按照块的大小，而不是内存地址的顺序。按地址排序对于最佳适配算法来说并不是必要的。

c）最佳适配方法选择与请求段匹配的最大的空闲块。【错误】

- 原因：实际上，最佳适配算法选择的是最小的足够大的块，而不是最大的块。其目标是尽量减少剩余空闲空间的大小，从而减少碎片。

d）在按照块大小递增的顺序排序的空闲链表上，使用首次适配算法与使用最佳适配算法等价。【正确】

- 原因：如果空闲链表按块大小递增排序，那么首次适配算法找到的第一个足够大的块实际上就是最小的能满足需求的块，这与最佳适配算法的目标一致。因此，在这种特定情况下，两种算法是等价的。
----------------------------------------------------------------------------------------------------

Mark&Sweep 垃圾收集器在下列哪种情况下叫做保守的：
a）它们只有在内存请求不能被满足时才合并被释放的内存。×
b）它们把一切看起来像指针的东西都当做指针。√
c）它们只在内存用尽时，才执行垃圾收集。×
d）它们不释放形成循环链表的内存块。×

## 9.17
开发 9.9.12 节中的分配器的一个版本，执行下一次适配搜索，而不是首次适配搜索。
~~~c
static void *find_fit(size_t asize) 
{
    static void *last_bp = NULL;  // Static pointer to remember the last position
    void *bp;

    // If last_bp is not set (NULL), start from the beginning
    if (last_bp == NULL) {
        last_bp = heap_listp;
    }

    // Next-fit search starting from last_bp
    for (bp = last_bp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
            last_bp = bp; // Update last_bp for the next search
            return bp;
        }
    }

    // If no fit is found, search from the beginning of the heap to last_bp
    for (bp = heap_listp; bp < last_bp; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
            last_bp = bp; // Update last_bp for the next search
            return bp;
        }
    }

    // If still no fit, return NULL and reset last_bp
    last_bp = NULL;
    return NULL; /* No fit */
}
~~~