#ifndef GET_TIMER_H
#define GET_TIMER_H

#include "stdint.h"
#include "stdbool.h"
#include <time.h>

/** convert seconds -> milliseconds */
#define SEC_MS(sec) ((int64_t)( (sec) * 1000 ))

/** convert minutes -> milliseconds */
#define MIN_MS(min) ((int64_t)( SEC_MS(min) * 60 ))

/** convert hour -> milliseconds */
#define HOR_MS(hor) ((int64_t)( MIN_MS(hor) * 60 ))

#ifdef __cplusplus
extern "C" {
#endif

int64_t get_tm_us();
int64_t get_tm_ms();

bool get_localtime(struct tm *__tm);
char* get_localtime_as_string(char *__buf);

long long get_timestamp();

#ifdef __cplusplus
}
#endif

#endif // GET_TIMER_H
