#include <stdio.h>

int globalVar = 0; // 全局变量，将在数据段中

void printHello() {
    printf("Hello, World!\n"); // 函数，将在代码段中
}

int main() {
    printHello();
    globalVar = 5;
    printf("Global variable value: %d\n", globalVar);
    return 0;
}