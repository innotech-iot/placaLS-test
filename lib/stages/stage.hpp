#ifndef STAGE_HPP
#define STAGE_HPP

#include <stdio.h>
#include <stdint.h>

#define STAGE_MAX_COUNT 4
#define PUMPS_MAX_COUNT 7

typedef struct {
    uint8_t pumps[PUMPS_MAX_COUNT];
    uint8_t count;
} stage_controller_t;

void stage_set(const uint8_t stg, const uint8_t *pumps, size_t n);
void stage_get(stage_controller_t *ctrl, const uint8_t stg);
void reset_stages();

#endif // STAGE_HPP
