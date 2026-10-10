#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <WiFi.h>

#include "fs.hpp"
#include "mqtt.hpp"

#define I2C_SDA 8
#define I2C_SCL 9

Adafruit_MCP23X17 mcp;

void setup(){
    Serial.begin(115200);
    Wire.begin(I2C_SDA, I2C_SCL);

    pinMode(41, OUTPUT);
    digitalWrite(41, LOW);
    delay(10);
    digitalWrite(41, HIGH);
    delay(10);

    if(!fs_init()){
        printf("[SYS]CRITICAL ERROR\n");
        for(;;);
    }

    if(!mcp.begin_I2C(0x20, &Wire)){
        Serial.println("Erro ao inicializar MCP23017");

        while(true){
            delay(1000);
        }
    }

    for(int i = 0; i <= 7; i++){
        mcp.pinMode(i, OUTPUT);
        mcp.digitalWrite(i, LOW);
    }
}

void loop(){
    for(int i = 0; i <= 7; i++){
        mcp.digitalWrite(i, HIGH);
        delay(1500);
        mcp.digitalWrite(i, LOW);
        delay(1500);
    }
}

bool is_connected_on_wifi(){
    return WiFi.status() == WL_CONNECTED;
}

bool try_connect_on_wifi(){
    WiFi.begin("VIVOFIBRA-4D58", "CBE041487D");
    const uint32_t start = millis();

    while((uint32_t)(millis() - start) < 30000){
        if(is_connected_on_wifi()){
            return true;
        }

        delay(100);
    }

    return is_connected_on_wifi();
}

void connectionTask(void *nothing){
    for(;;){
        if(!is_connected_on_wifi()){
            if(try_connect_on_wifi()){
                printf("[Wi-Fi]: connected\n");
                // fs::File sconf_bin = fs_open("/mqtt_server.bin", "rb");

                // if(sconf_bin){
                //     if(fs_size(&sconf_bin) == sizeof(mqtt_server_conf_t)){
                //         mqtt_server_conf_t sconf = { 0 };

                //         int read_size = fs_read(
                //             &sconf_bin,
                //             reinterpret_cast<uint8_t*>(&sconf),
                //             sizeof(sconf)
                //         );

                //         if(read_size == sizeof(sconf) && sconf.magic == MQTT_SERVER_MAGIC){
                            mqtt_conn_t conn = { 0 };
                            char uri[128];

                            int uri_size = snprintf(
                                uri,
                                sizeof(uri),
                                "mqtt://%s:%u",
                                "192.168.0.9", // sconf.host,
                                (unsigned int)1883
                            );

                //             if(uri_size > 0 && (size_t)uri_size < sizeof(uri)){
                //                 conn.uri = uri;
                //                 conn.client_id = "cli-test";

                //                 mqtt_connect(&conn);
                //             }
                //         }
                //     }

                //     fs_close(&sconf_bin);
                // }
            }
        }

        delay(7000);
    }
}
