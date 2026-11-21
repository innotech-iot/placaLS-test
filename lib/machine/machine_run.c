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

__machine_priv_lib__ void washing_run_task(void *params){
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

    if(!IOs_is_activated(in_num, machine->software_mode) && !machine_is_locked(machine)){
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

__machine_priv_lib__ mcp_gpio_num_t machine_pump_to_gpio(machine_pumps_num_t pump)
{
    if(pump < MACHINE_PUMP_1 || pump > MACHINE_PUMP_7){
        return MCP_GPIO_NUM_0x0;
    }

    return (mcp_gpio_num_t)(pump - MACHINE_PUMP_1);
}

__machine_priv_lib__ void activate_stage_pumps(
    machine_t *machine,
    machine_stage_num_t stage_num
)
{
    if(stage_num < MACHINE_STAGE_1 ||
       stage_num > MACHINE_STAGE_4){
        return;
    }

    /*
     * Sempre garante que nenhuma bomba esteja ligada
     * antes de iniciar um novo estágio.
     */
    deactivate_pumps();

    machine_stage_t *stage = &machine->history.stage[stage_num - MACHINE_STAGE_1];
    machine_pumps_t *pumps = &stage->st_pumps;

    for(uint8_t i = 0; i < pumps->c; i++){
        machine_pumps_num_t pump = pumps->p[i];

        if(pump < MACHINE_PUMP_1 || pump > MACHINE_PUMP_7){
            continue;
        }

        mcp_gpio_num_t gpio = machine_pump_to_gpio(pump);

        /*
         * LOW = bomba ligada
         */
        mcp_write(&mcp, gpio, false);
        int64_t duration = pumps->t[i] * pumps->r[i];

        if(duration <= 0){
            duration = 1000;
        }

        vTaskDelay(pdMS_TO_TICKS(duration));

        /*
         * HIGH = bomba desligada
         */
        mcp_write(&mcp, gpio, true);
    }
}

__machine_priv_lib__ void washing_controller(){
    static machine_stage_num_t last_stage = MACHINE_STAGE_I;

    bool stages[NUM_OF_STAGES] = { 0 };

    for(int i = 0; i < NUM_OF_STAGES; i++){
        stages[i] = IOs_is_activated((digital_inputs_num_t)i, washing.software_mode);
    }

    uint8_t number_of_stages_on = num_of_activates_stages(stages);

    if(number_of_stages_on > 1){
        /** Estado inválido */
        return;
    }

    if(washing.history.state != MACHINE_BUSY){
        last_stage = MACHINE_STAGE_I;
        deactivate_pumps();
        return;
    }

    machine_stage_num_t curr_stage = washing.history.stage_num;

    /*
     * Só executa o acionamento quando houver
     * efetivamente uma troca de estágio.
     */
    if(curr_stage != last_stage){
        activate_stage_pumps(
            &washing,
            curr_stage
        );

        last_stage = curr_stage;
    }

    if(curr_stage == MACHINE_STAGE_I){
        /*
         * MACHINE_STAGE_I representa ausência de estágio
         * operacional.
         */
        return;
    }

    if(curr_stage == MACHINE_STAGE_1){
        if(!number_of_stages_on){
            /** Erro por estar inoperante */
        }

        if(stages[STAGE_1]){
            
        }
    }
}

#undef __loop__
#undef __machine_priv_lib__
