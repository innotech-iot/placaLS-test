#include <Arduino.h>

#include "mqtt_client.h"
#include "mqtt.hpp"

esp_mqtt_client_handle_t mqtt = NULL;

esp_err_t mqtt_event(esp_mqtt_event_handle_t event);

bool mqtt_connect(mqtt_conn_t *conn){
    if(mqtt){
        return true;
    }

    if(conn->uri == NULL || conn->uri[0] == '\0'){
        return false;
    }

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

            esp_mqtt_client_subscribe(mqtt, "test", 0);
            break;

        case MQTT_EVENT_DISCONNECTED:
            printf("[MQTT] Disconnected\n");
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
            printf("  Topic: %.*s\n", event->topic_len, event->topic);
            printf("  Data: %.*s\n", event->data_len, event->data);
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