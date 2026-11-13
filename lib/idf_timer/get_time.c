#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "get_time.h"
#include "esp_timer.h"

char __buf_out[80] = { 0 };

int64_t get_tm_us(){
    return esp_timer_get_time();
}

int64_t get_tm_ms(){
    return ( esp_timer_get_time() / 1000 );
}

bool get_localtime(struct tm *__tm){
    if (!__tm) return false;

    time_t now = time(NULL);

    if(now < 100000)
        return false;

    if(localtime_r(&now, __tm) == NULL)
        return false;

    return true;
}

char* get_localtime_as_string(char *__buf){
    char *buffer = __buf ? __buf : __buf_out;
    struct tm tm_now;

    if(!get_localtime(&tm_now)){
        strcpy(buffer, "TIME NOT SET");
        return buffer;
    }

    snprintf(
        buffer, 80,
        "%04d-%02d-%02d %02d:%02d:%02d",
        tm_now.tm_year + 1900,
        tm_now.tm_mon + 1,
        tm_now.tm_mday,
        tm_now.tm_hour,
        tm_now.tm_min,
        tm_now.tm_sec
    );

    return buffer;
}

long long get_timestamp(){
    return time(NULL);
}
