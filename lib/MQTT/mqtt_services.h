#ifndef __mqtt_services_lib_mqtt_service_h__
#define __mqtt_services_lib_mqtt_service_h__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"

typedef struct {
    struct {
        struct {
            uint8_t requested;
        } report;

        struct {
            SemaphoreHandle_t semph;
        } os_src;
    } private;
} mqtt_monitor_t;

#ifdef __cplusplus
extern "C" {
#endif

void mqtt_services_init();
void mqtt_monitor_request_status_update();

#ifdef __cplusplus
}
#endif

extern mqtt_monitor_t mqtt_monitor;

#endif // __mqtt_services_lib_mqtt_service_h__
