# Common Memory-Related Bugs in C Programs
# C 程序中常见的与内存有关的错误

对 c 程序员来说，管理和使用虚拟内存可能是个困难的、容易出错的任务。与内存有关的错误属于那些最令人惊恐的错误，因为它们在时间和空间上，经常在距错误源一段距离之后才表现出来。将错误的数据写到错误的位置，你的程序可能在最终失败之前运行了好几个小时，且使程序中止的位置距离错误的位置已经很远了。我们用一些常见的与内存有关错误的讨论，来结束对虚拟内存的讨论。

----------------------------------------------------------------------------------------------------
*1. 间接引用坏指针* `Dereferencing Bad Pointers`

正如我们在 9.7.2 节中学到的，在进程的虚拟地址空间中有较大的洞，没有映射到任何有意义的数据。
如果我们试图间接引用一个指向这些洞的指针，那么操作系统就会以段异常中止程序。而且，虚拟内存的某些区域是只读的。
试图写这些区域将会以保护异常中止这个程序。

间接引用坏指针的一个常见示例是经典的 scanf 错误。
假设我们想要使用 scanf 从 stdin 读一个整数到一个变量。正确的方法是传递给 scanf 一个格式串和变量的地址：
~~~c
scanf("%d", &val)
~~~

然而，对于 C 程序员初学者而言（对有经验者也是如此！），很容易传递 val 的内容，而不是它的地址：
~~~c
scanf("%d", val)
~~~
在这种情况下，scanf 将把 val 的内容解释为一个地址，并试图将一个字写到这个位置。在最好的情况下，程序立即以异常终止。
在最糟糕的情况下，val 的内容对应于虚拟内存的某个合法的读/写区域，于是我们就覆盖了这块内存，这通常会在相当长的一段时间以后造成灾难性的、令人困惑的后果。

这段描述涉及到计算机内存管理中的一个常见错误：间接引用坏指针。

1. 虚拟地址空间和“洞”
首先，了解进程的虚拟地址空间是很重要的。在现代操作系统中，每个进程都有自己的虚拟地址空间，这是一个连续的内存地址区域，操作系统和硬件通过内存管理单元（MMU）将这些虚拟地址映射到物理内存地址。虚拟地址空间通常包括用于程序代码、数据、堆（用于动态分配内存）和栈（用于函数调用和局部变量）的区域。
在这些区域之间，可能存在大块未被使用的地址空间，这些空间通常称为“洞”。这些“洞”不映射到任何物理内存或者任何有意义的数据。

2. 间接引用坏指针
当程序试图访问一个指向这些“洞”的指针时，就发生了间接引用坏指针的错误。因为这些地址没有映射到实际的物理内存，所以操作系统无法找到相应的数据来完成操作。结果，操作系统将以“`段异常`”（Segmentation Fault）中止程序。段异常是一种保护机制，用于防止程序访问无效或未授权的内存区域。

3. 试图写入只读区域
虚拟内存中还有一些区域是只读的，比如存放程序代码的区域。这些区域被标记为只读，是为了防止程序意外或恶意地修改其内容，这可能导致程序行为异常或安全漏洞。
如果程序试图写入这些只读区域，操作系统会以“`保护异常`”中止程序。保护异常是另一种安全机制，确保程序不会违反内存访问权限。

### 1. Null Pointer Dereference
```c
#include <stdio.h>

int main() {
    int *ptr = NULL; // Null pointer
    *ptr = 3;        // Dereferencing null pointer - undefined behavior, likely to cause a segmentation fault
    return 0;
}
```

### 2. Memory Leak
```c
#include <stdlib.h>

int main() {
    int *ptr = malloc(sizeof(int)); // Memory allocation
    *ptr = 5;                       // Using allocated memory
    // Forgot to free the memory
    return 0; // Memory leak happens here as the allocated memory is not freed
}
```

### 3. Buffer Overflow
```c
#include <string.h>

int main() {
    char buffer[10];
    strcpy(buffer, "This is a very long string, definitely longer than 10 characters."); // Buffer overflow
    return 0;
}
```

