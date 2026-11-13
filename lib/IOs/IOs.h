#ifndef __libios_ios_h__
#define __libios_ios_h__

#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"

typedef uint8_t activate_t;

#define ACTIVATE_IN_HIGH true
#define ACTIVATE_IN_LOW  false

typedef enum {
    STAGE_1,
    STAGE_2,
    STAGE_3,
    STAGE_4,
    DONGLE_LAV,
    DONGLE_SEC,
    BOTAO_REDE,
    DINPUTS_PIN_COUNT
} digital_inputs_num_t;

typedef enum {
    RELE_LAV = 0,
    RELE_SEC,
    LED_ENERGIA,
    LED_WIFI,
    LED_SERVIDOR,
    MCP_RESET,
    OUTPUTS_PIN_COUNT
} outputs_num_t;

typedef struct {
    int64_t    same_state_tm;
    int64_t    tm_to_switch_st;
    gpio_num_t gpio;
    activate_t state;
    activate_t act_condition;
} IO_t;

#ifdef __cplusplus
extern "C" {
#endif

void IOs_start();
void IOs_read_task_(void *params);
bool IOs_is_activated(digital_inputs_num_t din_num);

#ifdef __cplusplus
}
#endif

#endif // __libios_ios_h__
