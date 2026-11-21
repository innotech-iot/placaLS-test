#include <WiFi.h>

#include "mqtt.h"
#include "factory_conf.h"

static void wifi_task(void *params);

void wifi_init(){
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);
    xTaskCreate(wifi_task, "wifi", 4096, &factory_file.conf, 1, NULL);
}

static bool connect_best_registered_network(factory_conf_t *conf){
    int16_t scan_count = WiFi.scanNetworks();

    if(scan_count <= 0){
        WiFi.scanDelete();
        return false;
    }

    int best_rssi = -127;
    int best_network = -1;
    uint8_t best_bssid[6] = { 0 };
    int32_t best_channel = 0;

    wifi_data_t *networks[] = {
        &conf->wifi.network_1,
        &conf->wifi.network_2,
        &conf->wifi.network_3,
        &conf->wifi.network_4
    };

    for(int i = 0; i < scan_count; i++){
        String scanned_ssid = WiFi.SSID(i);
        int rssi = WiFi.RSSI(i);

        for(int n = 0; n < 4; n++){
            if(!(conf->wifi.net_useds & (1U << n))){
                continue;
            }

            if(scanned_ssid == networks[n]->ssid){
                if(rssi > best_rssi){
                    best_rssi = rssi;
                    best_network = n;

                    const uint8_t *bssid = WiFi.BSSID(i);
                    memcpy(best_bssid, bssid, sizeof(best_bssid));
                    best_channel = WiFi.channel(i);
                }

                break;
            }
        }
    }

    if(best_network < 0){
        WiFi.scanDelete();
        return false;
    }

    wifi_data_t *selected = networks[best_network];

    WiFi.disconnect(false);
    delay(100);

    WiFi.begin(
        selected->ssid,
        selected->pswd,
        best_channel,
        best_bssid
    );

    WiFi.scanDelete();
    return true;
}

static const char *generate_device_id(){
    static char device_id[64];

    uint8_t mac[6];
    WiFi.macAddress(mac);

    snprintf(
        device_id,
        sizeof(device_id),
        "laundry-%02X%02X%02X%02X%02X%02X",
        mac[0], mac[1], mac[2],
        mac[3], mac[4], mac[5]
    );

    return device_id;
}

static void connect_to_broker(factory_conf_t *conf){
    mqtt_conn_t conn = { 0 };
    char uri[256] = { 0 };

    sprintf(
        uri, "%s://%s:%d", 
        conf->broker.has_tls ? "mqtts" : "mqtt",
        conf->broker.hostname, 
        conf->broker.port
    );

    conn.uri = uri;
    conn.client_id = generate_device_id();
    
    if(conf->broker.has_tls){
        conn.ca_path   = "/lfs/broker/ca.pem";
        conn.cert_path = "/lfs/broker/certificate.pem";
        conn.pkey_path = "/lfs/broker/private.key";
    }

    printf("mqtt err: %d\n\n", (int)mqtt_connection(&conn));
}

static void wifi_task(void *params){
    factory_conf_t *conf = (factory_conf_t*)params;

    for(;;){
        if(WiFi.status() != WL_CONNECTED){
            connect_best_registered_network(conf);
            uint32_t start = millis();

            while(WiFi.status() != WL_CONNECTED && millis() - start < 10000){
                vTaskDelay(pdMS_TO_TICKS(100));
            }

            if(WiFi.status() != WL_CONNECTED){
                WiFi.disconnect(false);
                vTaskDelay(pdMS_TO_TICKS(5000));
            }

            else {
                /** MQTT connection create */
                connect_to_broker(conf);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
