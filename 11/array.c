
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

// czy@czy-307-thinkcentre-m720q-n000:~/new_space/Virtual_Memory/11 (branch: master!)
// $ ./array 
// Size of arr: 40
// Size of ptr: 8