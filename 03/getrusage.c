   #include <stdio.h>
   #include <sys/resource.h>

   int main() {
       struct rusage usage;
       getrusage(RUSAGE_SELF, &usage);

       printf("Minor faults: %ld\n", usage.ru_minflt);
       printf("Major faults: %ld\n", usage.ru_majflt);

       return 0;
   }