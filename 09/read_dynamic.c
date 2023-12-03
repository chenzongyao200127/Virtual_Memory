#include <stdio.h>
#include <stdlib.h>

int main()
{
    int *array, i, n;
    
    scanf("%d", &n);
    array = (int *)valloc(n * sizeof(int));
    for (i = 0; i < n; i++)
        scanf("%d", &array[i]);
    free(array);
    exit(0);
}