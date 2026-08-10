#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

#include "influx.h"
#include "http.h"
#include "ap_config_payload_reader.h"
#include "ap_plugin.h"
#include "ap_result.h"
#include "ap_dispatcher.h"
#include "ap_event.h"
#include "ap_registry.h"

#define AP_INFLUX_CONFIG_VERSION 1u

static ap_influx_config_t influx_config;

static void *influx_http_context;


/* -------------------------------------------------- */
/* Config cleanup                                     */
/* -------------------------------------------------- */

static void ap_influx_config_clear(
    ap_influx_config_t *config)
{
    if (config == NULL)
        return;

    free(config->host);
    free(config->org);
    free(config->token);
    free(config->bucket);

    if (config->mappings != NULL)
    {
        for (uint8_t i = 0;
             i < config->mapping_count;
             i++)
        {
            free(config->mappings[i].name);
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

static int ap_influx_read_mapping(
    ap_config_payload_reader_t *reader,
    ap_influx_mapping_t *mapping)
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

    if (ap_config_read_u32_le(
            reader,
            &mapping->object_id) != 0)
    {
        return -1;
    }

    if (ap_config_read_u8(
            reader,
            &mapping->value_type) != 0)
    {
        return -1;
    }

    if (ap_config_read_string(
            reader,
            &mapping->name) != 0)
    {
        return -1;
    }

    return 0;
}


/* -------------------------------------------------- */
/* Mapping lookup                                     */
/* -------------------------------------------------- */

static const ap_influx_mapping_t *
ap_influx_find_mapping(
    uint32_t object_id)
{
    for (uint8_t i = 0;
         i < influx_config.mapping_count;
         i++)
    {
        const ap_influx_mapping_t *mapping =
            &influx_config.mappings[i];

        if (mapping->object_id == object_id)
            return mapping;
    }

    return NULL;
}


/* -------------------------------------------------- */
/* Plugin configuration                               */
/* -------------------------------------------------- */

static ap_result_t ap_influx_plugin_load(
    const ap_config_object_t *object)
{
    ap_config_payload_reader_t reader;

    uint8_t plugin_type;
    uint8_t version;
    uint8_t mapping_count;

    if (object == NULL ||
        object->payload == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_CONFIG_READER,
            AP_PLUGIN_INFLUX,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    reader.data =
        object->payload;

    reader.length =
        object->header.payload_length;

    reader.offset = 0;

    if (ap_config_read_u8(
            &reader,
            &plugin_type) != 0 ||
        ap_config_read_u8(
            &reader,
            &version) != 0)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_CONFIG_READER,
            AP_PLUGIN_INFLUX,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    if (plugin_type != AP_PLUGIN_INFLUX)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_CONFIG_READER,
            AP_PLUGIN_INFLUX,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    if (version != AP_INFLUX_CONFIG_VERSION)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_CONFIG_READER,
            AP_PLUGIN_INFLUX,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    ap_influx_config_clear(
        &influx_config
    );

    if (ap_config_read_string(
            &reader,
            &influx_config.host) != 0 ||
        ap_config_read_u16_le(
            &reader,
            &influx_config.port) != 0 ||
        ap_config_read_string(
            &reader,
            &influx_config.org) != 0 ||
        ap_config_read_string(
            &reader,
            &influx_config.token) != 0 ||
        ap_config_read_string(
            &reader,
            &influx_config.bucket) != 0 ||
        ap_config_read_u8(
            &reader,
            &mapping_count) != 0)
    {
        ap_influx_config_clear(
            &influx_config
        );

        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_CONFIG_READER,
            AP_PLUGIN_INFLUX,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    influx_config.mapping_count =
        mapping_count;

    if (mapping_count > 0)
    {
        influx_config.mappings =
            calloc(
                mapping_count,
                sizeof(*influx_config.mappings)
            );

        if (influx_config.mappings == NULL)
        {
            ap_influx_config_clear(
                &influx_config
            );

            return AP_RESULT_MAKE(
                AP_RESULT_SOURCE_PLUGIN,
                AP_COMPONENT_CONFIG_READER,
                AP_PLUGIN_INFLUX,
                AP_ERROR_OUT_OF_MEMORY
            );
        }

        for (uint8_t i = 0;
             i < mapping_count;
             i++)
        {
            if (ap_influx_read_mapping(
                    &reader,
                    &influx_config.mappings[i]) != 0)
            {
                ap_influx_config_clear(
                    &influx_config
                );

                return AP_RESULT_MAKE(
                    AP_RESULT_SOURCE_PLUGIN,
                    AP_COMPONENT_CONFIG_READER,
                    AP_PLUGIN_INFLUX,
                    AP_ERROR_INVALID_ARGUMENT
                );
            }

            const ap_object_t *object_ref;

            ap_result_t result =
                ap_registry_get_or_create(
                    influx_config.mappings[i].object_id,
                    influx_config.mappings[i].value_type,
                    &object_ref
                );

            if (result != AP_OK)
            {
                ap_influx_config_clear(
                    &influx_config
                );

                return result;
            }
        }
    }

    if (reader.offset != reader.length)
    {
        ap_influx_config_clear(
            &influx_config
        );

        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_CONFIG_READER,
            AP_PLUGIN_INFLUX,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    return AP_OK;
}


/* -------------------------------------------------- */
/* Line protocol                                      */
/* -------------------------------------------------- */
static int ap_influx_escape_measurement(
    const char *source,
    char *buffer,
    size_t buffer_size)
{
    size_t offset = 0;

    if (source == NULL ||
        buffer == NULL ||
        buffer_size == 0)
    {
        return -1;
    }

    for (const char *p = source;
         *p != '\0';
         p++)
    {
        if (*p == ' ' ||
            *p == ',' ||
            *p == '=' ||
            *p == '\\')
        {
            if (offset + 2 >= buffer_size)
                return -1;

            buffer[offset++] = '\\';
        }
        else
        {
            if (offset + 1 >= buffer_size)
                return -1;
        }

        buffer[offset++] = *p;
    }

    buffer[offset] = '\0';

    return 0;
}

static int ap_influx_build_line(
    const ap_event_t *event,
    const ap_influx_mapping_t *mapping,
    char *buffer,
    size_t buffer_size)
{
    char measurement[256];
    int length;

    if (event == NULL ||
        event->object == NULL ||
        mapping == NULL ||
        mapping->name == NULL ||
        buffer == NULL ||
        buffer_size == 0)
    {
        return -1;
    }

    if (ap_influx_escape_measurement(
            mapping->name,
            measurement,
            sizeof(measurement)) != 0)
    {
        return -1;
    }

    switch (event->object->value_type)
    {
        case AP_VALUE_BOOL:

            length =
                snprintf(
                    buffer,
                    buffer_size,
                    "%s value=%s",
                    measurement,
                    event->value.b
                        ? "true"
                        : "false"
                );

            break;

        case AP_VALUE_INT32:

            length =
                snprintf(
                    buffer,
                    buffer_size,
                    "%s value=%" PRId32 "i",
                    measurement,
                    event->value.i
                );

            break;

        case AP_VALUE_FLOAT:

            length =
                snprintf(
                    buffer,
                    buffer_size,
                    "%s value=%.6f",
                    measurement,
                    event->value.f
                );

            break;

        default:

            return -1;
    }

    if (length < 0 ||
        (size_t)length >= buffer_size)
    {
        return -1;
    }

    return 0;
}


/* -------------------------------------------------- */
/* Event handler                                      */
/* -------------------------------------------------- */

static ap_result_t ap_influx_event_handler(
    const ap_event_t *event)
{
    char line[512];

    if (event == NULL ||
        event->object == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_DISPATCHER,
            AP_PLUGIN_INFLUX,
            AP_ERROR_OPERATION_FAILED);
    }

    if (event->object->object_type != AP_OBJECT_SIGNAL)
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_DISPATCHER,
            AP_PLUGIN_INFLUX,
            AP_ERROR_OPERATION_FAILED);

    const ap_influx_mapping_t *mapping =
        ap_influx_find_mapping(
            event->object->id
        );

    if (mapping == NULL)
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_DISPATCHER,
            AP_PLUGIN_INFLUX,
            AP_ERROR_OPERATION_FAILED);

    if (ap_influx_build_line(
            event,
            mapping,
            line,
            sizeof(line)) != 0)
    {
            return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_DISPATCHER,
            AP_PLUGIN_INFLUX,
            AP_ERROR_OPERATION_FAILED);
    }

    char url[1024];

    int url_length =
        snprintf(
            url,
            sizeof(url),
            "http://%s:%u/api/v2/write?org=%s&bucket=%s&precision=ms",
            influx_config.host,
            (unsigned int)influx_config.port,
            influx_config.org,
            influx_config.bucket
        );

    if (url_length < 0 ||
        (size_t)url_length >= sizeof(url))
    {
            return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_DISPATCHER,
            AP_PLUGIN_INFLUX,
            AP_ERROR_OPERATION_FAILED);
    }

    char authorization[1024];

    int auth_length =
        snprintf(
            authorization,
            sizeof(authorization),
            "Token %s",
            influx_config.token
        );

    if (auth_length < 0 ||
        (size_t)auth_length >= sizeof(authorization))
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_DISPATCHER,
            AP_PLUGIN_INFLUX,
            AP_ERROR_OPERATION_FAILED);
    }

    const ap_http_header_t headers[] =
    {
        {
            .name = "Authorization",
            .value = authorization
        },
        {
            .name = "Content-Type",
            .value = "text/plain; charset=utf-8"
        }
    };

    ap_http_request_t request =
    {
        .method = AP_HTTP_METHOD_POST,
        .url = url,
        .headers = headers,
        .header_count =
            sizeof(headers) /
            sizeof(headers[0]),
        .body =
            (const uint8_t *)line,
        .body_length =
            strlen(line)
    };

    ap_http_response_t response;

    ap_result_t result =
        ap_http_backend.request(
            influx_http_context,
            &request,
            &response
        );

    if (result != AP_OK)
            return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_DISPATCHER,
            AP_PLUGIN_INFLUX,
            AP_ERROR_OPERATION_FAILED
        );

    ap_http_backend.response_free(
        influx_http_context,
        &response
    );

    return AP_OK;
}


