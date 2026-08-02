#include "mqtt.h"
#include <mosquitto.h>
#include <stddef.h>
#include <string.h>

static struct mosquitto *mqtt_client;
static ap_mqtt_message_callback_t mqtt_callback;
static void *mqtt_callback_user_data;

/* -------------------------------------------------- */
/* Mosquitto callback                                 */
/* -------------------------------------------------- */

static void ap_mosquitto_message_callback(
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

    if (mqtt_callback == NULL)
        return;

    mqtt_callback(
        message->topic,
        message->payload,
        (size_t)message->payloadlen,
        mqtt_callback_user_data
    );
}


/* -------------------------------------------------- */
/* Backend open                                      */
/* -------------------------------------------------- */

static ap_result_t ap_mosquitto_open(
    const ap_mqtt_backend_config_t *config,
    ap_mqtt_message_callback_t callback,
    void *user_data)
{
    int result;

    if (config == NULL ||
        config->host == NULL)
    {
        return AP_ERROR_INVALID_ARGUMENT;
    }

    result = mosquitto_lib_init();

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

    if (config->user != NULL ||
        config->password != NULL)
    {
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
    }

    mqtt_callback = callback;
    mqtt_callback_user_data = user_data;

    mosquitto_message_callback_set(
        mqtt_client,
        ap_mosquitto_message_callback
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
        mqtt_callback = NULL;
        mqtt_callback_user_data = NULL;

        mosquitto_destroy(mqtt_client);
        mqtt_client = NULL;

        mosquitto_lib_cleanup();

        return AP_ERROR_CONNECTION;
    }

    return AP_OK;
}


/* -------------------------------------------------- */
/* Subscribe                                         */
/* -------------------------------------------------- */

static ap_result_t ap_mosquitto_subscribe(
    const char *topic,
    uint8_t qos)
{
    int result;

    if (mqtt_client == NULL ||
        topic == NULL)
    {
        return AP_ERROR_INVALID_ARGUMENT;
    }

    result =
        mosquitto_subscribe(
            mqtt_client,
            NULL,
            topic,
            qos
        );

    if (result != MOSQ_ERR_SUCCESS)
        return AP_ERROR_OPERATION_FAILED;

    return AP_OK;
}


/* -------------------------------------------------- */
/* Publish                                           */
/* -------------------------------------------------- */

static ap_result_t ap_mosquitto_publish(
    const char *topic,
    const void *payload,
    size_t payload_length,
    uint8_t qos,
    bool retain)
{
    int result;

    if (mqtt_client == NULL ||
        topic == NULL ||
        (payload == NULL && payload_length > 0))
    {
        return AP_ERROR_INVALID_ARGUMENT;
    }

    result =
        mosquitto_publish(
            mqtt_client,
            NULL,
            topic,
            (int)payload_length,
            payload,
            qos,
            retain
        );

    if (result != MOSQ_ERR_SUCCESS)
        return AP_ERROR_OPERATION_FAILED;

    return AP_OK;
}


/* -------------------------------------------------- */
/* Process                                           */
/* -------------------------------------------------- */

static ap_result_t ap_mosquitto_process(void)
{
    int result;

    if (mqtt_client == NULL)
        return AP_ERROR_INVALID_ARGUMENT;

    result =
        mosquitto_loop(
            mqtt_client,
            0,
            1
        );

    if (result != MOSQ_ERR_SUCCESS)
        return AP_ERROR_OPERATION_FAILED;

    return AP_OK;
}


/* -------------------------------------------------- */
/* Close                                             */
/* -------------------------------------------------- */

static void ap_mosquitto_close(void)
{
    mqtt_callback = NULL;
    mqtt_callback_user_data = NULL;

    if (mqtt_client != NULL)
    {
        mosquitto_disconnect(mqtt_client);
        mosquitto_destroy(mqtt_client);
        mqtt_client = NULL;
    }

    mosquitto_lib_cleanup();
}


/* -------------------------------------------------- */
/* Backend definition                                */
/* -------------------------------------------------- */

const ap_mqtt_backend_t ap_mqtt_backend =
{
    .open = ap_mosquitto_open,
    .close = ap_mosquitto_close,

    .subscribe = ap_mosquitto_subscribe,
    .publish = ap_mosquitto_publish,

    .process = ap_mosquitto_process
};
