#ifndef MCP23017_H
#define MCP23017_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    MCP_GPIO_NUM_0x0,
    MCP_GPIO_NUM_0x1,
    MCP_GPIO_NUM_0x2,
    MCP_GPIO_NUM_0x3,
    MCP_GPIO_NUM_0x4,
    MCP_GPIO_NUM_0x5,
    MCP_GPIO_NUM_0x6,
    MCP_GPIO_NUM_0x7,
    MCP_GPIO_NUM_0x8,
    MCP_GPIO_NUM_0x9,
    MCP_GPIO_NUM_0xA,
    MCP_GPIO_NUM_0xB,
    MCP_GPIO_NUM_0xC,
    MCP_GPIO_NUM_0xD,
    MCP_GPIO_NUM_0xE,
    MCP_GPIO_NUM_0xF
} mcp_gpio_num_t;

typedef enum {
    MCP_GPIO_OUTPUT_MODE,
    MCP_GPIO_INPUT_MODE
} mcp_gpio_mode_t;

typedef struct {
    bool error;
    bool started;
} mcp23017_t;

bool mcp_init(mcp23017_t *mcp);
bool mcp_get_error(const mcp23017_t *mcp);
bool mcp_set_mode(mcp23017_t *mcp, mcp_gpio_num_t gpio, mcp_gpio_mode_t mode);
bool mcp_write(mcp23017_t *mcp, mcp_gpio_num_t gpio, bool value);

#endif // MCP23017_H
