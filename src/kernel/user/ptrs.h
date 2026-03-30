#pragma once

#define KSTART 0x0000800000000000ULL

static inline int is_valid_user_ptr(u64 pointer) {
	/* This should be used to chec k if a user-provided pointer
	 * is within the "safe" range (id est, not inside the kernel's memory space)
	 * and is not NULL.
	*/
	return pointer && pointer <= KSTART;
}

static inline int is_valid_user_ptr_range(u64 pointer, u64 size) {
	/* This should be used to check if a user-provided pointer and `size` ahead of it
	 * is within the "safe" range and is not NULL.
	*/
	if (size == 0) return 0;
	if (size == 1) return is_valid_user_ptr(pointer);
	
	u64 max;
	if (__builtin_add_overflow(pointer, size, &max))
		return 0;

	return max <= KSTART;
}