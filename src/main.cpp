#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>

// #include "fs.hpp"
#include "mqtt.h"
// #include "stage.hpp"
#include "IO.hpp"

// #define I2C_SDA 8
// #define I2C_SCL 9

extern uint8_t connected;

void setup(){
    /** Init esp32s3 peripherals */
    WiFi.disconnect();

    while(!WiFi.isConnected()){
        WiFi.disconnect();
        WiFi.begin("EtaDaSa_2.4", "etasamdani");
        delay(5000);
    }

    delay(2000);

    mqtt_conn_t conn = { 0 };
    conn.uri = MQTT_DEFAULT_URI;
    conn.client_id = "esp-teste-01";

    printf("mqtt err: %d\n\n", (int)mqtt_connection(&conn));
}

void loop(){
    if(connected){
        char payload[] = "{'result': 0}";
        mqtt_publish(payload, MQTT_ACTION_RES_TOPIC);
    }

    delay(2000);
}

/*

/home/ton618/.platformio/packages/tool-esptool/esptool \
    --chip esp32s3 \
    --port /dev/ttyACM0 \
    --baud 460800 \
    write-flash -z \
    --flash-mode dio \
    --flash-freq 80m \
    --flash-size 8MB \
    0x0000 .pio/build/esp32-s3-devkitc-1/bootloader.bin \
    0x8000 .pio/build/esp32-s3-devkitc-1/partitions.bin \
    0xe000 ~/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin \
    0x80000 .pio/build/esp32-s3-devkitc-1/firmware.bin

*/