### 4. Using Uninitialized Memory
```c
#include <stdio.h>

int main() {
    int a;          // Uninitialized variable
    printf("%d\n", a); // Using uninitialized variable - undefined behavior
    return 0;
}
```

### 5. Double Free
```c
#include <stdlib.h>

int main() {
    int *ptr = malloc(sizeof(int)); // Memory allocation
    free(ptr); // First free
    free(ptr); // Second free on the same pointer - undefined behavior, likely to cause a program crash
    return 0;
}
```

### 6. Dangling Pointer
```c
#include <stdlib.h>

int main() {
    int *ptr = malloc(sizeof(int)); // Memory allocation
    *ptr = 4; // Using allocated memory
    free(ptr); // Free memory
    *ptr = 5; // Dangling pointer usage - undefined behavior, can cause a crash or data corruption
    return 0;
}
```
----------------------------------------------------------------------------------------------------
*2. 读未初始化的内存* `Reading Uninitialized Memory`

虽然 bss 内存位置（诸如未初始化的全局 C 变量）总是被加载器初始化为零，但是对于堆内存却并不是这样的。
一个常见的错误就是假设堆内存被初始化为零：

~~~c
/* Return y = Ax */
int *matvec(int **A, int *x, int n)
{
    int i, j;
    
    int *y = (int *)Malloc(n * sizeof(int));
    
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
            y[i] += A[i][j] * x[j];
    return y;
}
~~~
在这个示例中，程序员不正确地假设向量 y 被初始化为零。正确的实现方式是显式地将 y[i] 设置为零，或者使用 calloc。

`calloc()` is a function in the C standard library that dynamically allocates memory on the heap. The name `calloc` stands for "contiguous allocation". This function is used to allocate memory for an array of elements of a certain size and initializes all bytes in the allocated storage to zero. 
The `calloc()` function has two distinct features that differentiate it from `malloc()`, another memory allocation function:

1. **Initialization**: `calloc()` automatically initializes all allocated memory to zero. This is particularly useful when you need an array with all elements set to zero.

2. **Parameters**: `calloc()` takes two parameters: the number of elements and the size of each element. It then calculates the total amount of memory required by multiplying these two values.

The syntax of `calloc()` is:
```c
void* calloc(size_t num_elements, size_t element_size);
```

- `num_elements`: Number of elements to be allocated.
- `element_size`: Size of each element.

The function returns a pointer to the allocated memory, or `NULL` if the memory allocation fails.

### Example Usage
```c
#include <stdio.h>
#include <stdlib.h>

int main() {
    int n = 5;
    int *arr = (int*) calloc(n, sizeof(int));

    if (arr == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]); // Prints 0, as calloc initializes all elements to zero
    }

    free(arr); // Don't forget to free the memory
    return 0;
}
```

In this example, `calloc()` is used to allocate memory for an array of `5` integers and initialize them to `0`. It's important to note that even though `calloc()` initializes the memory, it's still necessary to `free()` the allocated memory once it's no longer needed to avoid memory leaks.

----------------------------------------------------------------------------------------------------
*3. 允许栈缓冲区溢出* `Allowing Stack Buffer Overflows`

正如我们在 3.10.3 节中看到的，如果一个程序不检查输入串的大小就写入栈中的目标缓冲区，那么这个程序就会有缓冲区溢出错误（buffer overflow bug）。
例如，下面的函数就有缓冲区溢出错误，因为 gets 函数复制一个任意长度的串到缓冲区。为了纠正这个错误，我们必须使用 fgets 函数，这个函数限制了输入串的大小：
~~~c
void bufoverflow()
{
    char buf[64];
    gets(buf); /* Here is the stack buffer overflow bug */
    return;
}
~~~
----------------------------------------------------------------------------------------------------
*4. 假设指针和它们指向的对象是相同大小的* `Assuming That Pointers and the Objects They Point to Are the Same Size`

一种常见的错误是假设指向对象的指针和它们所指向的对象是相同大小的：
~~~c
/* Create an nxm array */
int **makeArray1(int n, int m)
{
    int i;
    int **A = (int **)Malloc(n * sizeof(int));
    
    for (i = 0; i < n; i++)
        A[i] = (int *)Malloc(m * sizeof(int));
    return A;
}
~~~

