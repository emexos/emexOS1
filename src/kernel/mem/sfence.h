#pragma once

static inline void store_fence()
{
    asm volatile("sfence" ::: "memory");
}
