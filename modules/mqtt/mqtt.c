#include "mqtt.h"

#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "ap_dispatcher.h"
#include "ap_event.h"
#include "ap_registry.h"
#include "mqtt_mapping.h"
#include "mqtt_config.h"

static struct mosquitto *mqtt_client;


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
        ap_mqtt_mapping_find_by_topic(
            message->topic
        );

    if (mapping == NULL)
    {
        printf(
            "[MQTT] No mapping for topic: %s\n",
            message->topic
        );

        return;
    }

    if (mapping->direction ==
        AP_MQTT_DIRECTION_PUBLISH)
    {
        printf(
            "[MQTT] Mapping is publish-only: %s\n",
            message->topic
        );

        return;
    }

    const ap_object_t *object =
        ap_registry_find(
            mapping->object.id
        );

    if (object == NULL)
    {
        printf(
            "[MQTT] Object %" PRIu32 " not found\n",
            mapping->object.id
        );

        return;
    }

    if (object->object_type != AP_OBJECT_SIGNAL)
    {
        printf(
            "[MQTT] Object %" PRIu32 " is not a signal\n",
            object->id
        );

        return;
    }

    char payload[256];

    if ((size_t)message->payloadlen >= sizeof(payload))
    {
        printf(
            "[MQTT] Payload too large for topic: %s\n",
            message->topic
        );

        return;
    }

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
            {
                event.value.b = true;
            }
            else if (strcmp(payload, "false") == 0)
            {
                event.value.b = false;
            }
            else
            {
                printf(
                    "[MQTT] Invalid boolean payload: %s\n",
                    payload
                );

                return;
            }

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

            printf(
                "[MQTT] Unsupported object value type\n"
            );

            return;
    }

    printf(
        "[MQTT] Received topic '%s' -> Object %" PRIu32 "\n",
        message->topic,
        object->id
    );

    ap_dispatcher_publish(
        &event
    );
}


ap_result_t ap_mqtt_init(
    const ap_mqtt_config_t *config
)
{
    if (config == NULL)
        return AP_ERROR_INVALID_ARGUMENT;

    if (config->host == NULL ||
        config->username == NULL ||
        config->password == NULL)
    {
        return AP_ERROR_INVALID_ARGUMENT;
    }

    int result =
        mosquitto_lib_init();

    if (result != MOSQ_ERR_SUCCESS)
        return AP_ERROR_INIT_FAILED;

    mqtt_client =
        mosquitto_new(
            NULL,
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
            config->username,
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

    uint32_t mapping_count =
        ap_mqtt_mapping_count();

    for (uint32_t i = 0; i < mapping_count; i++)
    {
        const ap_mqtt_mapping_t *mapping =
            ap_mqtt_mapping_get(i);

        if (mapping == NULL)
            continue;

        if (mapping->direction ==
                AP_MQTT_DIRECTION_SUBSCRIBE ||
            mapping->direction ==
                AP_MQTT_DIRECTION_BOTH)
        {
            result =
                mosquitto_subscribe(
                    mqtt_client,
                    NULL,
                    mapping->topic,
                    0
                );

            if (result != MOSQ_ERR_SUCCESS)
            {
                printf(
                    "[MQTT] Failed to subscribe to: %s\n",
                    mapping->topic
                );

                return AP_ERROR_OPERATION_FAILED;
            }

            printf(
                "[MQTT] Subscribed to: %s\n",
                mapping->topic
            );
        }
    }

    return AP_OK;
}


ap_result_t ap_mqtt_subscribe(
    const char *topic
)
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
    const ap_event_t *event
)
{
    if (mqtt_client == NULL ||
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
        ap_mqtt_mapping_find_by_object(
            object->id
        );

    if (mapping == NULL)
    {
        printf(
            "[MQTT] No mapping for object: %" PRIu32 "\n",
            object->id
        );

        return AP_ERROR_NOT_FOUND;
    }

    if (mapping->direction ==
        AP_MQTT_DIRECTION_SUBSCRIBE)
    {
        printf(
            "[MQTT] Object %" PRIu32 " is subscribe-only\n",
            object->id
        );

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

    int result =
        mosquitto_publish(
            mqtt_client,
            NULL,
            mapping->topic,
            (int)strlen(payload),
            payload,
			mapping->qos,
			mapping->retain
        );

    if (result != MOSQ_ERR_SUCCESS)
        return AP_ERROR_OPERATION_FAILED;

    printf(
        "[MQTT] Published object %" PRIu32 " -> %s\n",
        object->id,
        mapping->topic
    );

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
    if (mqtt_client != NULL)
    {
        mosquitto_disconnect(mqtt_client);
        mosquitto_destroy(mqtt_client);
        mqtt_client = NULL;
    }

    mosquitto_lib_cleanup();
}