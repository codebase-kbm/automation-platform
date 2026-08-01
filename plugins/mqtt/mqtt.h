#ifndef AP_MQTT_PLUGIN_H
#define AP_MQTT_PLUGIN_H

#include "ap_plugin.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AP_MQTT_MAP_FLAG_SUBSCRIBE  0x01u
#define AP_MQTT_MAP_QOS_SHIFT       1u
#define AP_MQTT_MAP_QOS_MASK        0x06u
#define AP_MQTT_MAP_FLAG_RETAIN     0x08u


typedef struct
{
    uint32_t object_id;
    uint8_t value_type;
    uint8_t flags;

    char *topic;

} ap_mqtt_mapping_t;


typedef struct
{
    char *host;
    uint16_t port;

    char *user;
    char *password;
    char *client_id;

    uint8_t mapping_count;
    ap_mqtt_mapping_t *mappings;

} ap_mqtt_config_t;

extern const ap_plugin_t ap_mqtt_plugin;

#ifdef __cplusplus
}
#endif

#endif /* AP_MQTT_PLUGIN_H */