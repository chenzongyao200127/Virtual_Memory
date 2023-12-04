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
