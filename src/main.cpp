#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <WiFi.h>

#include "fs.hpp"
#include "mqtt.hpp"
#include "stage.hpp"
#include "IO.hpp"

#define I2C_SDA 8
#define I2C_SCL 9

Adafruit_MCP23X17 mcp;

typedef struct {
    int lastlv;
    int currlv;
    uint32_t timer;
} read_input_t;

read_input_t reads[INPUTS_PIN_COUNT] = { 0 };

char device_id[64] = { 0 };

void connection_task(void *nothing);
void read_inputs_task(void *nothing);

void setup(){
    Serial.begin(115200);
    Wire.begin(I2C_SDA, I2C_SCL);

    IO_output_activate(RELE_LAV);
    IO_output_activate(RELE_SEC);

    pinMode(41, OUTPUT);
    digitalWrite(41, LOW);
    delay(1);
    digitalWrite(41, HIGH);
    delay(1);

    if(!fs_init()){
        printf("[SYS] CRITICAL ERROR\n");
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

    IO_output_activate(LED_ENERGIA);
    reset_stages();

    xTaskCreate(connection_task, "connection", 8192, NULL, 1, NULL);
    xTaskCreate(read_inputs_task, "reads", 3072, NULL, 1, NULL);
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

void connection_task(void *nothing){
    for(;;){
        if(!is_connected_on_wifi()){
            IO_output_deactivate(LED_WIFI);

            if(try_connect_on_wifi()){
                IO_output_activate(LED_WIFI);
                printf("[Wi-Fi] Connected\n");
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

        else {
            IO_output_activate(LED_WIFI);
        }

        delay(7000);
    }
}

void read_inputs_task(void *nothing){
    for(;;){
        for(int i = 0; i < INPUTS_PIN_COUNT; i++){
            uint32_t filter_timer = i < ESTAGIO_1 ? 200 : 50;
            uint32_t now = millis();

            bool level = digitalRead(inputs[i].gpio);

            if(level != reads[i].lastlv){
                reads[i].lastlv = level;
                reads[i].timer = now;
            }

            if(level != reads[i].currlv && (uint32_t)(now - reads[i].timer) >= filter_timer){
                reads[i].currlv = level;
            }
        }

        delay(10);
    }
}
