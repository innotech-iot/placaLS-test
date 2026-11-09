#include "mcp23017.h"

#include <Adafruit_MCP23X17.h>
#include <Arduino.h>

static Adafruit_MCP23X17 mcp_driver;
mcp23017_t mcp = { 0 };

bool mcp_init(mcp23017_t *mcp){
    if(mcp == NULL){
        return false;
    }

    mcp->started = mcp_driver.begin_I2C();
    mcp->error = !mcp->started;

    return mcp->started;
}

bool mcp_get_error(const mcp23017_t *mcp){
    if(mcp == NULL){
        return true;
    }

    return mcp->error;
}

bool mcp_set_mode(mcp23017_t *mcp, mcp_gpio_num_t gpio, mcp_gpio_mode_t mode){
    if(mcp == NULL){
        return false;
    }

    if(!mcp->started){
        mcp->error = true;
        return false;
    }

    mcp_driver.pinMode((uint8_t)gpio, (uint8_t)mode);
    return true;
}

bool mcp_write(mcp23017_t *mcp, mcp_gpio_num_t gpio, bool value){
    if(mcp == NULL){
        return false;
    }

    if(!mcp->started){
        mcp->error = true;
        return false;
    }

    int gpio_ = (int)gpio;

    if(gpio_ < 0 || gpio_ > MCP_GPIO_NUM_0xF){
        mcp->error = true;
        return false;
    }

    mcp_driver.digitalWrite(gpio_, value);
    delay(10);

    uint8_t gpiox;

    if(gpio_ <= MCP_GPIO_NUM_0x7){
        gpiox = mcp_driver.readGPIOA();
    } 
    
    else {
        gpiox = mcp_driver.readGPIOB();
    }

    bool gpio_st = (gpiox & (1 << (gpio_ % 8))) != 0;
    mcp->error = gpio_st != value;

    return gpio_st == value;
}