/* -------------------------------------------------- */
/* Plugin init                                        */
/* -------------------------------------------------- */

static ap_result_t ap_influx_plugin_init(void)
{
    ap_result_t result;

    influx_http_context = NULL;

    result =
        ap_http_backend.create(
            &influx_http_context
        );

    if (result != AP_OK)
        return result;

    if (influx_http_context == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_INIT,
            AP_PLUGIN_INFLUX,
            AP_ERROR_OUT_OF_MEMORY
        );
    }

    result =
        ap_http_backend.open(
            influx_http_context
        );

    if (result != AP_OK)
    {
        ap_http_backend.destroy(
            influx_http_context
        );

        influx_http_context = NULL;

        return result;
    }

    result =
        ap_dispatcher_register(
            ap_influx_event_handler
        );

    if (result != AP_OK)
    {
        ap_http_backend.close(
            influx_http_context
        );

        ap_http_backend.destroy(
            influx_http_context
        );

        influx_http_context = NULL;

        return result;
    }

    return AP_OK;
}


/* -------------------------------------------------- */
/* Plugin process                                     */
/* -------------------------------------------------- */

static ap_result_t ap_influx_plugin_process(void)
{
    return AP_OK;
}


/* -------------------------------------------------- */
/* Plugin shutdown                                    */
/* -------------------------------------------------- */

static void ap_influx_plugin_shutdown(void)
{
    if (influx_http_context != NULL)
    {
        ap_http_backend.close(
            influx_http_context
        );

        ap_http_backend.destroy(
            influx_http_context
        );

        influx_http_context = NULL;
    }

    ap_influx_config_clear(
        &influx_config
    );
}


/* -------------------------------------------------- */
/* Plugin definition                                  */
/* -------------------------------------------------- */

const ap_plugin_t ap_influx_plugin =
{
    .type = AP_PLUGIN_INFLUX,
    .name = "influx",

    .dependencies = NULL,
    .dependency_count = 0,

    .load = ap_influx_plugin_load,
    .init = ap_influx_plugin_init,
    .process = ap_influx_plugin_process,
    .shutdown = ap_influx_plugin_shutdown
};

AP_PLUGIN_REGISTER(ap_influx_plugin);