#include <stdint.h>

#include "can_mapping.h"
#include "ap_event.h"
#include "ap_registry.h"
#include "ap_dispatcher.h"

#include <stdio.h>

static ap_result_t can_decode_field(
    const ap_can_frame_t *frame,
    const ap_can_mapping_t *mapping,
    ap_event_t *event)
{
    const ap_can_field_t *field;
    uint64_t raw = 0;

    if (frame == NULL ||
        mapping == NULL ||
        event == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_PROCESS,
            AP_PLUGIN_CAN,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    field = &mapping->field;

    if (field->length == 0 ||
        field->length > 64)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_PROCESS,
            AP_PLUGIN_CAN,
            AP_ERROR_DECODING
        );
    }

    /*
     * Little-endian / default bit field.
     */
    if (field->encoding == AP_CAN_ENCODING_NONE ||
        field->encoding == AP_CAN_ENCODING_LE)
    {
        for (uint8_t i = 0; i < field->length; i++)
        {
            uint8_t bit = field->start_bit + i;
            uint8_t byte = bit / 8;
            uint8_t bit_offset = bit % 8;

            if (byte >= frame->dlc)
            {
                return AP_RESULT_MAKE(
                    AP_RESULT_SOURCE_PLUGIN,
                    AP_COMPONENT_PROCESS,
                    AP_PLUGIN_CAN,
                    AP_ERROR_DECODING
                );
            }

            if (frame->data[byte] & (1u << bit_offset))
            {
                raw |= (uint64_t)1 << i;
            }
        }
    }

    /*
     * Big-endian byte field.
     *
     * Currently only byte-aligned fields are supported.
     */
    else if (field->encoding == AP_CAN_ENCODING_BE)
    {
        if ((field->start_bit % 8) != 0 || (field->length % 8) != 0)
        {
            return AP_RESULT_MAKE(
                AP_RESULT_SOURCE_PLUGIN,
                AP_COMPONENT_PROCESS,
                AP_PLUGIN_CAN,
                AP_ERROR_ENCODING
            );
        }

        uint8_t start_byte = field->start_bit / 8;
        uint8_t byte_count = field->length / 8;

        if ((uint16_t)start_byte + byte_count > frame->dlc)
        {
            return AP_RESULT_MAKE(
                AP_RESULT_SOURCE_PLUGIN,
                AP_COMPONENT_PROCESS,
                AP_PLUGIN_CAN,
                AP_ERROR_DECODING
            );
        }

        for (uint8_t i = 0;
             i < byte_count;
             i++)
        {
            raw <<= 8;

            raw |=
                frame->data[start_byte + i];
        }
    }
    else
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_PROCESS,
            AP_PLUGIN_CAN,
            AP_ERROR_ENCODING
        );
    }

    switch (mapping->value_type)
    {
        case AP_VALUE_BOOL:

            event->value.b =
                raw != 0;

            break;

        case AP_VALUE_INT32:

            event->value.i =
                (int32_t)raw;

            break;

        case AP_VALUE_FLOAT:

            event->value.f =
                (float)raw *
                field->scale +
                field->offset;

            break;

        default:

            return AP_RESULT_MAKE(
                AP_RESULT_SOURCE_PLUGIN,
                AP_COMPONENT_PROCESS,
                AP_PLUGIN_CAN,
                AP_ERROR_INVALID_ARGUMENT
            );
    }

    return AP_OK;
}

ap_result_t ap_can_mapping_rx(
    const ap_can_frame_t *frame,
    const ap_can_config_t *config)
{
    if (frame == NULL ||
        config == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_PROCESS,
            AP_PLUGIN_CAN,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    for (uint8_t i = 0;
         i < config->mapping_count;
         i++)
    {
        const ap_can_mapping_t *mapping =
            &config->mappings[i];

        if (mapping->direction !=
            AP_CAN_DIRECTION_RX)
        {
            continue;
        }

        if (mapping->can_id !=
            frame->can_id)
        {
            continue;
        }

        const ap_object_t *object = NULL;

        ap_result_t result =
            ap_registry_get_or_create(
                mapping->object_id,
                mapping->value_type,
                &object
            );

        if (result != AP_OK)
        {
            fprintf(
                stderr,
                "CAN RX: failed to get object %u\n",
                mapping->object_id
            );

            return result;
        }

        ap_event_t event;

        ap_event_init(
            &event,
            object,
            0
        );

        if (mapping->mapping_type ==
            AP_CAN_MAPPING_FIELD)
        {
            result =
                can_decode_field(
                    frame,
                    mapping,
                    &event
                );

            if (result != AP_OK)
                return result;
        }
        else
        {
            continue;
        }

        ap_result_report(ap_dispatcher_publish(&event));
    }

    return AP_OK;
}