#include "IOs.h"
#include "get_time.h"
#include "FreeRTOS.h"

#define EMPTY_BUFFER { 0 }
#define loop for(;;)

#define __ios_priv_lib__ static inline

const gpio_num_t dinputs_gpio[DINPUTS_PIN_COUNT] = {
    [STAGE_1]    = GPIO_NUM_12,
    [STAGE_2]    = GPIO_NUM_13,
    [STAGE_3]    = GPIO_NUM_14,
    [STAGE_4]    = GPIO_NUM_21,
    [DONGLE_LAV] = GPIO_NUM_47,
    [DONGLE_SEC] = GPIO_NUM_48,
    [BOTAO_REDE] = GPIO_NUM_4
};

const gpio_num_t outputs_gpio[OUTPUTS_PIN_COUNT] = {
    [RELE_LAV]     = GPIO_NUM_5,
    [RELE_SEC]     = GPIO_NUM_6,
    [LED_ENERGIA]  = GPIO_NUM_18,
    [LED_WIFI]     = GPIO_NUM_10,
    [LED_SERVIDOR] = GPIO_NUM_11,
    [MCP_RESET]    = GPIO_NUM_41
};

IO_t dinputs[DINPUTS_PIN_COUNT] = EMPTY_BUFFER;
IO_t outputs[OUTPUTS_PIN_COUNT] = EMPTY_BUFFER;

void IOs_read_task_(void *params);
__ios_priv_lib__ void check_switch_state_(bool read, int i);

void IOs_start(){
    for(int i = 0; i < DINPUTS_PIN_COUNT; i++){
        gpio_set_direction(dinputs_gpio[i], GPIO_MODE_INPUT);

        dinputs[i].same_state_tm   = 0;
        dinputs[i].tm_to_switch_st = i <= STAGE_4 ? 100 : 50;
        dinputs[i].gpio            = dinputs_gpio[i];
        dinputs[i].state           = 0;
        dinputs[i].act_condition   = i <= STAGE_4 ? ACTIVATE_IN_LOW : ACTIVATE_IN_HIGH;
    }

    /** Delay to normalize readings */
    vTaskDelay(pdMS_TO_TICKS(10));
    xTaskCreate(IOs_read_task_, "IOs-Read", 2048, NULL, 1, NULL);
    /** Pause to wait IOs_read_task done first readings */
    vTaskDelay(pdMS_TO_TICKS(500));
}

void IOs_read_task_(void *params){
    loop {
        int curr_reads[DINPUTS_PIN_COUNT] = EMPTY_BUFFER;

        for(int i = 0; i < DINPUTS_PIN_COUNT; i++){
            curr_reads[i] = gpio_get_level(dinputs[i].gpio) ? 1 : 0;
            /** Small debounse */
            vTaskDelay(pdMS_TO_TICKS(5));
            /** Done new read */
            int read = gpio_get_level(dinputs[i].gpio) ? 1 : 0;

            if(read != curr_reads[i]){
                continue;
            }

            check_switch_state(read, i);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

bool IOs_is_activated(digital_inputs_num_t din_num){
    return dinputs[din_num].state;
}

__ios_priv_lib__ void check_switch_state_(bool read, int i){
    activate_t act = read == dinputs[i].act_condition ? 1 : 0;

    if(dinputs[i].state == act){
        dinputs[i].same_state_tm = 0;
        return;
    }

    if(!dinputs[i].same_state_tm){
        dinputs[i].same_state_tm = get_tm_ms();
        return;
    }

    if(get_tm_ms() - dinputs[i].same_state_tm >= dinputs[i].tm_to_switch_st){
        dinputs[i].state = act;
        dinputs[i].same_state_tm = 0;
    }
}

#undef EMPTY_BUFFER
#undef loop
