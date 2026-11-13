#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "mqtt.h"
#include "ArduinoJson.h"

#define __callback_priv_lib__ static inline

#define clear_tx_buf() memset(tx_buf, 0, sizeof(tx_buf))

typedef enum {
    CB_SEL_WASHING,
    CB_SEL_DRYER,
    CB_SEL_NOTHING
} cb_machine_select_t_;

__callback_priv_lib__ void action_req_topic_(char *data);
__callback_priv_lib__ void log_req_topic_(char *data);
__callback_priv_lib__ void publish_tx_buf_to_action_topic_(int result);
__callback_priv_lib__ cb_machine_select_t_ machine_is_valid_(const char *mchstr);

char tx_buf[4096] = { 0 };

void mqtt_callback(const char *topic, char *data){
#define __topic_is__(topic_id) \
    (!strncmp(topic, all_topics[topic_id].topic_buf, strlen(all_topics[topic_id].topic_buf)))

    printf("[MQTT::CB]> Topic: %s | Payload: %s\n\n\n", topic, data);

    if(__topic_is__(MQTT_ACTION_REQ_TOPIC)){
        action_req_topic_(data);
    }

    if(__topic_is__(MQTT_LOG_REQ_TOPIC)){
        log_req_topic_(data);
    }

#undef __topic_is__
}

__callback_priv_lib__ void action_req_topic_(char *data){
#define publish_tx_buf_etpjo()  \
    publish_tx_buf_to_action_topic_((int)ERROR_TO_PARSE_JSON_OBJECT)

#define publish_tx_buf_ic()     \
    publish_tx_buf_to_action_topic_((int)INVALID_COMMAND)

#define publish_tx_buf_ok()     \
    publish_tx_buf_to_action_topic_((int)OK)

    typedef enum {
        OK,
        ERROR_TO_PARSE_JSON_OBJECT = -1,
        INVALID_COMMAND            = -2,
    } result_t_;

    JsonDocument json;

    DeserializationError error = deserializeJson(json, data);

    if(error){
        publish_tx_buf_etpjo();
        return;
    }

    JsonVariant machine_item = json["machine"];

    if(machine_item.isNull() || !machine_item.is<const char*>()){
        publish_tx_buf_ic();
        return;
    }

    const char *machine_str = machine_item.as<const char*>();
    cb_machine_select_t_ machine_select = machine_is_valid_(machine_str);

    if(machine_select == CB_SEL_NOTHING){
        publish_tx_buf_ic();
        return;
    }

    publish_tx_buf_ok();

#undef publish_tx_buf_etpjo
#undef publish_tx_buf_ic
#undef publish_tx_buf_ok
}

__callback_priv_lib__ void log_req_topic_(char *data){
    clear_tx_buf();
}

__callback_priv_lib__ void publish_tx_buf_to_action_topic_(int result){
    memset(tx_buf, 0, sizeof(tx_buf));
    sprintf(tx_buf, "{'result': %d}", result);
    mqtt_publish(tx_buf, MQTT_ACTION_RES_TOPIC);
}

__callback_priv_lib__ cb_machine_select_t_ machine_is_valid_(const char *mchstr){
    size_t mchlen = strlen(mchstr);
    size_t washlen = sizeof("washing") - 1;
    size_t dryerlen = sizeof("dryer") - 1;

    if(mchlen != washlen && mchlen != dryerlen){
        return CB_SEL_NOTHING;
    }

    if(mchlen == washlen && !strncmp(mchstr, "washing", washlen)){
        return CB_SEL_WASHING;
    }

    if(mchlen == dryerlen && !strncmp(mchstr, "dryer", dryerlen)){
        return CB_SEL_DRYER;
    }

    return CB_SEL_NOTHING;
}

#undef clear_tx_buf
