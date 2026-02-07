// macros
#if BOOTUP_VISUALS == 0 // verbose boot
    #define BOOTUP_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
    //--------------------------------------
    #define BOOTUP_PRINT(msg, col) \
        do { \
            print(msg, col); \
            if (init_boot_log >= 0) { \
                fs_write(init_boot_log, msg, str_len(msg)); \
            } \
        } while(0)
    //--------------------------------------
    #define BOOTUP_PRINT_INT(num, col) \
        do { \
            printInt(num, col); \
            if (init_boot_log >= 0) { \
                char buf[12]; \
                buf[0] = '\0'; \
                str_append_uint(buf, (u32)num); \
                fs_write(init_boot_log, buf, str_len(buf)); \
            } \
        } while(0)
    //--------------------------------------
    #define BOOTUP_PRINTBS(msg, col) \
        do { \
            printbs(msg, col); \
            if (init_boot_log >= 0) { \
                fs_write(init_boot_log, msg, str_len(msg)); \
            } \
        } while(0)
    //--------------------------------------

#else     // silent boot
    #define BOOTUP_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
    //--------------------------------------
    #define BOOTUP_PRINT(msg, col) \
        do { \
            if (init_boot_log >= 0) { \
                fs_write(init_boot_log, msg, str_len(msg)); \
            } \
        } while(0)
    //--------------------------------------
    #define BOOTUP_PRINT_INT(num, col) \
        do { \
            if (init_boot_log >= 0) { \
                char buf[12]; \
                buf[0] = '\0'; \
                str_append_uint(buf, (u32)num); \
                fs_write(init_boot_log, buf, str_len(buf)); \
            } \
        } while(0)
    //--------------------------------------
    #define BOOTUP_PRINTBS(msg, col) \
        do { \
            if (init_boot_log >= 0) { \
                fs_write(init_boot_log, msg, str_len(msg)); \
            } \
        } while(0)
    //--------------------------------------

#endif
