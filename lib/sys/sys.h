#ifndef __syslib_sys_h__
#define __syslib_sys_h__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    struct {
        uint8_t requested;
    } report;
} sys_t;

#endif // __syslib_sys_h__
