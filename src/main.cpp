#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>

// #include "fs.hpp"
#include "mqtt.h"
#include "mqtt_services.h"
#include "get_time.h"
#include "IOs.h"
#include "LittleFS.h"

// #define I2C_SDA 8
// #define I2C_SCL 9

extern uint8_t connected;

void setup(){
    printf("--lfs begin: %d\n\n", LittleFS.begin(true, "/lfs", 10, "littlefs"));

    /** Init esp32s3 peripherals */
    

    delay(2000);

    mqtt_conn_t conn = { 0 };
    conn.uri = MQTT_DEFAULT_URI;
    conn.client_id = "laundry-AABBCCDDEEFF";

    printf("mqtt err: %d\n\n", (int)mqtt_connection(&conn));
}

void loop(){
    // bool already_start_mqtt_services = false;
    // int64_t start = 0;

    // for(;;){
    //     if(!already_start_mqtt_services){
    //         mqtt_services_init();
    //         already_start_mqtt_services = true;
    //         printf("mqtt services started\n\n");
    //     }

    //     int64_t now = get_tm_ms();

    //     if(now - start >= 2000){
    //         mqtt_monitor_request_status_update();
    //         start = now;
    //     }
    // }

    for(;;){
        FILE *file = fopen("/lfs/test.txt", "r");

        if(file){
            char buf[256] = { 0 };
            size_t nread = fread(buf, sizeof(char), sizeof(buf), file);
            buf[nread] = '\0';
            printf("buf: %s\n", buf);
            fclose(file);
        }

        delay(3000);
    }
}

void connect_on_wifi(){
    WiFi.disconnect();

    while(!WiFi.isConnected()){
        WiFi.disconnect();
        WiFi.begin("EtaDaSa_2.4", "etasamdani");
        delay(5000);
    }
}

/*

/home/ton618/.platformio/packages/tool-esptool/esptool \
    --chip esp32s3 \
    --baud 460800 \
    write-flash \
    0x0000 .pio/build/esp32-s3-devkitc-1/bootloader.bin \
    0x8000 .pio/build/esp32-s3-devkitc-1/partitions.bin \
    0x80000 .pio/build/esp32-s3-devkitc-1/firmware.bin

*/
