#ifndef __libmqtt_mqtt_h__
#define __libmqtt_mqtt_h__

#include "esp_err.h"

typedef struct {
    const char *uri;
    const char *client_id;
    const char *ca_path;
    const char *cert_path;
    const char *pkey_path;
} mqtt_conn_t;

typedef enum {
    MQTT_ACTION_REQ_TOPIC,
    MQTT_ACTION_RES_TOPIC,
    MQTT_LOG_REQ_TOPIC,
    MQTT_LOG_RES_TOPIC,
    MQTT_STATUS_TOPIC,
    MQTT_LWT_TOPIC,
    MQTT_NUM_OF_TOPICS
} mqtt_topics_t;

typedef struct {
    char *topic_buf;
    char *topic_name;
} topic_t;

esp_err_t mqtt_create_conn(mqtt_conn_t *conn);
int mqtt_publish(char *data, const mqtt_topics_t topic);

extern const topic_t all_topics[MQTT_NUM_OF_TOPICS];

#endif // __libmqtt_mqtt_h__
