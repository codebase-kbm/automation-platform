//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "can.h"
#include "ap_config_payload_reader.h"
#include "ap_plugin.h"
#include "ap_result.h"

#include "can_mapping.h"


#define AP_CAN_CONFIG_VERSION 1u

static ap_can_config_t can_config;


/* -------------------------------------------------- */
/* Config cleanup                                     */
/* -------------------------------------------------- */

static void ap_can_config_clear(
    ap_can_config_t *config)
{
    if (config == NULL)
        return;

    free(config->interface);
    free(config->mappings);

    memset(
        config,
        0,
        sizeof(*config)
    );
}


/* -------------------------------------------------- */
/* Mapping parser                                     */
/* -------------------------------------------------- */

static int ap_can_read_mapping(

    ap_config_payload_reader_t *reader,
    ap_can_mapping_t *mapping)
{
    uint8_t value_type;
    uint8_t direction;
    uint8_t mapping_type;

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
            &mapping->object_id) != 0 ||
        ap_config_read_u8(
            reader,
            &value_type) != 0 ||
        ap_config_read_u8(
            reader,
            &direction) != 0 ||
        ap_config_read_u8(
            reader,
            &mapping_type) != 0 ||
        ap_config_read_u32_le(
            reader,
            &mapping->can_id) != 0 ||
        ap_config_read_u8(
            reader,
            &mapping->dlc) != 0)
    {
        return -1;
    }

    mapping->value_type =
        (ap_value_type_t)value_type;

    mapping->direction =
        (ap_can_direction_t)direction;

    mapping->mapping_type =
        (ap_can_mapping_type_t)mapping_type;

    switch (mapping->mapping_type)
    {
        case AP_CAN_MAPPING_FIELD:

            if (ap_config_read_u8(
                    reader,
                    &mapping->field.start_bit) != 0 ||
                ap_config_read_u8(
                    reader,
                    &mapping->field.length) != 0 ||
                ap_config_read_u8(
                    reader,
                    &mapping->field.encoding) != 0 ||
                ap_config_read_u8(
                    reader,
                    &mapping->field.flags) != 0 ||
                ap_config_read_float(
                    reader,
                    &mapping->field.scale) != 0 ||
                ap_config_read_float(
                    reader,
                    &mapping->field.offset) != 0)
            {
                return -1;
            }

            break;

        case AP_CAN_MAPPING_RAW:

            if (ap_config_read_u8(
                    reader,
                    &mapping->raw.length) != 0)
            {
                return -1;
            }

            if (mapping->raw.length > sizeof(mapping->raw.data))
                return -1;

            for (uint8_t i = 0;
                 i < mapping->raw.length;
                 i++)
            {
                if (ap_config_read_u8(
                        reader,
                        &mapping->raw.data[i]) != 0)
                {
                    return -1;
                }
            }

            break;

        default:

            return -1;
    }

    /*
     * Trigger follows the frame payload.
     */
    {
        uint8_t trigger_type;

        if (ap_config_read_u8(
                reader,
                &trigger_type) != 0)
        {
            return -1;
        }

        mapping->trigger.type =
            (ap_can_trigger_type_t)trigger_type;

        switch (mapping->trigger.type)
        {
            case AP_CAN_TRIGGER_NONE:

                break;

            case AP_CAN_TRIGGER_VALUE:
            {
                uint8_t value;

                if (ap_config_read_u8(
                        reader,
                        &value) != 0)
                {
                    return -1;
                }

                if (value > 1)
                    return -1;

                mapping->trigger.value =
                    value ? true : false;

                break;
            }

            case AP_CAN_TRIGGER_OBJECT:

                if (ap_config_read_u32_le(
                        reader,
                        &mapping->trigger.object_id) != 0)
                {
                    return -1;
                }

                break;

            default:

                return -1;
        }
    }

    return 0;
}

/* -------------------------------------------------- */
/* Plugin lifecycle                                  */
/* -------------------------------------------------- */

