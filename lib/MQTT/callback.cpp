#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "mqtt.h"
#include "machine.h"
#include "IOs.h"
#include "ArduinoJson.h"

#define __callback_priv_lib__ static inline

#define clear_tx_buf() memset(tx_buf, 0, sizeof(tx_buf))

#define create_json(publish_tx_fn)                              \
    JsonDocument json;                                          \
    DeserializationError error = deserializeJson(json, data);   \
                                                                \
    if(error){                                                  \
        publish_tx_fn;                                          \
        return;                                                 \
    }

typedef enum {
    CB_SEL_WASHING,
    CB_SEL_DRYER,
    CB_SEL_NOTHING
} cb_machine_select_t_;

__callback_priv_lib__ void action_req_topic_(char *data);
__callback_priv_lib__ void log_req_topic_(const char *data);
__callback_priv_lib__ void publish_tx_buf_to_action_topic_(int result);
__callback_priv_lib__ void set_sw_mode_values_from_arrays(JsonDocument& json, const char *key, int start, int end);
__callback_priv_lib__ cb_machine_select_t_ machine_is_valid_(const char *mchstr);

char tx_buf[4096] = { 0 };

extern "C" void mqtt_callback(const char *topic, char *data){
#define __topic_is__(topic_id) \
    (!strncmp(topic, all_topics[topic_id].topic_buf, strlen(all_topics[topic_id].topic_buf)))

    printf("[MQTT::CB]> Topic: %s | Payload: %s\n\n\n", topic, data);

    if(__topic_is__(MQTT_ACTION_REQ_TOPIC)){
        action_req_topic_(data);
        return;
    }

    if(__topic_is__(MQTT_LOG_REQ_TOPIC)){
        log_req_topic_(data);
        return;
    }

    if(__topic_is__(MQTT_SET_SOFTWARE_MODE)){
        log_req_topic_(data);
        return;
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

    create_json(publish_tx_buf_etpjo());
    JsonVariant machine_item = json["machine"];

    if(machine_item.isNull() || !machine_item.is<const char*>()){
        json.clear();
        publish_tx_buf_ic();
        return;
    }

    const char *machine_str = machine_item.as<const char*>();
    cb_machine_select_t_ machine_select = machine_is_valid_(machine_str);

    if(machine_select == CB_SEL_NOTHING){
        json.clear();
        publish_tx_buf_ic();
        return;
    }

    publish_tx_buf_ok();

#undef publish_tx_buf_etpjo
#undef publish_tx_buf_ic
#undef publish_tx_buf_ok
}

__callback_priv_lib__ void log_req_topic_(const char *data){
    clear_tx_buf();
}

__callback_priv_lib__ void set_software_mode(const char *data){
    create_json(((void)0));
    JsonVariant machine_item = json["machine"];

    if(machine_item.isNull() || !machine_item.is<const char*>()){
        json.clear();
        return;
    }

    const char *machine_str = machine_item.as<const char*>();
    cb_machine_select_t_ machine_select = machine_is_valid_(machine_str);

    if(machine_select == CB_SEL_NOTHING){
        json.clear();
        return;
    }

    machine_t *machine = machine_select == CB_SEL_WASHING ? &washing : &dryer;
    machine_history_t history = machine_history(machine);

    if(history.state == MACHINE_BUSY && !machine->software_mode){
        json.clear();
        return;
    }

    if(json["software-mode"].is<int>()){
        bool software_mode = (bool)json["software-mode"].as<int>();

        if(!software_mode){
            machine->software_mode = false;

            if(history.state == MACHINE_BUSY){
                machine_set_state(machine, MACHINE_FREE);
            }

            json.clear();
            return;
        }

        machine->software_mode = 1;
        set_sw_mode_values_from_arrays(json, "stages", STAGE_1, STAGE_4);
        set_sw_mode_values_from_arrays(json, "dongles", DONGLE_WAS, DONGLE_DRY);
    }

    json.clear();
}

__callback_priv_lib__ void set_sw_mode_values_from_arrays(JsonDocument& json, const char *key, int start, int end){
    if(json[key].is<JsonArray>()){
        JsonArray array = json[key].as<JsonArray>();

        if(array.size() == (end - start + 1)){
            for(int i = start; i <= end; i++){
                if(array[i].is<int>()){
                    software_in[(digital_inputs_num_t)i].state = 
                        array[i].as<int>() ? true : false;
                }
            }
        }
    }
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
