#include "mqtt.h"

#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ap_dispatcher.h"
#include "ap_event.h"
#include "ap_registry.h"
#include "mqtt_mapping.h"

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


    const ap_signal_t *signal =
        ap_registry_find(
            mapping->signal_id
        );

    if (signal == NULL)
    {
        printf(
            "[MQTT] Signal %u not found\n",
            mapping->signal_id
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
        signal,
        0
    );


    switch (signal->type)
    {
        case AP_SIGNAL_BOOL:

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


        case AP_SIGNAL_INT32:

            event.value.i =
                (int32_t)strtol(
                    payload,
                    NULL,
                    10
                );

            break;


        case AP_SIGNAL_FLOAT:

            event.value.f =
                strtof(
                    payload,
                    NULL
                );

            break;


        case AP_SIGNAL_STRING:

            event.value.s = payload;

            break;


        default:

            printf(
                "[MQTT] Unsupported signal type\n"
            );

            return;
    }


    printf(
        "[MQTT] Received topic '%s' -> Signal %u\n",
        message->topic,
        signal->id
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
        mosquitto_destroy(
            mqtt_client
        );

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
        mosquitto_destroy(
            mqtt_client
        );

        mqtt_client = NULL;

        mosquitto_lib_cleanup();

        return AP_ERROR_CONNECTION;
    }


    uint32_t mapping_count =
        ap_mqtt_mapping_count();


    for (uint32_t i = 0;
         i < mapping_count;
         i++)
    {
        const ap_mqtt_mapping_t *mapping =
            ap_mqtt_mapping_get(i);


        if (mapping == NULL)
            continue;


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
        event->signal == NULL)
    {
        return AP_ERROR_INVALID_ARGUMENT;
    }


    const ap_signal_t *signal =
        event->signal;


    const ap_mqtt_mapping_t *mapping =
        ap_mqtt_mapping_find_by_signal(
            signal->id
        );


    if (mapping == NULL)
    {
        printf(
            "[MQTT] No mapping for signal: %u\n",
            signal->id
        );

        return AP_ERROR_NOT_FOUND;
    }


    char payload[256];


    switch (signal->type)
    {
        case AP_SIGNAL_BOOL:

            snprintf(
                payload,
                sizeof(payload),
                "%s",
                event->value.b
                    ? "true"
                    : "false"
            );

            break;


        case AP_SIGNAL_INT32:

            snprintf(
                payload,
                sizeof(payload),
                "%d",
                event->value.i
            );

            break;


        case AP_SIGNAL_FLOAT:

            snprintf(
                payload,
                sizeof(payload),
                "%.6f",
                event->value.f
            );

            break;


        case AP_SIGNAL_STRING:

            if (event->value.s == NULL)
                return AP_ERROR_INVALID_ARGUMENT;

            snprintf(
                payload,
                sizeof(payload),
                "%s",
                event->value.s
            );

            break;


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
            0,
            false
        );


    if (result != MOSQ_ERR_SUCCESS)
        return AP_ERROR_OPERATION_FAILED;


    printf(
        "[MQTT] Published signal %u -> %s\n",
        signal->id,
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
        mosquitto_disconnect(
            mqtt_client
        );

        mosquitto_destroy(
            mqtt_client
        );

        mqtt_client = NULL;
    }


    mosquitto_lib_cleanup();
}