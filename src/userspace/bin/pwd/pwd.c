#include <stdio.h>
#include <unistd.h>

int main(void) {
    char buf[256];
    if (!getcwd(buf, sizeof(buf))) {
        printf("/\n");
        return 1;
    }
    printf("%s\n", buf);
    return 0;
}