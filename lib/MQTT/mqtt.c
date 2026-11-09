#include <stdio.h>
#include <stdlib.h>

#include "mqtt.h"
#include "mqtt_client.h"

#define GENERIC_ID "laudryctrl-AABBCCDDEEFF/"
#define EMPTY_BUF  { 0 }

#define TOPIC_NAME_LEN(NAME) sizeof(GENERIC_ID NAME)

#define FS_ERROR_OR_FILE_NOT_FOUND (-1)
#define FILE_EMPTY                 (-2)

#define MQTT_BROKER_FILES_SZ 2048
#define MQTT_BUFFER_SIZE     4096

#define __mqtt_priv_lib__ static inline

__mqtt_priv_lib__ void start_topics_(const char *id);
__mqtt_priv_lib__ int load_broker_file_(char *fbuf, size_t fbuf_sz, const char *fname);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

void mqtt_callback(const char *topic, char *data);

esp_mqtt_client_handle_t mqtt = NULL;

char topic_buf[128] = EMPTY_BUF;
char data_buf[MQTT_BUFFER_SIZE + 1] = EMPTY_BUF;

char topic_action_req[TOPIC_NAME_LEN("action/req")] = EMPTY_BUF;
char topic_action_res[TOPIC_NAME_LEN("action/res")] = EMPTY_BUF;
char topic_log_req[TOPIC_NAME_LEN("log/req")]       = EMPTY_BUF;
char topic_log_res[TOPIC_NAME_LEN("log/res")]       = EMPTY_BUF;
char topic_status[TOPIC_NAME_LEN("status")]         = EMPTY_BUF;
char topic_lwt[TOPIC_NAME_LEN("lwt")]               = EMPTY_BUF;

const topic_t all_topics[MQTT_NUM_OF_TOPICS] = {
    { .topic_buf = topic_action_req, .topic_name = "action/req" },
    { .topic_buf = topic_action_res, .topic_name = "action/res" },
    { .topic_buf = topic_log_req,    .topic_name = "log/req"    },
    { .topic_buf = topic_log_res,    .topic_name = "log/res"    },
    { .topic_buf = topic_status,     .topic_name = "status"     },
    { .topic_buf = topic_lwt,        .topic_name = "lwt"        }
};

char *sub_topics[] = { topic_action_req, topic_log_req };

esp_err_t mqtt_create_conn(mqtt_conn_t *conn){
    if(mqtt){
        return ESP_ERR_INVALID_STATE;
    }

    start_topics_(conn->client_id);
    static char ca[MQTT_BROKER_FILES_SZ], cert[MQTT_BROKER_FILES_SZ], pkey[MQTT_BROKER_FILES_SZ];

    if(load_broker_file_(ca, MQTT_BROKER_FILES_SZ, conn->ca_path) <= 0){
        return ESP_ERR_NOT_FOUND;
    }

    if(load_broker_file_(cert, MQTT_BROKER_FILES_SZ, conn->cert_path) <= 0){
        return ESP_ERR_NOT_FOUND;
    }

    if(load_broker_file_(pkey, MQTT_BROKER_FILES_SZ, conn->pkey_path) <= 0){
        return ESP_ERR_NOT_FOUND;
    }

    esp_mqtt_client_config_t conf = {
        .broker.address.uri = conn->uri,
        .broker.verification.certificate = ca,
        .broker.verification.certificate_len = strlen(ca) + 1,

        .credentials.authentication.certificate = cert,
        .credentials.authentication.certificate_len = strlen(cert) + 1,
        .credentials.authentication.key = pkey,
        .credentials.authentication.key_len = strlen(pkey) + 1,
        .credentials.client_id = conn->client_id,

        .buffer.size = MQTT_BUFFER_SIZE,

        .task.priority = 2,
        .task.stack_size = 8192,

        .session.keepalive = 60,
        .session.last_will.msg = "{\"status\": \"disconnected\"}",
        .session.last_will.msg_len = sizeof("{\"status\": \"disconnected\"}"),
        .session.last_will.qos = 0
    };

    mqtt = esp_mqtt_client_init(&conf);

    if(!mqtt){
        return ESP_ERR_INVALID_RESPONSE;
    }

    esp_err_t err = esp_mqtt_client_register_event(mqtt, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    if(err){
        esp_mqtt_client_destroy(mqtt);
        mqtt = NULL;
        return err;
    }

    err = esp_mqtt_client_start(mqtt);

    if(err){
        esp_mqtt_client_destroy(mqtt);
        mqtt = NULL;
    }

    return err;
}

int mqtt_publish(char *data, const mqtt_topics_t topic){
    int len = strlen(data);

    for(int i = 0; i < len; i++){
        if(data[i] == '\''){
            data[i] = '"';
        }
    }

    return esp_mqtt_client_publish(mqtt, all_topics[topic].topic_buf, data, len, 0, 0);
}

__mqtt_priv_lib__ void start_topics_(const char *id){
    int ntopics = MQTT_NUM_OF_TOPICS;

    for(int i = 0; i < ntopics; i++){
        sprintf(
            all_topics[i].topic_buf, 
            "%s/%s", id, 
            all_topics[i].topic_name
        );
    }
}

__mqtt_priv_lib__ int load_broker_file_(char *fbuf, size_t fbuf_sz, const char *fname){
    char path[64] = { 0 };
    snprintf(path, sizeof(path) - 1, "/lfs/%s", fname);

    FILE *file = fopen(path, "r");

    if(!file){
        return FS_ERROR_OR_FILE_NOT_FOUND;
    }

    size_t rbytes = fread(fbuf, sizeof(char), fbuf_sz - 1, file);
    fclose(file);

    if(!rbytes){
        fbuf[0] = '\0';
        return FILE_EMPTY;
    }

    fbuf[rbytes] = '\0';
    return (int)rbytes;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    esp_mqtt_event_t *event = event_data;

    switch((esp_mqtt_event_id_t)event_id){
        case MQTT_EVENT_CONNECTED:
            printf("[MQTT]> Connected\n");
            int nsub_topics = sizeof(sub_topics) / sizeof(char*);

            for(int i = 0; i < nsub_topics; i++){
                esp_mqtt_client_subscribe_single(mqtt, sub_topics[i], 0);
            }

            break;

        case MQTT_EVENT_DISCONNECTED:
            printf("[MQTT]> Disconnected\n");
            break;

        case MQTT_EVENT_DATA:
            memcpy(topic_buf, event->topic, event->topic_len);
            topic_buf[event->topic_len] = '\0';

            memcpy(data_buf, event->data, event->data_len);
            data_buf[event->data_len] = '\0';

            mqtt_callback(topic_buf, data_buf);
            break;

        case MQTT_EVENT_ERROR:
            printf("[MQTT]> Error\n");
            break;

        default:
            break;
    }
}

#undef EMPTY_BUF