这里的目的是创建一个由 n 个指针组成的数组，每个指针都指向一个包含 m 个 int 的数组。
然而，因为程序员在第 5 行将 sizeof(int *) 写成了 sizeof(int)，代码实际上创建的是一个 int 的数组。

这段代码只有`在 int 和指向 int 的指针大小相同`的机器上运行良好。
但是，如果我们在像 Core i7 这样的机器上运行这段代码，其中指针大于 int，那么第 7 行和第 8 行的循环将写到超出 A 数组结尾的地方。
因为这些字中的一个很可能是已分配块的边界标记脚部，所以我们可能不会发现这个错误，直到在这个程序的后面很久释放这个块时，此时，分配器中的合并代码会戏剧性地失败，而没有任何明显的原因。

这是“在远处起作用（`action at distance`）”的一个阴险的示例，这类“在远处起作用”是与内存有关的编程错误的典型情况。

The code provided demonstrates a common mistake in C programming related to pointer sizes. The error is in the line where the memory is allocated for the array of pointers `A`. Let's break down the issue and correct it:

### Error in the Code
```c
int **A = (int **)Malloc(n * sizeof(int));
```
In this line, the size of an `int` is used for allocating memory for an array of `int *` (pointer to `int`). However, the size of a pointer (`int *`) may not be the same as the size of an `int`. This is especially true on systems where `int` is 32 bits and pointers are 64 bits.

### Corrected Code
The correct approach is to allocate memory based on the size of the pointer type, not the type that the pointer points to:
```c
int **A = (int **)Malloc(n * sizeof(int *));
```

### Complete Corrected Function
```c
#include <stdlib.h> // For malloc

/* Create an nxm array */
int **makeArray1(int n, int m)
{
    int i;
    // Allocate an array of pointers
    int **A = (int **)malloc(n * sizeof(int *)); // Corrected size

    if (A == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    for (i = 0; i < n; i++) {
        A[i] = (int *)malloc(m * sizeof(int)); // Allocate memory for each row
        if (A[i] == NULL) {
            // Handle memory allocation failure for a row
            // Cleanup previously allocated rows before returning
            for (int j = 0; j < i; j++) {
                free(A[j]);
            }
            free(A);
            return NULL;
        }
    }
    return A;
}
```

In this corrected version, memory is properly allocated for an array of `int *` pointers. Additionally, the code now includes checks for `malloc` failure, which is a good practice to avoid potential issues with memory allocation errors. If memory allocation fails at any point, the function cleans up any previously allocated memory before returning `NULL`.

----------------------------------------------------------------------------------------------------
*5. 造成错位错误* `Making Off-by-One Errors`

错位（off-by-one）错误是另一种很常见的造成覆盖错误的来源：
~~~c
/* Create an nxm array */
int **makeArray2(int n, int m)
{
    int i;
    int **A = (int **)Malloc(n * sizeof(int *));
    
    for (i = 0; i <= n; i++) // i < n
        A[i] = (int *)Malloc(m * sizeof(int));
    return A;
}
~~~
这是前面一节中程序的另一个版本。这里我们在第 5 行创建了一个 n 个元素的指针数组，但是随后在第 7 行和第 8 行试图初始化这个数组的 n+1 个元素，在这个过程中覆盖了 A 数组后面的某个内存位置。

----------------------------------------------------------------------------------------------------
*6. 引用指针，而不是它所指向的对象* `Referencing a Pointer Instead of the Object It Points To`

如果不太注意 C 操作符的优先级和结合性，我们就会错误地操作指针，而不是指针所指向的对象。
比如，考虑下面的函数，其目的是删除一个有  `*size` 项的二叉堆里的第一项，然后对剩下的 `*size-1` 项重新建堆：
~~~c
int *binheapDelete(int **binheap, int *size)
{
    int *packet = binheap[0];
    
    binheap[0] = binheap[*size - 1];
    *size--; /* This should be (*size)-- */
    heapify(binheap, *size, 0);
    return (packet);
}
~~~

