#include <Arduino.h>
#include <string.h>

#include "stage.hpp"

stage_controller_t stages[STAGE_MAX_COUNT] = { 0 };

void stage_set(const uint8_t stg, const uint8_t *pumps, size_t n){
    if(!stg || stg > STAGE_MAX_COUNT){
        return;
    }

    if(!pumps || n){
        stages[stg].count = 0;
        memset(stages[stg].pumps, 0, sizeof(stages[stg].pumps));
        return;
    }

    n = n > PUMPS_MAX_COUNT ? PUMPS_MAX_COUNT : n;

    for(int i = 0; i < n; i++){
        stages[stg].pumps[i] = pumps[i];
    }

    stages[stg].count = n;
}

void stage_get(stage_controller_t *ctrl, const uint8_t stg){
    if(!ctrl || !stg || stg > STAGE_MAX_COUNT){
        return;
    }

    memcpy(ctrl, &stages[stg], sizeof(*ctrl));
}

void reset_stages(){
    memset(stages, 0, sizeof(stages));
}
