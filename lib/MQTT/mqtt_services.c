#include "mqtt_services.h"
#include "mqtt.h"

#include "IOs.h"

#define mqtt_services_priv_lib__ static inline
#define __loop__ for(;;)

#define __mqtt_services_lock__(service)                                           \
    (!service.private.os_src.semph ||                                             \
     xSemaphoreTake(service.private.os_src.semph, pdMS_TO_TICKS(5000)) == pdTRUE)

#define __mqtt_services_unlock__(service)                 \
    do {                                                  \
        if(service.private.os_src.semph){                 \
            xSemaphoreGive(service.private.os_src.semph); \
        }                                                 \
    } while(0)

mqtt_monitor_t mqtt_monitor = { 0 };

mqtt_services_priv_lib__ void monitor_init();

void mqtt_services_init(){
    monitor_init();
}

void mqtt_monitor_request_status_update(){
    __mqtt_services_lock__(mqtt_monitor);
    mqtt_monitor.private.report.requested = 1;
    __mqtt_services_unlock__(mqtt_monitor);
}

mqtt_services_priv_lib__ void monitor_init(){
    mqtt_monitor.private.os_src.semph = xSemaphoreCreateMutex();
}

mqtt_services_priv_lib__ bool monitor_get_report_request(){
    __mqtt_services_lock__(mqtt_monitor);
    bool req = mqtt_monitor.private.report.requested ? true : false;
    __mqtt_services_unlock__(mqtt_monitor);

    return req;
}

mqtt_services_priv_lib__ void monitor_reset_request(){
    __mqtt_services_lock__(mqtt_monitor);
    mqtt_monitor.private.report.requested = 0;
    __mqtt_services_unlock__(mqtt_monitor);
}

// mqtt_services_priv_lib__ bool monitor_publish_report(){
//     char buffer[2048] = { 0 };

//     sprintf(
//         buffer,
//         "{"
//         "   'washing': {"
//         "       'status': '%s',"
//         "       'error': %d,"
//         "       '"
//     );
// }

mqtt_services_priv_lib__ void monitor_task(void *params){
    __loop__ {
        vTaskDelay(pdMS_TO_TICKS(100));

        if(!monitor_get_report_request()){
            continue;
        }

        monitor_reset_request();
    }
}

#undef __loop__
