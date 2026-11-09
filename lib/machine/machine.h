#ifndef __libmachine_machine_h__
#define __libmachine_machine_h__

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "esp_err.h"

#define MAX_PUMPS_USED_PER_STAGE 7
#define NUM_OF_STAGES            4

#define MACHINE_METADATA_MAGIC_NUM 0x20771984

#define washing_setup()                 machine_setup(&washing, MACHINE_WASHING)
#define washing_set_state(STN)          machine_set_state(&washing, STN)
#define washing_set_stage_pumps(STN, P) machine_set_stage_pumps(&washing, STN, P)
#define washing_history()               machine_history(&washing)
#define washing_get_st()                washing_history().state

#define washing_is_free()               ( washing_get_st() == MACHINE_FREE  )
#define washing_is_busy()               ( washing_get_st() == MACHINE_BUSY  )
#define washing_in_err()                ( washing_get_st() == MACHINE_ERROR )

#define dryer_setup()                   machine_setup(&dryer, MACHINE_DRYER)
#define dryer_set_state(STN)            machine_set_state(&washing, STN)
#define dryer_set_stage_pumps(STN, P)   machine_set_stage_pumps(&dryer, STN, P)
#define dryer_history()                 machine_history(&dryer)
#define dryer_get_st()                  washing_history().state

typedef enum {
    MACHINE_WASHING,
    MACHINE_DRYER
} machine_select_t;

typedef enum {
    MACHINE_OK,
    MACHINE_ERR_OPEN_HISTORY,
    MACHINE_ERR_LOAD_HISTORY,
    MACHINE_ERR_SAVE_HISTORY,
    MACHINE_ERR_ALLOC_SEMPH,
    MACHINE_ERR_INVALID_STATE
} machine_err_t;

typedef enum {
    MACHINE_FREE,
    MACHINE_BUSY,
    MACHINE_ERROR,
} machine_st_t;

typedef enum {
    MACHINE_OK,
    MACHINE_MCP_ERR,
    MACHINE_RTC_ERR,
    MACHINE_MEM_ERR,
    MACHINE_MQTT_BROKER_CERTIFICATE_INV,
    MACHINE_NO_RESPONSE_FROM_WASHING,
    MACHINE_NO_RESPONSE_FROM_DRYER,
} machine_sys_err_t;

typedef enum {
    MACHINE_STAGE_I = -1,
    MACHINE_STAGE_1,
    MACHINE_STAGE_2,
    MACHINE_STAGE_3,
    MACHINE_STAGE_4
} machine_stage_num_t;

typedef enum {
    MACHINE_PUMP_N,
    MACHINE_PUMP_1,
    MACHINE_PUMP_2,
    MACHINE_PUMP_3,
    MACHINE_PUMP_4,
    MACHINE_PUMP_5,
    MACHINE_PUMP_6,
    MACHINE_PUMP_7
} machine_pumps_num_t;

typedef struct {
    uint8_t c;                                       /** Used pumps counter */
    machine_pumps_num_t p[MAX_PUMPS_USED_PER_STAGE]; /** Used pumps */
} machine_pumps_t;

typedef struct {
    machine_pumps_t st_pumps;
} machine_stage_t;

typedef struct {
    machine_stage_t   stage[NUM_OF_STAGES];
    machine_st_t      state;
    machine_sys_err_t err;
} machine_history_t;

typedef struct {
    uint32_t magic_num;
} machine_history_metadata_t;

typedef struct {
    machine_select_t  select;
    machine_history_t history;
    SemaphoreHandle_t semph;
} machine_t;

machine_err_t machine_setup(machine_t *machine, machine_select_t select);
machine_err_t machine_set_state(machine_t *machine, machine_st_t state);
machine_err_t machine_set_stage_pumps(machine_t *machine, machine_stage_num_t st_num, machine_pumps_t *pumps, uint8_t counter);
machine_err_t machine_run(machine_t *machine);
machine_err_t machine_set_stage(machine_t *machine);
machine_history_t machine_history(machine_t *machine);

extern machine_t washing;
extern machine_t dryer;

#endif // __libmachine_machine_h__
