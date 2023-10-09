#pragma once
#include <stdio.h>

#define CHECK(condition, errorText, ...){		\
	if (!(condition)){							\
		printf(errorText);                      \
        __VA_ARGS__;							\
        return 0;								\
	}											\
} 