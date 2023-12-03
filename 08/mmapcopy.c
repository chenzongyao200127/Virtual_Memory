#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int fd;
    struct stat sb;
    char *p;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    // 打开文件
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    // 获取文件的属性
    if (fstat(fd, &sb) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    // 将文件映射到内存
    p = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (p == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    // 将文件内容写到标准输出
    if (write(STDOUT_FILENO, p, sb.st_size) != sb.st_size) {
        perror("write");
        munmap(p, sb.st_size);
        close(fd);
        return 1;
    }

    // 清理工作
    munmap(p, sb.st_size);
    close(fd);

    return 0;
}
