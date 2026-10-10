#ifndef IO_HPP
#define IO_HPP

#include <Arduino.h>
#include "driver/gpio.h"

#define activate_t bool

#define ACTIVATE_IN_HIGH true
#define ACTIVATE_IN_LOW  false

typedef enum {
    SINAL_LAV_LIVRE = 0,
    SINAL_LAV_OCUPADA,
    SINAL_SEC_LIVRE,
    SINAL_SEC_OCUPADA,
    ESTAGIO_1,
    ESTAGIO_2,
    ESTAGIO_3,
    ESTAGIO_4,
    DONGLE_LAV,
    DONGLE_SEC,
    BOTAO_REDE,
    INPUTS_PIN_COUNT
} inputs_num_t;

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
    gpio_num_t gpio;
    activate_t activate;
} IO_t;

typedef struct {
    int lastlv;
    int currlv;
    uint32_t timer;
} read_input_t;

extern const gpio_num_t inputs_gpio[INPUTS_PIN_COUNT];
extern const gpio_num_t outputs_gpio[OUTPUTS_PIN_COUNT];

extern IO_t inputs[INPUTS_PIN_COUNT];
extern IO_t outputs[OUTPUTS_PIN_COUNT];

extern read_input_t reads[INPUTS_PIN_COUNT];

void IO_init_inputs();
void IO_init_outputs();
void IO_print_input(inputs_num_t in);
void IO_print_all_inputs();
void IO_output_write(gpio_num_t gpio, bool level);
void IO_output_activate(outputs_num_t out);
void IO_output_deactivate(outputs_num_t out);
void IO_output_pulse(outputs_num_t out);

bool IO_gpio_state(gpio_num_t gpio);
bool IO_input_is_activate(inputs_num_t in);

#endif // IO_HPP
