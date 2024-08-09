#include <unistd.h>
#include <stdio.h>

int main() {
    close(1);
    if (write(1, "h", 1) == -1)
        write(2, "Error\n", 6);
    return 1;
}