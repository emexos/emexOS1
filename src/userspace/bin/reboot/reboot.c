#include <emx/system.h>
#include <unistd.h>

int main(void) {
    reboot(RSYSTEM_CMD_RESTART);
    _exit(1);
}