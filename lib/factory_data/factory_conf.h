#ifndef __libfactory_conf_factory_conf_h__
#define __libfactory_conf_factory_conf_h__

#include "esp_wifi.h"

#define FACTORY_CONF_MAGIC   0x20771984
#define FACTORY_CONF_VERSION 1

#define MAX_MQTT_HOSTNAME_LEN 128
#define MAX_PUMP              7

typedef enum {
    FACTORY_STAGE_0, /** No Wi-Fi/Broker save */
    FACTORY_STAGE_1  /** Register in Broker stage */
} factory_stage_t;

typedef struct {
    char ssid[MAX_SSID_LEN + 1];
    char pswd[MAX_PASSPHRASE_LEN + 1];
} wifi_data_t;

typedef struct {
    struct {
        wifi_data_t network_1;
        wifi_data_t network_2;
        wifi_data_t network_3;
        wifi_data_t network_4;
        uint8_t     net_useds; /** Bitmap to check which networks are used */
    } wifi;

    struct {
        char     hostname[MAX_MQTT_HOSTNAME_LEN + 1];
        uint16_t port;
        uint8_t  has_tls;
    } broker;

    struct {
        struct {
            int64_t activation_time[MAX_PUMP];
        } pumps;

        struct {
            uint32_t operating;
            uint32_t idle;
        } current;
    } hardware;
} factory_conf_t;

typedef struct {
    factory_stage_t stage;
    uint32_t        magic;
    uint32_t        version;
    factory_conf_t  conf;
} factory_file_t;

extern factory_file_t factory_file;

#endif // __libfactory_conf_factory_conf_h__
