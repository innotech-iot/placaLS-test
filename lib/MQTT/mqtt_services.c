#include <string.h>

#include "mqtt_services.h"
#include "mqtt.h"

#include "machine.h"
#include "mcp23017.h"
#include "IOs.h"

#define mqtt_services_priv_lib__ static inline
#define __loop__ for(;;)

#define __mqtt_services_lock__(service)                                           \
    (!service.private_.os_src.semph ||                                             \
     xSemaphoreTake(service.private_.os_src.semph, pdMS_TO_TICKS(5000)) == pdTRUE)

#define __mqtt_services_unlock__(service)                 \
    do {                                                  \
        if(service.private_.os_src.semph){                 \
            xSemaphoreGive(service.private_.os_src.semph); \
        }                                                 \
    } while(0)

char report_buf[4096] = { 0 };
mqtt_monitor_t mqtt_monitor = { 0 };

mqtt_services_priv_lib__ void monitor_init();
mqtt_services_priv_lib__ void monitor_task(void *params);

void mqtt_services_init(){
    monitor_init();
}

void mqtt_monitor_request_status_update(){
    printf("vai obter mutex\n\n");
    __mqtt_services_lock__(mqtt_monitor);
    printf("obteve mutex\n\n");
    mqtt_monitor.private_.report.requested = 1;
    __mqtt_services_unlock__(mqtt_monitor);
}

mqtt_services_priv_lib__ void monitor_init(){
    mqtt_monitor.private_.os_src.semph = xSemaphoreCreateMutex();
    xTaskCreate(monitor_task, "monitor", 8192, NULL, 1, NULL);
}

mqtt_services_priv_lib__ bool monitor_get_report_request(){
    __mqtt_services_lock__(mqtt_monitor);
    bool req = mqtt_monitor.private_.report.requested ? true : false;
    __mqtt_services_unlock__(mqtt_monitor);

    return req;
}

mqtt_services_priv_lib__ void monitor_reset_request(){
    __mqtt_services_lock__(mqtt_monitor);
    mqtt_monitor.private_.report.requested = 0;
    __mqtt_services_unlock__(mqtt_monitor);
}

mqtt_services_priv_lib__ const char* machine_state_to_str(machine_select_t select){
    static char wstate[sizeof("error")] = "";
    static char dstate[sizeof("error")] = "";

    machine_t *machine = select == MACHINE_WASHING ? &washing : &dryer;
    char *mstate = select == MACHINE_WASHING ? wstate : dstate;

    const char *states[] = { "free", "busy", "error" };
    int st = machine->history.state;

    memcpy(mstate, states[st], strlen(states[st]) + 1);
    return mstate;
}

mqtt_services_priv_lib__ bool monitor_publish_report(){
    __mqtt_services_lock__(mqtt_monitor);
    memset(report_buf, 0, sizeof(report_buf));

    sprintf(
        report_buf,

        "{                                      \n"
        "   'event-timestamp': %" PRId64 ",     \n"
        "                                       \n"
        "   'washing': {                        \n"
        "       'status': '%s',                 \n"
        "       'error': %d,                    \n"
        "       'stage': %d,                    \n"
        "       'locked': %d,                   \n"
        "       'start-timestamp': %" PRId64 "  \n"
        "   },                                  \n"
        "                                       \n"
        "   'dryer': {                          \n"
        "       'status': '%s',                 \n"
        "       'error': %d,                    \n"
        "       'stage': %d,                    \n"
        "       'locked': %d,                   \n"
        "       'start-timestamp': %" PRId64 "  \n"
        "   },                                  \n"
        "                                       \n"
        "   'peripherals': {                    \n"
        "       'mcp': '%s',                    \n"
        "       'rtc': '%s',                    \n"
        "       'aux-board': '%s'               \n"
        "   },                                  \n"
        "                                       \n"
        "   'io': {                             \n"
        "       'inputs': {                     \n"
        "           'washing-state': %d,        \n"
        "           'dryer-state': %d,          \n"
        "           'stage-1': %d,              \n"
        "           'stage-2': %d,              \n"
        "           'stage-3': %d,              \n"
        "           'stage-4': %d,              \n"
        "           'dongle-washing': %d,       \n"
        "           'dongle-dryer': %d          \n"
        "       },                              \n"
        "                                       \n"
        "       'outputs': {                    \n"
        "           'washing-relay': %d,        \n"
        "           'dryer-relay': %d           \n"
        "       }                               \n"
        "   },                                  \n"
        "                                       \n"
        "   'pumps': {                          \n"
        "       'P1': %d,                       \n"
        "       'P2': %d,                       \n"
        "       'P3': %d,                       \n"
        "       'P4': %d,                       \n"
        "       'P5': %d,                       \n"
        "       'P6': %d,                       \n"
        "       'P7': %d                        \n"
        "   }                                   \n"
        "}                                      ",

        0LL,

        machine_state_to_str(MACHINE_WASHING),
        washing.err,
        washing.history.stage_num,
        washing.history.locked,
        washing.history.start_oper_tm,

        machine_state_to_str(MACHINE_DRYER),
        dryer.err,
        dryer.history.stage_num,
        dryer.history.locked,
        dryer.history.start_oper_tm,

        "online",
        "online",
        "online",

        IOs_is_activated(WASHING_STATE, washing.software_mode),
        IOs_is_activated(DRYER_STATE, dryer.software_mode),
        IOs_is_activated(STAGE_1, washing.software_mode),
        IOs_is_activated(STAGE_2, washing.software_mode),
        IOs_is_activated(STAGE_3, washing.software_mode),
        IOs_is_activated(STAGE_4, washing.software_mode),
        IOs_is_activated(DONGLE_WAS, washing.software_mode),
        IOs_is_activated(DONGLE_DRY, dryer.software_mode),

        0,
        0,

        mcp_read_gpiox_state(&mcp, MCP_GPIO_NUM_0x0),
        mcp_read_gpiox_state(&mcp, MCP_GPIO_NUM_0x1),
        mcp_read_gpiox_state(&mcp, MCP_GPIO_NUM_0x2),
        mcp_read_gpiox_state(&mcp, MCP_GPIO_NUM_0x3),
        mcp_read_gpiox_state(&mcp, MCP_GPIO_NUM_0x4),
        mcp_read_gpiox_state(&mcp, MCP_GPIO_NUM_0x5),
        mcp_read_gpiox_state(&mcp, MCP_GPIO_NUM_0x6)
    );

    mqtt_publish(report_buf, MQTT_STATUS_TOPIC);
    __mqtt_services_unlock__(mqtt_monitor);

    return true;
}

mqtt_services_priv_lib__ void monitor_task(void *params){
    __loop__ {
        printf("monitor_task\n\n");
        vTaskDelay(pdMS_TO_TICKS(100));

        if(!monitor_get_report_request() || !mqtt_is_connected()){
            continue;
        }

        monitor_publish_report();
        monitor_reset_request();
    }
}

#undef __loop__