在第 6 行，目的是减少 size 指针指向的整数的值。然而，因为一元运算符——和 * 的优先级相同，从右向左结合，所以第 6 行中的代码实际减少的是指针自己的值，而不是它所指向的整数的值。
如果幸运地话，程序会立即失败；但是更有可能发生的是，当程序在执行过程后很久才产生出一个不正确的结果时，我们只有一头的雾水。
这里的原则是当你对优先级和结合性有疑问的时候，就使用括号。
比如，在第 6 行，我们可以使用表达式 `(*size)--`，清晰地表明我们的意图。

----------------------------------------------------------------------------------------------------
*7. 误解指针运算* `Misunderstanding Pointer Arithmetic`

另一种常见的错误是忘记了指针的算术操作是以它们指向的对象的大小为单位来进行的，而这种大小単位并不一定是字节。
例如，下面函数的目的是扫描一个 int 的数组，并返回一个指针，指向 val 的首次出现
~~~c
int *search(int *p, int val)
{
    while (*p && *p != val)
        p += sizeof(int); /* Should be p++ */
    return p;
}
~~~
然而，因为每次循环时，第 4 行都把指针加了 4（一个整数的字节数），函数就不正确地扫描数组中每 4 个整数。

----------------------------------------------------------------------------------------------------
*8. 引用不存在的变量* `Referencing Nonexistent Variables`

没有太多经验的 C 程序员不理解栈的规则，有时会引用不再合法的本地变量，如下列所示：
~~~c
int *stackref ()
{
    int val;
    
    return &val;
}
~~~
这个函数返回一个指针（比如说是 p），指向栈里的一个局部变量，然后弹出它的栈帧。
尽管 p 仍然指向一个合法的内存地址，但是它已经不再指向一个合法的变量了。
当以后在程序中调用其他函数时，内存将重用它们的栈帧。再后来，如果程序分配某个值给 `*p`，那么它可能实际上正在修改另一个函数的栈帧中的一个条目，从而潜在地带来灾难性的、令人困惑的后果。

----------------------------------------------------------------------------------------------------
*9. 引用空闲堆块中的数据* `Referencing Data in Free Heap Blocks`

一个相似的错误是引用已经被释放了的堆块中的数据。
例如，考虑下面的示例，这个示例在第 6 行分配了一个整数数组 x，在第 10 行中先释放了块 x，然后在第 14 行中又引用了它：

~~~c
int *heapref(int n, int m)
{
    int i;
    int *x, *y;
    
    x = (int *)Malloc(n * sizeof(int));
.
. // Other calls to malloc and free go here
.
    free(x);
    
    y = (int *)Malloc(m * sizeof(int));
    for (i = 0; i < m; i++)
        y[i] = x[i]++; /* Oops! x[i] is a word in a free block */
        
    return y;
}
~~~
取决于在第 6 行和第 10 行发生的 malloc 和 free 的调用模式，当程序在第 14 行引用 `x[i]` 时，数组 x 可能是某个其他已分配堆块的一部分了，因此其内容被重写了。
和其他许多与内存有关的错误一样，这个错误只会在程序执行的后面，当我们注意到 y 中的值被破坏了时才会显现出来。

----------------------------------------------------------------------------------------------------
*10. 引起内存泄漏* `Introducing Memory Leaks`

内存泄漏是缓慢、隐性的杀手，当程序员不小心忘记释放已分配块，而在堆里创建了垃圾时，会发生这种问题。
例如，下面的函数分配了一个堆块 `x`，然后不释放它就返回：
~~~c
void leak(int n)
{
    int *x = (int *)Malloc(n * sizeof(int));
    return;  /* x is garbage at this point */
}
~~~

如果经常调用 leak，那么渐渐地，堆里就会充满了垃圾，最糟糕的情况下，会占用整个虚拟地址空间。
对于像守护进程和服务器这样的程序来说，内存泄漏是特别严重的，根据定义这些程序是不会终止的。

----------------------------------------------------------------------------------------------------
# sizeof usage

`sizeof` 是 C 和 C++ 语言中的一个操作符，用于获取对象或类型在内存中的大小。它返回的是以字节为单位的大小。
当 `sizeof` 用于指针时，它返回的是该指针类型在当前平台下的大小，而不是指针所指向的对象的大小。

### `sizeof` 指针

