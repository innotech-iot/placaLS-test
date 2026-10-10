#include <Arduino.h>
#include <ArduinoJson.h>

#include "mqtt_client.h"
#include "mqtt.hpp"
#include "stage.hpp"
#include "IO.hpp"

extern char device_id[64];
esp_mqtt_client_handle_t mqtt = NULL;

esp_err_t mqtt_event(esp_mqtt_event_handle_t event);
void recv_message(const char *_topic, const char *_message, size_t tlen, size_t mlen);
bool can_release(bool lav);

char topic_cmd[128] = { 0 };
char topic_res[128] = { 0 };
char topic_status[128] = { 0 };
char topic_online[128] = { 0 };

void start_topics(){
    sprintf(topic_cmd, "%s/cmd", device_id);
    sprintf(topic_res, "%s/res", device_id);
    sprintf(topic_status, "%s/status", device_id);
    sprintf(topic_online, "%s/online", device_id);
}

bool mqtt_connect(mqtt_conn_t *conn){
    if(mqtt){
        return true;
    }

    if(conn->uri == NULL || conn->uri[0] == '\0'){
        return false;
    }

    start_topics();

    esp_mqtt_client_config_t config = {};
    config.uri          = conn->uri;
    config.client_id    = conn->client_id;
    config.keepalive    = 60;
    config.event_handle = mqtt_event;

    mqtt = esp_mqtt_client_init(&config);

    if(mqtt == NULL){
        return false;
    }

    if(esp_mqtt_client_start(mqtt) != ESP_OK){
        esp_mqtt_client_destroy(mqtt);
        mqtt = NULL;
        return false;
    }

    return true;
}

esp_err_t mqtt_event(esp_mqtt_event_handle_t event){
    switch(event->event_id){
        case MQTT_EVENT_CONNECTED:
            printf("[MQTT] Connected\n");
            IO_output_activate(LED_SERVIDOR);
            esp_mqtt_client_subscribe(mqtt, topic_cmd, 0);
            break;

        case MQTT_EVENT_DISCONNECTED:
            printf("[MQTT] Disconnected\n");
            IO_output_deactivate(LED_SERVIDOR);
            break;

        case MQTT_EVENT_SUBSCRIBED:
            printf("[MQTT] Subscribed: msg_id=%d\n", event->msg_id);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            Serial.printf("[MQTT] Unsubscribed: msg_id=%d\n", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            Serial.printf("[MQTT] Message published: msg_id=%d\n", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            printf("[MQTT] Message received\n");
            recv_message(event->topic, event->data, event->topic_len, event->data_len);
            break;

        case MQTT_EVENT_ERROR:
            printf("[MQTT] Connection error\n");

            if(event->error_handle != NULL){
                printf("  Error type: %d\n", event->error_handle->error_type);

                if(event->error_handle->esp_transport_sock_errno != 0){
                    printf("  Socket errno: %d\n", event->error_handle->esp_transport_sock_errno);
                }
            }

            break;

        case MQTT_EVENT_BEFORE_CONNECT:
            printf("[MQTT] Try connecting...\n");
            break;

        case MQTT_EVENT_DELETED:
            printf("[MQTT] Client deleted\n");
            break;

        default:
            printf("[MQTT] Unknown event: %d\n", event->event_id);
            break;
    }

    return ESP_OK;
}

void recv_message(const char *_topic, const char *_message, size_t tlen, size_t mlen){
    char topic[tlen + 1] = { 0 };
    char message[mlen + 1] = { 0 };

    memcpy(topic, _topic, tlen);
    memcpy(message, _message, mlen);

    printf("topic: %s | message: %s\n\n", topic, message);

    JsonDocument json;
    deserializeJson(json, message);

    if(!strcmp(topic, topic_cmd)){
        if(json["dados"].is<JsonObject>()){
            if(json["dados"]["plano"].is<JsonObject>()){
                const char *keys[] = { "1", "2", "3", "4" };

                for(int i = 0; i < 4; i++){
                    if(json["dados"]["plano"][keys[i]].is<JsonArray>()){
                        uint8_t pumps[PUMPS_MAX_COUNT] = { 0 };
                        size_t n = json["dados"]["plano"][keys[i]].as<JsonArray>().size();

                        n = n > PUMPS_MAX_COUNT ? PUMPS_MAX_COUNT : n;

                        for(int j = 0; j < n; j++){
                            if(json["dados"]["plano"][keys[i]].as<JsonArray>()[j].is<uint8_t>()){
                                uint8_t v = json["dados"]["plano"][keys[i]].as<JsonArray>()[j].as<uint8_t>();
                                pumps[j] = v;
                            }

                            else {
                                pumps[j] = 0;
                            }
                        }

                        stage_set(i, pumps, n);
                    }
                }
            }
        }

        if(json["cmd"].is<String>() && json["maquina"].is<String>()){
            if(json["cmd"].as<String>() == "LIBERAR"){
                if(can_release(json["maquina"].as<String>() == "LAVADORA")){
                    
                }
            }
        }

        json.clear();
    }
}

bool can_release(bool lav){
    inputs_num_t free_num = lav ? SINAL_LAV_LIVRE : SINAL_SEC_LIVRE;
    inputs_num_t busy_num = lav ? SINAL_LAV_OCUPADA : SINAL_SEC_OCUPADA;

    bool free = reads[free_num].currlv == inputs[free_num].activate;
    free &= reads[busy_num].currlv == !inputs[busy_num].activate;

    return free;
}
