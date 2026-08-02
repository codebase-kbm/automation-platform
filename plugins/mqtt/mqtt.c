#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

#include "mqtt.h"
#include "ap_config_payload_reader.h"
#include "ap_module.h"
#include "ap_plugin.h"
#include "ap_result.h"
#include "ap_dispatcher.h"
#include "ap_event.h"
#include "ap_registry.h"


#define AP_MQTT_CONFIG_VERSION 1u

static ap_mqtt_config_t mqtt_config;

static ap_result_t ap_mqtt_publish_event(const ap_event_t *event);

/* -------------------------------------------------- */
/* Config cleanup                                     */
/* -------------------------------------------------- */

static void ap_mqtt_config_clear(
    ap_mqtt_config_t *config)
{
    if (config == NULL)
        return;

    free(config->host);
    free(config->user);
    free(config->password);
    free(config->client_id);

    if (config->mappings != NULL)
    {
        for (uint8_t i = 0;
             i < config->mapping_count;
             i++)
        {
            free(config->mappings[i].topic);
        }

        free(config->mappings);
    }

    memset(
        config,
        0,
        sizeof(*config)
    );
}


/* -------------------------------------------------- */
/* Mapping parser                                     */
/* -------------------------------------------------- */

static int ap_mqtt_read_mapping(
    ap_config_payload_reader_t *reader,
    ap_mqtt_mapping_t *mapping)
{
    if (reader == NULL ||
        mapping == NULL)
    {
        return -1;
    }

    memset(
        mapping,
        0,
        sizeof(*mapping)
    );

    if (ap_config_read_string(
            reader,
            &mapping->topic) != 0)
    {
        return -1;
    }

    if (ap_config_read_u32_le(
            reader,
            &mapping->object_id) != 0)
    {
        free(mapping->topic);
        mapping->topic = NULL;
        return -1;
    }

    if (ap_config_read_u8(
            reader,
            &mapping->value_type) != 0)
    {
        free(mapping->topic);
        mapping->topic = NULL;
        return -1;
    }

    if (ap_config_read_u8(
            reader,
            &mapping->flags) != 0)
    {
        free(mapping->topic);
        mapping->topic = NULL;
        return -1;
    }

    return 0;
}


/* -------------------------------------------------- */
/* Mapping lookup                                    */
/* -------------------------------------------------- */

static const ap_mqtt_mapping_t *
ap_mqtt_find_mapping_by_topic(
    const char *topic)
{
    if (topic == NULL)
        return NULL;

    for (uint8_t i = 0;
         i < mqtt_config.mapping_count;
         i++)
    {
        const ap_mqtt_mapping_t *mapping =
            &mqtt_config.mappings[i];

        if (mapping->topic == NULL)
            continue;

        if (strcmp(mapping->topic, topic) == 0)
            return mapping;
    }

    return NULL;
}


static const ap_mqtt_mapping_t *
ap_mqtt_find_mapping_by_object(
    uint32_t object_id)
{
    for (uint8_t i = 0;
         i < mqtt_config.mapping_count;
         i++)
    {
        const ap_mqtt_mapping_t *mapping =
            &mqtt_config.mappings[i];

        if (mapping->object_id == object_id)
            return mapping;
    }

    return NULL;
}


/* -------------------------------------------------- */
/* MQTT receive callback                             */
/* -------------------------------------------------- */

static void ap_mqtt_message_callback(
    const char *topic,
    const void *payload_data,
    size_t payload_length,
    void *user_data)
{
    (void)user_data;

    if (topic == NULL ||
        payload_data == NULL)
    {
        return;
    }

    const ap_mqtt_mapping_t *mapping =
        ap_mqtt_find_mapping_by_topic(
            topic
        );

    if (mapping == NULL)
        return;

    if (!(mapping->flags &
          AP_MQTT_MAP_FLAG_SUBSCRIBE))
    {
        return;
    }

    const ap_object_t *object =
        ap_registry_find(
            mapping->object_id
        );

    if (object == NULL ||
        object->object_type != AP_OBJECT_SIGNAL)
    {
        return;
    }

    char payload[256];

    if (payload_length >= sizeof(payload))
        return;

    memcpy(
        payload,
        payload_data,
        payload_length
    );

    payload[payload_length] = '\0';

    ap_event_t event;

    ap_event_init(
        &event,
        object,
        0
    );

    switch (object->value_type)
    {
        case AP_VALUE_BOOL:

            if (strcmp(payload, "true") == 0)
                event.value.b = true;

            else if (strcmp(payload, "false") == 0)
                event.value.b = false;

            else
                return;

            break;


        case AP_VALUE_INT32:

            event.value.i =
                (int32_t)strtol(
                    payload,
                    NULL,
                    10
                );

            break;


        case AP_VALUE_FLOAT:

            event.value.f =
                strtof(
                    payload,
                    NULL
                );

            break;


        case AP_VALUE_STRING:

            event.value.s = payload;

            break;


        case AP_VALUE_NONE:
        default:

            return;
    }

    ap_dispatcher_publish(
        &event
    );
}


