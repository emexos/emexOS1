#pragma once

static inline void store_fence()
{
    __asm__ volatile("sfence" ::: "memory");
}