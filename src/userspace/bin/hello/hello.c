#include <unistd.h>
#include <stdio.h>

#include <emx/ansi.h>

int main(void)
{
    printf(A_GFX_GREEN "\nhello, world!\n\n");
    return 0;
}
