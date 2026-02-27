#include <unistd.h>

int main(void)
{
    write(STDOUT_FILENO, "hello, world!\n", 14);
    return 0;
}
