#include "mqtt_adapter.h"

#include <mosquitto.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>

#include "ap_dispatcher.h"
#include "ap_event.h"
#include "ap_registry.h"


static struct mosquitto *mqtt_client;
static const ap_mqtt_config_t *mqtt_config;


static const ap_mqtt_mapping_t *
find_mapping_by_topic(
    const char *topic)
{
    if (mqtt_config == NULL ||
        topic == NULL)
    {
        return NULL;
    }

    for (uint8_t i = 0;
         i < mqtt_config->mapping_count;
         i++)
    {
        const ap_mqtt_mapping_t *mapping =
            &mqtt_config->mappings[i];

        if (mapping->topic == NULL)
            continue;

        if (strcmp(mapping->topic, topic) == 0)
            return mapping;
    }

    return NULL;
}


static const ap_mqtt_mapping_t *
find_mapping_by_object(
    uint32_t object_id)
{
    if (mqtt_config == NULL)
        return NULL;

    for (uint8_t i = 0;
         i < mqtt_config->mapping_count;
         i++)
    {
        const ap_mqtt_mapping_t *mapping =
            &mqtt_config->mappings[i];

        if (mapping->object_id == object_id)
            return mapping;
    }

    return NULL;
}


static void ap_mqtt_message_callback(
    struct mosquitto *mosq,
    void *userdata,
    const struct mosquitto_message *message)
{
    (void)mosq;
    (void)userdata;

    if (message == NULL ||
        message->topic == NULL ||
        message->payload == NULL)
    {
        return;
    }

    const ap_mqtt_mapping_t *mapping =
        find_mapping_by_topic(
            message->topic
        );

    if (mapping == NULL)
        return;

    if (!(mapping->flags & AP_MQTT_MAP_FLAG_SUBSCRIBE))
        return;

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

    if ((size_t)message->payloadlen >= sizeof(payload))
        return;

    memcpy(
        payload,
        message->payload,
        (size_t)message->payloadlen
    );

    payload[message->payloadlen] = '\0';

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


ap_result_t ap_mqtt_init(
    const ap_mqtt_config_t *config)
{
    if (config == NULL ||
        config->host == NULL ||
        config->user == NULL ||
        config->password == NULL ||
        config->mappings == NULL)
    {
        return AP_ERROR_INVALID_ARGUMENT;
    }

    int result =
        mosquitto_lib_init();

    if (result != MOSQ_ERR_SUCCESS)
        return AP_ERROR_INIT_FAILED;

    mqtt_client =
        mosquitto_new(
            config->client_id,
            true,
            NULL
        );

    if (mqtt_client == NULL)
    {
        mosquitto_lib_cleanup();
        return AP_ERROR_INIT_FAILED;
    }

    result =
        mosquitto_username_pw_set(
            mqtt_client,
            config->user,
            config->password
        );

    if (result != MOSQ_ERR_SUCCESS)
    {
        mosquitto_destroy(mqtt_client);
        mqtt_client = NULL;
        mosquitto_lib_cleanup();

        return AP_ERROR_AUTHENTICATION;
    }

    mosquitto_message_callback_set(
        mqtt_client,
        ap_mqtt_message_callback
    );

    result =
        mosquitto_connect(
            mqtt_client,
            config->host,
            config->port,
            60
        );

    if (result != MOSQ_ERR_SUCCESS)
    {
        mosquitto_destroy(mqtt_client);
        mqtt_client = NULL;
        mosquitto_lib_cleanup();

        return AP_ERROR_CONNECTION;
    }

    mqtt_config = config;

    for (uint8_t i = 0;
         i < config->mapping_count;
         i++)
    {
        const ap_mqtt_mapping_t *mapping =
            &config->mappings[i];

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
            mosquitto_subscribe(
                mqtt_client,
                NULL,
                mapping->topic,
                qos
            );

        if (result != MOSQ_ERR_SUCCESS)
        {
            mqtt_config = NULL;

            mosquitto_disconnect(mqtt_client);
            mosquitto_destroy(mqtt_client);
            mqtt_client = NULL;
            mosquitto_lib_cleanup();

            return AP_ERROR_OPERATION_FAILED;
        }
    }

    return AP_OK;
}


ap_result_t ap_mqtt_subscribe(
    const char *topic)
{
    if (mqtt_client == NULL ||
        topic == NULL)
    {
        return AP_ERROR_INVALID_ARGUMENT;
    }

    int result =
        mosquitto_subscribe(
            mqtt_client,
            NULL,
            topic,
            0
        );

    if (result != MOSQ_ERR_SUCCESS)
        return AP_ERROR_OPERATION_FAILED;

    return AP_OK;
}


ap_result_t ap_mqtt_publish_event(
    const ap_event_t *event)
{
    if (mqtt_client == NULL ||
        mqtt_config == NULL ||
        event == NULL ||
        event->object == NULL)
    {
        return AP_ERROR_INVALID_ARGUMENT;
    }

    const ap_object_t *object =
        event->object;

    if (object->object_type != AP_OBJECT_SIGNAL)
        return AP_ERROR_INVALID_TYPE;

    const ap_mqtt_mapping_t *mapping =
        find_mapping_by_object(
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

    int result =
        mosquitto_publish(
            mqtt_client,
            NULL,
            mapping->topic,
            (int)strlen(payload),
            payload,
            qos,
            retain
        );

    if (result != MOSQ_ERR_SUCCESS)
        return AP_ERROR_OPERATION_FAILED;

    return AP_OK;
}


ap_result_t ap_mqtt_process(void)
{
    if (mqtt_client == NULL)
        return AP_ERROR_INVALID_ARGUMENT;

    int result =
        mosquitto_loop(
            mqtt_client,
            0,
            1
        );

    if (result != MOSQ_ERR_SUCCESS)
        return AP_ERROR_OPERATION_FAILED;

    return AP_OK;
}


void ap_mqtt_shutdown(void)
{
    mqtt_config = NULL;

    if (mqtt_client != NULL)
    {
        mosquitto_disconnect(mqtt_client);
        mosquitto_destroy(mqtt_client);
        mqtt_client = NULL;
    }

    mosquitto_lib_cleanup();
}