当对一个指针使用 `sizeof` 时，返回的是存储该指针所需的字节数。这个大小是与平台相关的，通常取决于操作系统和硬件的地址长度。例如，在 32 位系统上，指针的大小通常是 4 字节，而在 64 位系统上，指针的大小通常是 8 字节。

```c
int *p;
size_t size = sizeof(p); // 在 32 位系统上通常是 4，64 位系统上通常是 8
```

### `sizeof` 的用法

`sizeof` 可以用于数据类型或变量。以下是一些常见用法的例子：

1. **获取基本数据类型的大小**：
   ```c
   size_t intSize = sizeof(int); // 获取 int 类型的大小
   ```

2. **获取数组的总大小**：
   ```c
   int arr[10];
   size_t arrSize = sizeof(arr); // 获取整个数组的大小，结果是 10 * sizeof(int)
   ```

3. **获取数组单个元素的大小**：
   ```c
   size_t elemSize = sizeof(arr[0]); // 获取数组单个元素的大小
   ```

4. **获取结构体或联合体的大小**：
   ```c
   struct MyStruct {
       int a;
       double b;
   };
   size_t myStructSize = sizeof(struct MyStruct); // 获取结构体的大小
   ```

5. **在动态内存分配中使用**：
   ```c
   int *ptr = malloc(10 * sizeof(*ptr)); // 分配 10 个 int 的空间
   ```

### 注意事项

- `sizeof` 操作是在编译时进行的，不是在运行时。因此，`sizeof` 的参数不会被实际求值。
- 对于可变长度的数据结构（如 VLAs - Variable Length Arrays），`sizeof` 返回的是数组的实际分配大小，这是一个例外，其中 `sizeof` 是在运行时计算的。
- `sizeof` 返回的类型是 `size_t`，这是一个无符号整数类型。

`sizeof` 是理解和优化内存使用中一个非常重要的工具，特别是在涉及到跨平台编程和动态内存管理时。

----------------------------------------------------------------------------------------------------

在 C 语言中，数组名和指向该数组的指针之间的关系是一个经常被误解的概念。要理解这个关系，我们需要考虑几个关键的概念：

### 数组名作为指针

1. **数组名作为地址**：在大多数情况下，数组名被用作指向数组首元素的指针。例如，如果你有一个数组 `int arr[10];`，那么表达式 `arr` 就可以被看作是一个指针，指向 `arr[0]`。

2. **类型**：数组名作为指针的类型是指向数组元素类型的指针。在上面的例子中，`arr` 是 `int *` 类型。

3. **常量性**：虽然数组名可以被看作是指针，但它是一个常量指针，你不能改变它的值。例如，你不能对 `arr` 进行赋值操作。

### 数组名和指针的不同

尽管在很多情况下数组名可以当作指针来使用，但数组名和指针在本质上是不同的：

1. **内存分配**：数组名代表的是一个内存中连续分配的固定大小的元素序列。而指针是一个变量，它存储的是内存地址，可以被改变指向不同的地址。

2. **sizeof 操作符**：当使用 `sizeof` 时，数组名和指针的行为不同。对数组名使用 `sizeof` 会返回整个数组的大小（所有元素的大小总和），而对指针使用 `sizeof` 仅返回指针本身的大小。

3. **自增操作**：你可以对指针进行自增操作来改变它的值（即改变它指向的地址），但不能对数组名进行这样的操作。

### 示例

```c
#include <stdio.h>

int main() {
    int arr[10];
    int *ptr = arr; // ptr 现在指向 arr 的第一个元素

    printf("Size of arr: %zu\n", sizeof(arr));  // 输出整个数组的大小
    printf("Size of ptr: %zu\n", sizeof(ptr));  // 输出指针的大小

    ptr++; // 合法操作，指针移动到下一个 int
    // arr++; // 非法操作，不能改变数组名的值

    return 0;
}
```

在这个示例中，`arr` 是一个数组名，代表一个有 10 个 `int` 的数组。`ptr` 是一个指针，指向 `arr` 的第一个元素。注意 `sizeof(arr)` 和 `sizeof(ptr)` 的区别，以及对 `ptr` 和 `arr` 自增操作的合法性。

