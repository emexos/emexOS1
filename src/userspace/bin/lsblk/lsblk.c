#include <stdio.h>
#include <dirent.h>

int main(void) {
    struct dirent *entry;
    DIR *dp = opendir("/sys/block");
    if (dp == NULL) {
        perror("lsblk");
        return 1;
    }

    printf("NAME\n");
    while ((entry = readdir(dp))) {
        if (entry->d_name[0] == '.') continue;
        printf("%s\n", entry->d_name);
    }

    closedir(dp);
    return 0;
}