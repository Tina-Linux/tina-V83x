#include <stdlib.h>

int main(int argc, char *argv[])
{
    int *array = (int *)malloc(100 * sizeof(int));
    free(array);
    return array[argc]; /* BOOM */
}
