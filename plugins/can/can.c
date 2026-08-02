#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "can.h"
#include "ap_config_payload_reader.h"
#include "ap_module.h"
#include "ap_plugin.h"
#include "ap_result.h"


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
            &mapping->signal_type) != 0 ||
        ap_config_read_u8(
            reader,
            (uint8_t *)&mapping->direction) != 0 ||
        ap_config_read_u8(
            reader,
            (uint8_t *)&mapping->mapping_type) != 0 ||
        ap_config_read_u32_le(
            reader,
            &mapping->can_id) != 0 ||
        ap_config_read_u8(
            reader,
            &mapping->dlc) != 0)
    {
        return -1;
    }

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

        default:

            return -1;
    }

    return 0;
}


/* -------------------------------------------------- */
/* Plugin lifecycle                                  */
/* -------------------------------------------------- */

static ap_result_t ap_can_module_load(
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

    if (module_type != AP_MODULE_CAN)
    {
        return AP_ERROR_INVALID_ARGUMENT;
    }

    if (version != AP_CAN_CONFIG_VERSION)
    {
        return AP_ERROR_INVALID_ARGUMENT;
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
        return AP_ERROR_INVALID_ARGUMENT;
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
            return AP_ERROR_OUT_OF_MEMORY;
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
                return AP_ERROR_INVALID_ARGUMENT;
            }
        }
    }

    if (reader.offset != reader.length)
    {
        ap_can_config_clear(&can_config);
        return AP_ERROR_INVALID_ARGUMENT;
    }

    return AP_OK;
}


/* -------------------------------------------------- */
/* Plugin init / shutdown                            */
/* -------------------------------------------------- */

static void *can_backend_context;


static ap_result_t ap_can_module_init(void)
{
    ap_can_backend_config_t config;

    config.interface = can_config.interface;
    config.bitrate   = can_config.bitrate;

    can_backend_context = ap_can_backend.create();

    if (can_backend_context == NULL)
        return AP_ERROR_OUT_OF_MEMORY;

    if (ap_can_backend.open(
            can_backend_context,
            &config) != 0)
    {
        ap_can_backend.destroy(can_backend_context);
        can_backend_context = NULL;

        return AP_ERROR_INVALID_ARGUMENT;
    }

    return AP_OK;
}

static ap_result_t ap_can_module_process(void)
{
    ap_can_frame_t frame;

    int result =
        ap_can_backend.receive(
            can_backend_context,
            &frame
        );

    if (result < 0)
        return AP_ERROR_OPERATION_FAILED;

    if (result > 0)
        return AP_OK;

    /* Frame vorhanden:
     * CAN frame → Mapping → AP Event
     */

    return AP_OK;
}

static void ap_can_module_shutdown(void)
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
    .type = AP_MODULE_CAN,
    .name = "can",

    .dependencies = NULL,
    .dependency_count = 0,

    .load = ap_can_module_load,
    .init = ap_can_module_init,
    .process = ap_can_module_process,
    .shutdown = ap_can_module_shutdown
};


AP_PLUGIN_REGISTER(ap_can_plugin);