#pragma once

// gen net

#define _EU_UNIX 0
#define _EU_P 1
#include <config/tasks.h>
//#include <kernel/net/pipe/pipe.h>

// initial tasks
#ifndef SOCKET1
#   include <head.h>
#	define EITASK "1"
#elif SOCKET2
#	define EITASK "2"
#elif SOCKET3
#	define EITASK "3"
#elif SOCKET4
#	define EITASK "4"
#endif