#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "mqtt.h"
#include "mqtt_adapter.h"
#include "ap_config_payload_reader.h"
#include "ap_module.h"
#include "ap_plugin.h"
#include "ap_result.h"


#define AP_MQTT_CONFIG_VERSION 1u

static ap_mqtt_config_t mqtt_config;
//ap_config_payload_reader_t reader;

/* -------------------------------------------------- */
/* Config cleanup                                     */
/* -------------------------------------------------- */

static void ap_mqtt_config_clear(ap_mqtt_config_t *config)
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

static int ap_mqtt_read_mapping(ap_config_payload_reader_t *reader,ap_mqtt_mapping_t *mapping)
{
    if (reader == NULL ||
        mapping == NULL)
    {
        return -1;
    }
    memset(mapping,0,sizeof(*mapping));

    if (ap_config_read_string(reader,&mapping->topic) != 0)
    {
        return -1;
    }

    if (ap_config_read_u32_le(reader,&mapping->object_id) != 0)
    {
        free(mapping->topic);
        mapping->topic = NULL;
        return -1;
    }

    if (ap_config_read_u8(reader,&mapping->value_type) != 0)
    {
        free(mapping->topic);
        mapping->topic = NULL;
        return -1;
    }

    if (ap_config_read_u8(reader,&mapping->flags) != 0)
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

    if (ap_config_read_u8(&reader, &module_type) != 0 || ap_config_read_u8(&reader, &version) != 0)
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