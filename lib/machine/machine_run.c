#include "machine.h"
#include "mcp23017.h"
#include "mqtt_services.h"
#include "get_time.h"
#include "IOs.h"

#define __machine_violation_guard__(machine)    \
    machine_violation_guard(machine);           \
                                                \
    if(machine_is_locked(machine)){             \
        continue;                               \
    }

#define __machine_error_guard__(machine)        \
    machine_error_guard(machine, &already_err); \
                                                \
    if(machine.history.state == MACHINE_ERROR){ \
        continue;                               \
    }

#define __machine_is_washing__() ( machine->select == MACHINE_WASHING )

#define __machine_priv_lib__ static inline
#define __loop__ for(;;)

__machine_priv_lib__ void machine_violation_guard(machine_t *machine);
__machine_priv_lib__ void machine_error_guard(machine_t *machine, bool *already_err);
__machine_priv_lib__ void deactivate_pumps();

__machine_priv_lib__ washing_run_task(void *params){
    bool already_err = washing.history.state == MACHINE_ERROR;
    machine_stage_num_t last_stage = washing.history.stage_num;

    __loop__ {
        vTaskDelay(pdMS_TO_TICKS(10));

        /** If the machine's dongle is not detected, mark the machine as tampered with */
        __machine_violation_guard__(&washing);

        /** Machine's state is ERROR */
        __machine_error_guard__(&washing);
    }
}

__machine_priv_lib__ void machine_violation_guard(machine_t *machine){
    digital_inputs_num_t in_num = __machine_is_washing__() ? DONGLE_WAS : DONGLE_DRY;

    if(!IOs_is_activated(in_num) && !machine_is_locked(machine)){
        machine_set_locked(machine, true);

        if(__machine_is_washing__()){
            deactivate_pumps();
        }

        mqtt_monitor_request_status_update();
    }
}

__machine_priv_lib__ void machine_error_guard(machine_t *machine, bool *already_err){
    if(machine->history.state != MACHINE_ERROR){
        return;
    }
}

__machine_priv_lib__ void deactivate_pumps(){
    for(int i = 0; i < 7; i++){
        mcp_write(&mcp, (mcp_gpio_num_t)i, true);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

__machine_priv_lib__ uint8_t num_of_activates_stages(bool *stages){
    uint8_t num = 0;

    for(int i = 0; i < NUM_OF_STAGES; i++){
        if(stages[i]){
            num++;
        }
    }

    return num;
}

__machine_priv_lib__ void washing_controller(){
    bool stages[NUM_OF_STAGES] = { 0 };

    for(int i = 0; i < NUM_OF_STAGES; i++){
        stages[i] = IOs_is_activated((digital_inputs_num_t)i);
    }

    if(washing.history.state == MACHINE_BUSY){
        // if(get)

        machine_stage_num_t curr_stage = washing.history.stage_num;

        if(curr_stage == MACHINE_STAGE_I){
            if(num_of_activates_stages(stages)){

            }
        }
    }
}

/*

1 - recebe comando do broker
2 - verifica se está livre
3 - se estiver insere moeda
4 - aguarda estado


*/

#undef __loop__
#undef __machine_priv_lib__