/* -------------------------------------------------- */
/* Plugin load                                       */
/* -------------------------------------------------- */

static ap_result_t ap_mqtt_module_load(
    const ap_config_object_t *object)
{
    ap_config_payload_reader_t reader;

    uint8_t module_type;
    uint8_t version;
    uint8_t mapping_count;

    if (object == NULL ||
        object->payload == NULL)
    {
        return AP_ERROR_INVALID_ARGUMENT;
    }

    reader.data = object->payload;
    reader.length = object->header.payload_length;
    reader.offset = 0;

    if (ap_config_read_u8(
            &reader,
            &module_type) != 0 ||
        ap_config_read_u8(
            &reader,
            &version) != 0)
    {
        return AP_ERROR_INVALID_ARGUMENT;
    }

    if (module_type != AP_MODULE_MQTT)
        return AP_ERROR_INVALID_ARGUMENT;

    if (version != AP_MQTT_CONFIG_VERSION)
        return AP_ERROR_INVALID_ARGUMENT;

    ap_mqtt_config_clear(
        &mqtt_config
    );

    if (ap_config_read_string(
            &reader,
            &mqtt_config.host) != 0 ||
        ap_config_read_u16_le(
            &reader,
            &mqtt_config.port) != 0 ||
        ap_config_read_string(
            &reader,
            &mqtt_config.user) != 0 ||
        ap_config_read_string(
            &reader,
            &mqtt_config.password) != 0 ||
        ap_config_read_string(
            &reader,
            &mqtt_config.client_id) != 0 ||
        ap_config_read_u8(
            &reader,
            &mapping_count) != 0)
    {
        ap_mqtt_config_clear(
            &mqtt_config
        );

        return AP_ERROR_INVALID_ARGUMENT;
    }

    mqtt_config.mapping_count = mapping_count;

    if (mapping_count > 0)
    {
        mqtt_config.mappings =
            calloc(
                mapping_count,
                sizeof(*mqtt_config.mappings)
            );

        if (mqtt_config.mappings == NULL)
        {
            ap_mqtt_config_clear(
                &mqtt_config
            );

            return AP_ERROR_OUT_OF_MEMORY;
        }

        for (uint8_t i = 0;
             i < mapping_count;
             i++)
        {
            if (ap_mqtt_read_mapping(
                    &reader,
                    &mqtt_config.mappings[i]) != 0)
            {
                ap_mqtt_config_clear(
                    &mqtt_config
                );

                return AP_ERROR_INVALID_ARGUMENT;
            }
        }
    }

    if (reader.offset != reader.length)
    {
        ap_mqtt_config_clear(
            &mqtt_config
        );

        return AP_ERROR_INVALID_ARGUMENT;
    }

    return AP_OK;
}


/* -------------------------------------------------- */
/* Plugin init                                       */
/* -------------------------------------------------- */

static void ap_mqtt_event_handler(
    const ap_event_t *event)
{
    (void)ap_mqtt_publish_event(event);
}


