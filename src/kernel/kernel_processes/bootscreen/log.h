#ifndef LOG_H
#define LOG_H

#include <types.h>
#include <kernel/graph/theme.h>

// Log levels
typedef enum {
    LDEF = 0,
    LSUCCESS,
    LWARNING,
    LERROR
} log_level_t;

// Keywords for log levels
#define d       LDEF
#define success LSUCCESS
#define warning LWARNING
#define error   LERROR

// Log colors based on level
#define LCOLOR_TAG     green()//GFX_GRAY_70
#define LCOLOR_BRACKET GFX_GRAY_70
#define LCOLOR_D       white()
#define LCOLOR_SCS     white()//green()
#define LCOLOR_WAR     white()//yellow()
#define LCOLOR_ERR     red()
// optional macro
#define log(tag, message, ...) log_message(tag, message, _LOG_LEVEL(__VA_ARGS__))
#define _LOG_LEVEL(...) _LOG_LEVEL_IMPL(__VA_ARGS__, LDEF)
#define _LOG_LEVEL_IMPL(level, ...) level

void log_message(const char *tag, const char *message, log_level_t level);

//alias logf
void log_printf(log_level_t level, const char *tag, const char *format, ...);

#define logf(level, tag, format, ...) log_printf(level, tag, format, ##__VA_ARGS__)

#endif
