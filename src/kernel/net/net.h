#pragma once

// commit: deleted unix_sockets

//! (
//!  this file is half empty now :(
//! )


// gen net

#define _EAF_UNIX 1
#include <config/tasks.h>
#include <kernel/net/unix/unix_sock.h>
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