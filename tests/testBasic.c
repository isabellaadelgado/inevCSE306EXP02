#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("basic tests\n");
    int res = system("./encoder README.md README.md > output.txt");
    if (res != 0) {
        printf("failed\n");
        return 1;
    }
    printf("passed\n");
    return 0;
}