static ap_result_t ap_mqtt_module_init(void)
{
    ap_mqtt_backend_config_t backend_config;

    backend_config.host =
        mqtt_config.host;

    backend_config.port =
        mqtt_config.port;

    backend_config.user =
        mqtt_config.user;

    backend_config.password =
        mqtt_config.password;

    backend_config.client_id =
        mqtt_config.client_id;

    ap_result_t result =
        ap_mqtt_backend.open(
            &backend_config,
            ap_mqtt_message_callback,
            NULL
        );

    if (result != AP_OK)
        return result;

    for (uint8_t i = 0;
         i < mqtt_config.mapping_count;
         i++)
    {
        const ap_mqtt_mapping_t *mapping =
            &mqtt_config.mappings[i];

        if (!(mapping->flags &
              AP_MQTT_MAP_FLAG_SUBSCRIBE))
        {
            continue;
        }

        uint8_t qos =
            (mapping->flags &
             AP_MQTT_MAP_QOS_MASK)
            >> AP_MQTT_MAP_QOS_SHIFT;

        result =
            ap_mqtt_backend.subscribe(
                mapping->topic,
                qos
            );

        if (result != AP_OK)
        {
            ap_mqtt_backend.close();
            return result;
        }
    }

    result =
        ap_dispatcher_register(
            ap_mqtt_event_handler
        );

    if (result != AP_OK)
    {
        ap_mqtt_backend.close();
        return result;
    }

    return AP_OK;
}


/* -------------------------------------------------- */
/* Plugin process                                    */
/* -------------------------------------------------- */

static ap_result_t ap_mqtt_module_process(void)
{
    return ap_mqtt_backend.process();
}


/* -------------------------------------------------- */
/* Plugin shutdown                                   */
/* -------------------------------------------------- */

static void ap_mqtt_module_shutdown(void)
{
    ap_mqtt_backend.close();

    ap_mqtt_config_clear(
        &mqtt_config
    );
}


/* -------------------------------------------------- */
/* Core → MQTT                                       */
/* -------------------------------------------------- */

static ap_result_t ap_mqtt_publish_event(
    const ap_event_t *event)
{
    if (event == NULL ||
        event->object == NULL)
    {
        return AP_ERROR_INVALID_ARGUMENT;
    }

    const ap_object_t *object =
        event->object;

    if (object->object_type != AP_OBJECT_SIGNAL)
        return AP_ERROR_INVALID_TYPE;

    const ap_mqtt_mapping_t *mapping =
        ap_mqtt_find_mapping_by_object(
            object->id
        );

    if (mapping == NULL)
        return AP_ERROR_NOT_FOUND;

    if (mapping->flags &
        AP_MQTT_MAP_FLAG_SUBSCRIBE)
    {
        return AP_ERROR_NOT_SUPPORTED;
    }

    char payload[256];

    switch (object->value_type)
    {
        case AP_VALUE_BOOL:

            snprintf(
                payload,
                sizeof(payload),
                "%s",
                event->value.b
                    ? "true"
                    : "false"
            );

            break;


        case AP_VALUE_INT32:

            snprintf(
                payload,
                sizeof(payload),
                "%" PRId32,
                event->value.i
            );

            break;


        case AP_VALUE_FLOAT:

            snprintf(
                payload,
                sizeof(payload),
                "%.6f",
                event->value.f
            );

            break;


        case AP_VALUE_STRING:

            if (event->value.s == NULL)
                return AP_ERROR_INVALID_ARGUMENT;

            snprintf(
                payload,
                sizeof(payload),
                "%s",
                event->value.s
            );

            break;


        case AP_VALUE_NONE:
        default:

            return AP_ERROR_NOT_SUPPORTED;
    }

    uint8_t qos =
        (mapping->flags &
         AP_MQTT_MAP_QOS_MASK)
        >> AP_MQTT_MAP_QOS_SHIFT;

    bool retain =
        (mapping->flags &
         AP_MQTT_MAP_FLAG_RETAIN) != 0;

    return ap_mqtt_backend.publish(
        mapping->topic,
        payload,
        strlen(payload),
        qos,
        retain
    );
}


/* -------------------------------------------------- */
/* Plugin definition                                 */
/* -------------------------------------------------- */

const ap_plugin_t ap_mqtt_plugin =
{
    .type = AP_MODULE_MQTT,
    .name = "mqtt",

    .dependencies = NULL,
    .dependency_count = 0,

    .load = ap_mqtt_module_load,
    .init = ap_mqtt_module_init,
    .process = ap_mqtt_module_process,
    .shutdown = ap_mqtt_module_shutdown
};


AP_PLUGIN_REGISTER(ap_mqtt_plugin);
