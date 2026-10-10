#include "IO.hpp"

const gpio_num_t inputs_gpio[INPUTS_PIN_COUNT] = {
    [SINAL_LAV_LIVRE]   = GPIO_NUM_15,
    [SINAL_LAV_OCUPADA] = GPIO_NUM_7,
    [SINAL_SEC_LIVRE]   = GPIO_NUM_17,
    [SINAL_SEC_OCUPADA] = GPIO_NUM_16,
    [ESTAGIO_1]         = GPIO_NUM_12,
    [ESTAGIO_2]         = GPIO_NUM_13,
    [ESTAGIO_3]         = GPIO_NUM_14,
    [ESTAGIO_4]         = GPIO_NUM_21,
    [DONGLE_LAV]        = GPIO_NUM_47,
    [DONGLE_SEC]        = GPIO_NUM_48,
    [BOTAO_REDE]        = GPIO_NUM_4
};

const gpio_num_t outputs_gpio[OUTPUTS_PIN_COUNT] = {
    [RELE_LAV]     = GPIO_NUM_5,
    [RELE_SEC]     = GPIO_NUM_6,
    [LED_ENERGIA]  = GPIO_NUM_18,
    [LED_WIFI]     = GPIO_NUM_10,
    [LED_SERVIDOR] = GPIO_NUM_11,
    [MCP_RESET]    = GPIO_NUM_41
};

IO_t inputs[INPUTS_PIN_COUNT];
IO_t outputs[OUTPUTS_PIN_COUNT];

void IO_init_inputs(){
    for(int i = 0; i < INPUTS_PIN_COUNT; i++){
        pinMode(inputs_gpio[i], INPUT);
        inputs[i].gpio = inputs_gpio[i];
        inputs[i].activate = i <= ESTAGIO_4 ? ACTIVATE_IN_HIGH : ACTIVATE_IN_LOW;
    }
}

void IO_init_outputs(){
    for(int i = 0; i < OUTPUTS_PIN_COUNT; i++){
        pinMode(outputs_gpio[i], OUTPUT);
        digitalWrite(outputs_gpio[i], LOW);

        outputs[i].gpio = outputs_gpio[i];
        outputs[i].activate = i != MCP_RESET ? ACTIVATE_IN_HIGH : ACTIVATE_IN_LOW;
    }

    digitalWrite(outputs[MCP_RESET].gpio, HIGH);
}

bool IO_gpio_state(gpio_num_t gpio){
    return digitalRead(gpio) ? ACTIVATE_IN_HIGH : ACTIVATE_IN_LOW;
}

bool IO_input_is_activate(inputs_num_t in){
    return IO_gpio_state(inputs[in].gpio) == inputs[in].activate;
}

void IO_print_input(inputs_num_t in){
    printf("gpio ( %d ): %s\n", (int)inputs[in].gpio, IO_input_is_activate(in) ? "activate" : "deactivate");
}

void IO_print_all_inputs(){
    printf("\n\n=========< INPUTS >=========\n\n");

    for(int i = 0; i < INPUTS_PIN_COUNT; i++){
        printf("    ");
        IO_print_input((inputs_num_t)i);
    }

    printf("\n============================\n\n");
}

void IO_output_write(gpio_num_t gpio, bool level){
    digitalWrite(gpio, level);
}

void IO_output_activate(outputs_num_t out){
    IO_t *output = &outputs[out];
    IO_output_write(output->gpio, output->activate);
}

void IO_output_deactivate(outputs_num_t out){
    IO_t *output = &outputs[out];
    IO_output_write(output->gpio, !output->activate);
}

void IO_output_pulse(outputs_num_t out){
    IO_output_deactivate(out);
    delay(10);
    IO_output_activate(out);
    delay(1000);
    IO_output_deactivate(out);
}