static ap_result_t ap_can_plugin_load(
    const ap_config_object_t *object)
{
    ap_config_payload_reader_t reader;

    uint8_t plugin_type;
    uint8_t version;
    uint8_t mapping_count;

    if (object == NULL ||
        object->payload == NULL)
    {
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_PLUGIN,AP_COMPONENT_CONFIG_READER,AP_PLUGIN_CAN,AP_ERROR_INVALID_ARGUMENT);
    }

    reader.data = object->payload;
    reader.length = object->header.payload_length;
    reader.offset = 0;

    if (ap_config_read_u8(
            &reader,
            &plugin_type) != 0 ||
        ap_config_read_u8(
            &reader,
            &version) != 0)
    {
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_PLUGIN,AP_COMPONENT_CONFIG_READER,AP_PLUGIN_CAN,AP_ERROR_INVALID_ARGUMENT);
    }

    if (plugin_type != AP_PLUGIN_CAN)
    {
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_PLUGIN,AP_COMPONENT_CONFIG_READER,AP_PLUGIN_CAN,AP_ERROR_INVALID_ARGUMENT);
    }

    if (version != AP_CAN_CONFIG_VERSION)
    {
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_PLUGIN,AP_COMPONENT_CONFIG_READER,AP_PLUGIN_CAN,AP_ERROR_INVALID_ARGUMENT);
    }

    ap_can_config_clear(&can_config);

    if (ap_config_read_string(
            &reader,
            &can_config.interface) != 0 ||
        ap_config_read_u32_le(
            &reader,
            &can_config.bitrate) != 0 ||
        ap_config_read_u8(
            &reader,
            &mapping_count) != 0)
    {
        ap_can_config_clear(&can_config);
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_PLUGIN,AP_COMPONENT_CONFIG_READER,AP_PLUGIN_CAN,AP_ERROR_INVALID_ARGUMENT);
    }

    can_config.mapping_count = mapping_count;

    if (mapping_count > 0)
    {
        can_config.mappings =
            calloc(
                mapping_count,
                sizeof(*can_config.mappings)
            );

        if (can_config.mappings == NULL)
        {
            ap_can_config_clear(&can_config);
            return AP_RESULT_MAKE(AP_RESULT_SOURCE_PLUGIN,AP_COMPONENT_MAPPER,AP_PLUGIN_CAN,AP_ERROR_OUT_OF_MEMORY);
        }

        for (uint8_t i = 0;
             i < mapping_count;
             i++)
        {
            if (ap_can_read_mapping(
                    &reader,
                    &can_config.mappings[i]) != 0)
            {
                ap_can_config_clear(&can_config);
                return AP_RESULT_MAKE(AP_RESULT_SOURCE_PLUGIN,AP_COMPONENT_MAPPER,AP_PLUGIN_CAN,AP_ERROR_INVALID_ARGUMENT);
            }
        }
    }

    if (reader.offset != reader.length)
    {
        ap_can_config_clear(&can_config);
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_PLUGIN,AP_COMPONENT_CONFIG_READER,AP_PLUGIN_CAN,AP_ERROR_INVALID_ARGUMENT);
    }

    return AP_OK;
}


/* -------------------------------------------------- */
/* Plugin init / shutdown                            */
/* -------------------------------------------------- */

static void *can_backend_context;


static ap_result_t ap_can_plugin_init(void)
{
    ap_can_backend_config_t config;

    config.interface = can_config.interface;
    config.bitrate   = can_config.bitrate;

    can_backend_context = ap_can_backend.create();

    if (can_backend_context == NULL)
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_PLUGIN,AP_COMPONENT_INIT,AP_PLUGIN_CAN,AP_ERROR_OUT_OF_MEMORY);

    if (ap_can_backend.open(
            can_backend_context,
            &config) != 0)
    {
        ap_can_backend.destroy(can_backend_context);
        can_backend_context = NULL;

        return AP_RESULT_MAKE(AP_RESULT_SOURCE_PLUGIN,AP_COMPONENT_INIT,AP_PLUGIN_CAN,AP_ERROR_INVALID_ARGUMENT);
    }

    return AP_OK;
}

static ap_result_t ap_can_plugin_process(void)
{
    ap_can_frame_t frame;

    int result =
        ap_can_backend.receive(
            can_backend_context,
            &frame
        );

    if (result < 0)
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_PLUGIN,AP_COMPONENT_PROCESS,AP_PLUGIN_CAN,AP_ERROR_OPERATION_FAILED);

    if (result > 0)
        return AP_OK;

    /* Frame vorhanden:
     * CAN frame → Mapping → AP Event
     */
    ap_can_mapping_rx(&frame,&can_config);

    return AP_OK;
}

static void ap_can_plugin_shutdown(void)
{
    if (can_backend_context != NULL)
    {
        ap_can_backend.close(can_backend_context);
        ap_can_backend.destroy(can_backend_context);
        can_backend_context = NULL;
    }

    ap_can_config_clear(&can_config);
}

/* -------------------------------------------------- */
/* Plugin definition                                  */
/* -------------------------------------------------- */

const ap_plugin_t ap_can_plugin =
{
    .type = AP_PLUGIN_CAN,
    .name = "can",

    .dependencies = NULL,
    .dependency_count = 0,

    .load = ap_can_plugin_load,
    .init = ap_can_plugin_init,
    .process = ap_can_plugin_process,
    .shutdown = ap_can_plugin_shutdown
};


AP_PLUGIN_REGISTER(ap_can_plugin);