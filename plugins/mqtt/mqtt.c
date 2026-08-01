#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "mqtt.h"
#include "mqtt_adapter.h"

#include "ap_module.h"
#include "ap_plugin.h"
#include "ap_result.h"


#define AP_MQTT_CONFIG_VERSION 1u

typedef struct
{
    const uint8_t *data;
    size_t length;
    size_t offset;

} ap_mqtt_config_reader_t;


static ap_mqtt_config_t mqtt_config;


/* -------------------------------------------------- */
/* Config reader                                      */
/* -------------------------------------------------- */

static int read_u8(
    ap_mqtt_config_reader_t *reader,
    uint8_t *value)
{
    if (reader == NULL ||
        value == NULL ||
        reader->offset + 1 > reader->length)
    {
        return -1;
    }

    *value = reader->data[reader->offset++];

    return 0;
}


static int read_u16_le(
    ap_mqtt_config_reader_t *reader,
    uint16_t *value)
{
    uint16_t result;

    if (reader == NULL ||
        value == NULL ||
        reader->offset + 2 > reader->length)
    {
        return -1;
    }

    result =
        (uint16_t)reader->data[reader->offset] |
        ((uint16_t)reader->data[reader->offset + 1] << 8);

    reader->offset += 2;

    *value = result;

    return 0;
}


static int read_u32_le(
    ap_mqtt_config_reader_t *reader,
    uint32_t *value)
{
    uint32_t result;

    if (reader == NULL ||
        value == NULL ||
        reader->offset + 4 > reader->length)
    {
        return -1;
    }

    result =
        (uint32_t)reader->data[reader->offset] |
        ((uint32_t)reader->data[reader->offset + 1] << 8) |
        ((uint32_t)reader->data[reader->offset + 2] << 16) |
        ((uint32_t)reader->data[reader->offset + 3] << 24);

    reader->offset += 4;

    *value = result;

    return 0;
}


static int read_string(
    ap_mqtt_config_reader_t *reader,
    char **value)
{
    uint8_t length;
    char *string;

    if (reader == NULL ||
        value == NULL)
    {
        return -1;
    }

    if (read_u8(reader, &length) != 0)
        return -1;

    if (reader->offset + length > reader->length)
        return -1;

    string = malloc((size_t)length + 1);

    if (string == NULL)
        return -1;

    memcpy(
        string,
        &reader->data[reader->offset],
        length
    );

    string[length] = '\0';

    reader->offset += length;

    *value = string;

    return 0;
}


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
/* Mapping parser                                    */
/* -------------------------------------------------- */

static int ap_mqtt_read_mapping(
    ap_mqtt_config_reader_t *reader,
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

    if (read_string(
            reader,
            &mapping->topic) != 0)
    {
        return -1;
    }

    if (read_u32_le(
            reader,
            &mapping->object_id) != 0)
    {
        free(mapping->topic);
        mapping->topic = NULL;
        return -1;
    }

    if (read_u8(
            reader,
            &mapping->value_type) != 0)
    {
        free(mapping->topic);
        mapping->topic = NULL;
        return -1;
    }

    if (read_u8(
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
/* Plugin lifecycle                                  */
/* -------------------------------------------------- */

static ap_result_t ap_mqtt_module_load(
    const ap_config_object_t *object)
{
    ap_mqtt_config_reader_t reader;

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

    if (read_u8(&reader, &module_type) != 0 ||
        read_u8(&reader, &version) != 0)
    {
        return AP_ERROR_INVALID_ARGUMENT;
    }

    if (module_type != AP_MODULE_MQTT)
    {
        return AP_ERROR_INVALID_ARGUMENT;
    }

    if (version != AP_MQTT_CONFIG_VERSION)
    {
        return AP_ERROR_INVALID_ARGUMENT;
    }

    ap_mqtt_config_clear(&mqtt_config);

    if (read_string(
            &reader,
            &mqtt_config.host) != 0 ||
        read_u16_le(
            &reader,
            &mqtt_config.port) != 0 ||
        read_string(
            &reader,
            &mqtt_config.user) != 0 ||
        read_string(
            &reader,
            &mqtt_config.password) != 0 ||
        read_string(
            &reader,
            &mqtt_config.client_id) != 0 ||
        read_u8(
            &reader,
            &mapping_count) != 0)
    {
        ap_mqtt_config_clear(&mqtt_config);
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
            ap_mqtt_config_clear(&mqtt_config);
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
                ap_mqtt_config_clear(&mqtt_config);
                return AP_ERROR_INVALID_ARGUMENT;
            }
        }
    }

    if (reader.offset != reader.length)
    {
        ap_mqtt_config_clear(&mqtt_config);
        return AP_ERROR_INVALID_ARGUMENT;
    }
	
    return AP_OK;
}


static ap_result_t ap_mqtt_module_init(void)
{
	return ap_mqtt_init(&mqtt_config);
}


static void ap_mqtt_module_shutdown(void)
{
    ap_mqtt_config_clear(&mqtt_config);
}


const ap_plugin_t ap_mqtt_plugin =
{
    .type = AP_MODULE_MQTT,
    .name = "mqtt",
	
	.dependencies = NULL,
    .dependency_count = 0,

    .load = ap_mqtt_module_load,
    .init = ap_mqtt_module_init,
    .shutdown = ap_mqtt_module_shutdown
};


AP_PLUGIN_REGISTER(ap_mqtt_plugin);