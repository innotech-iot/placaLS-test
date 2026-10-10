#ifndef MQTT_HPP
#define MQTT_HPP

#include <stdint.h>

#define MQTT_SERVER_MAGIC 0x2077

typedef struct {
    const char *uri;
    const char *client_id;
} mqtt_conn_t;

typedef struct {
    char host[128];

    uint16_t port;
    uint16_t magic;
} mqtt_server_conf_t;

bool mqtt_connect(mqtt_conn_t *conn);

#endif // MQTT_HPP
