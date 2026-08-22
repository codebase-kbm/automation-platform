#ifndef AP_MQTT_PLUGIN_H
#define AP_MQTT_PLUGIN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ap_plugin.h"

#ifdef __cplusplus
extern "C" {
#endif


#define AP_MQTT_MAP_FLAG_SUBSCRIBE  0x01u
#define AP_MQTT_MAP_QOS_SHIFT       1u
#define AP_MQTT_MAP_QOS_MASK        0x06u
#define AP_MQTT_MAP_FLAG_RETAIN     0x08u


/* -------------------------------------------------- */
/* MQTT Plugin Mapping                               */
/* -------------------------------------------------- */

typedef struct
{
    uint32_t object_id;
    uint8_t value_type;
    uint8_t flags;

    char *topic;

} ap_mqtt_mapping_t;


/* -------------------------------------------------- */
/* MQTT Plugin Configuration                         */
/* -------------------------------------------------- */

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


/* -------------------------------------------------- */
/* MQTT Backend API                                  */
/* -------------------------------------------------- */

typedef struct
{
    const char *host;
    uint16_t port;

    const char *user;
    const char *password;
    const char *client_id;

} ap_mqtt_backend_config_t;


typedef void (*ap_mqtt_message_callback_t)(
    const char *topic,
    const void *payload,
    size_t payload_length,
    void *user_data
);


typedef struct
{
    ap_result_t (*open)(
        const ap_mqtt_backend_config_t *config,
        ap_mqtt_message_callback_t callback,
        void *user_data
    );

    void (*close)(void);

    ap_result_t (*subscribe)(
        const char *topic,
        uint8_t qos
    );

    ap_result_t (*publish)(
        const char *topic,
        const void *payload,
        size_t payload_length,
        uint8_t qos,
        bool retain
    );

    ap_result_t (*process)(void);

} ap_mqtt_backend_t;


extern const ap_mqtt_backend_t ap_mqtt_backend;


/* -------------------------------------------------- */
/* Plugin                                             */
/* -------------------------------------------------- */

extern const ap_plugin_t ap_mqtt_plugin;


#ifdef __cplusplus
}
#endif

#endif /* AP_MQTT_PLUGIN_H */