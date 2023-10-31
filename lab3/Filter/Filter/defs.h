#pragma once
#include <stdio.h>

#define CHECK(condition, ret, errorText, ...){		\
	if (!(condition)){								\
		printf(errorText);							\
        __VA_ARGS__;								\
        return (ret);								\
	}												\
} 