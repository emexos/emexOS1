#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#include <emx/sinfo.h>

static void show_uptime(unsigned long uptime_secs, int *h, int *m, int *d, int *s)
{
    *h =  uptime_secs 	/ 3600;
    *m = (uptime_secs 	% 3600) / 60;
    *s =  uptime_secs 	% 60;
    *d =  uptime_secs 	/ 86400;
}

static void get_username(char *out, size_t size)
{
    DIR *dir = opendir("/home");
    struct dirent *ent;

    if (!dir)
    {
        strncpy(out, "unknown", size);
        return;
    }

    while ((ent = readdir(dir)) != NULL)
    {
        if (
        	strcmp(ent->d_name, "guest") != 0 &&
            strcmp(ent->d_name, ".") != 0 &&
            strcmp(ent->d_name, "..") != 0
        ) {
            strncpy(out, ent->d_name, size);
            closedir(dir);
            return;
        }
    }

    closedir(dir);
    strncpy(out, "unknown", size);
}

int main(void)
{
    struct sysinfo_t info;
    emx_sinfo_t emx;

    char user[32];

    sysinfo(&info);
    emx_sinfo(&emx);

    get_username(user, sizeof(user));

    int h, m, d, s;
    show_uptime(info.uptime, &h, &m, &d, &s);

    #define RESET   "\033[0m"
    #define WHITE   "\033[37m"
    #define LPURPLE "\033[38;2;177;170;231m"
    #define PURPLE  "\033[34m"

    const char *colors[] =
    {
        "\033[31m", "\033[32m", "\033[33m",
        "\033[34m", "\033[35m", "\033[36m",
        "\033[91m", "\033[92m", "\033[93m", "\033[94m"
    };


    printf(LPURPLE" ##########;m; "RESET WHITE"    USER"PURPLE" %s\n"      , user);
    printf(LPURPLE" # ########;m; "RESET WHITE"      OS"PURPLE" %s\n"      , emx.sysname);
    printf(LPURPLE" # #;m;        "RESET WHITE"  KERNEL"PURPLE" %s\n"      , emx.release);
    printf(LPURPLE" # #;m;        "RESET WHITE"  UPTIME"PURPLE" %d:%02d:%02d, %d\n", h, m, s, d);
    printf(LPURPLE" # #######;m;  "RESET WHITE"   SHELL"PURPLE" eXsh\n");
    printf(LPURPLE" # #######;m;  "RESET WHITE"        "PURPLE"     \n");
    printf(LPURPLE" # #;m;        "RESET WHITE"        "PURPLE"     \n");
    printf(LPURPLE" # #;m;        "RESET WHITE"        "PURPLE"     \n");
    printf(LPURPLE" # ########;m; "RESET WHITE"  ");

    for (int i = 0; i < 5; i++)
    {
        printf("%s[=]%s ", colors[i], RESET);
    }
    printf("\n");

    printf(LPURPLE" ##########;m; "RESET WHITE"  ");

    for (int i = 5; i < 10; i++) {
        printf("%s[=]%s ", colors[i], RESET);
    }

    return 0;
}