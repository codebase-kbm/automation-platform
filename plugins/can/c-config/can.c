#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <libxml/tree.h>
#include "payload_helpers.h"
#include "xml_helpers.h"
#include "ap_object.h"
#include "ap_plugin_compiler.h"
#include "can.h"

static int ap_can_config_compile(
    xmlNodePtr module,
    ap_plugin_config_buffer_t *buffer)
{
    if (module == NULL ||
        buffer == NULL ||
        buffer->data == NULL)
    {
        return -1;
    }

    printf("CAN config compiler called\n");

    return 0;
}

int parse_encoding(const xmlChar *value,uint8_t *result)
{
    if (value == NULL ||
        result == NULL)
    {
        return -1;
    }


    if (xmlStrcmp(
            value,
            BAD_CAST "le") == 0)
    {
        *result = AP_CAN_ENCODING_LE;
        return 0;
    }


    if (xmlStrcmp(
            value,
            BAD_CAST "be") == 0)
    {
        *result = AP_CAN_ENCODING_BE;
        return 0;
    }


    if (xmlStrcmp(
            value,
            BAD_CAST "none") == 0)
    {
        *result = AP_CAN_ENCODING_NONE;
        return 0;
    }


    return -1;
}

static int compile_field(
    xmlNodePtr field,
    ap_plugin_config_buffer_t *buffer)
{
    xmlChar *start_bit = NULL;
    xmlChar *length = NULL;
    xmlChar *encoding = NULL;
    xmlChar *scale = NULL;
    xmlChar *offset = NULL;

    uint8_t start_bit_value;
    uint8_t length_value;
    uint8_t encoding_value = AP_CAN_ENCODING_NONE;
    uint8_t flags = 0;

    float scale_value = 1.0f;
    float offset_value = 0.0f;


    if (field == NULL ||
        buffer == NULL)
    {
        return -1;
    }


    start_bit =
        get_attribute(field, "start_bit");

    length =
        get_attribute(field, "length");


    if (start_bit == NULL ||
        length == NULL)
    {
        goto error;
    }


    if (parse_u8(
            start_bit,
            &start_bit_value) != 0)
    {
        goto error;
    }


    if (parse_u8(
            length,
            &length_value) != 0)
    {
        goto error;
    }


    encoding =
        get_attribute(field, "encoding");

    if (encoding != NULL)
    {
        if (parse_encoding(
                encoding,
                &encoding_value) != 0)
        {
            goto error;
        }
    }


    scale =
        get_attribute(field, "scale");

    if (scale != NULL)
    {
        scale_value =
            strtof(
                (char *)scale,
                NULL
            );
    }


    offset =
        get_attribute(field, "offset");

    if (offset != NULL)
    {
        offset_value =
            strtof(
                (char *)offset,
                NULL
            );
    }


    if (payload_write_u8(
            buffer,
            start_bit_value) != 0 ||

        payload_write_u8(
            buffer,
            length_value) != 0 ||

        payload_write_u8(
            buffer,
            encoding_value) != 0 ||

        payload_write_u8(
            buffer,
            flags) != 0 ||

        payload_write_float(
            buffer,
            scale_value) != 0 ||

        payload_write_float(
            buffer,
            offset_value) != 0)
    {
        goto error;
    }


    xmlFree(start_bit);
    xmlFree(length);
    xmlFree(encoding);
    xmlFree(scale);
    xmlFree(offset);

    return 0;


error:

    xmlFree(start_bit);
    xmlFree(length);
    xmlFree(encoding);
    xmlFree(scale);
    xmlFree(offset);

    return -1;
}

static int compile_mapping(
    xmlNodePtr mapping,
    ap_plugin_config_buffer_t *buffer)
{
    xmlChar *object_id = NULL;
    xmlChar *direction = NULL;
    xmlChar *type = NULL;

    uint32_t object_id_value;

    uint8_t direction_value;
    uint8_t signal_type;

    xmlNodePtr field;


    if (mapping == NULL ||
        buffer == NULL)
    {
        return -1;
    }


    object_id =
        get_attribute(
            mapping,
            "object_id"
        );

    direction =
        get_attribute(
            mapping,
            "direction"
        );

    type =
        get_attribute(
            mapping,
            "type"
        );


    if (object_id == NULL ||
        direction == NULL ||
        type == NULL)
    {
        goto error;
    }


    if (parse_u32(
            object_id,
            &object_id_value) != 0)
    {
        goto error;
    }


    if (strcmp(
            (char *)direction,
            "rx") == 0)
    {
        direction_value =
            AP_CAN_DIRECTION_RX;
    }
    else if (strcmp(
                (char *)direction,
                "tx") == 0)
    {
        direction_value =
            AP_CAN_DIRECTION_TX;
    }
    else
    {
        goto error;
    }


    if (parse_value_type(
            type,
            &signal_type) != 0)
    {
        goto error;
    }


    field =
        find_child(
            mapping,
            "field"
        );


    /*
     * Version 1:
     * only field mappings
     */
    if (field == NULL)
    {
        goto error;
    }


    /*
     * Mapping header
     */

    if (payload_write_u32_le(
            buffer,
            object_id_value) != 0 ||

        payload_write_u8(
            buffer,
            signal_type) != 0 ||

        payload_write_u8(
            buffer,
            direction_value) != 0 ||

        payload_write_u8(
            buffer,
            AP_CAN_MAPPING_FIELD) != 0)
    {
        goto error;
    }


    if (compile_field(
            field,
            buffer) != 0)
    {
        goto error;
    }


    xmlFree(object_id);
    xmlFree(direction);
    xmlFree(type);

    return 0;


error:

    xmlFree(object_id);
    xmlFree(direction);
    xmlFree(type);

    return -1;
}

static int compile_frame(
    xmlNodePtr frame,
    ap_plugin_config_buffer_t *buffer)
{
    xmlChar *id = NULL;
    xmlChar *dlc = NULL;

    uint32_t can_id;
    uint8_t dlc_value;

    xmlNodePtr mappings;

    uint16_t mapping_count = 0;
    size_t mapping_count_offset;


    if (frame == NULL ||
        buffer == NULL)
    {
        return -1;
    }


    id = get_attribute(frame, "id");
    dlc = get_attribute(frame, "dlc");


    if (id == NULL)
    {
        goto error;
    }


    /*
     * Default CAN DLC
     */
    dlc_value = 8;


    if (dlc != NULL)
    {
        if (parse_u8(
                dlc,
                &dlc_value) != 0)
        {
            goto error;
        }
    }


    if (parse_u32_hex(
            id,
            &can_id) != 0)
    {
        goto error;
    }


    /*
     * Frame Header
     */
    if (payload_write_u32_le(
            buffer,
            can_id) != 0 ||

        payload_write_u8(
            buffer,
            dlc_value) != 0)
    {
        goto error;
    }


    /*
     * Mapping count placeholder
     */
    mapping_count_offset =
        buffer->length;


    if (payload_write_u16_le(
            buffer,
            0) != 0)
    {
        goto error;
    }


    mappings = find_child(
        frame,
        "mappings"
    );


    if (mappings == NULL)
    {
        /*
         * Empty frame is valid
         */
        buffer->data[mapping_count_offset] = 0;
        buffer->data[mapping_count_offset + 1] = 0;

        goto success;
    }


    for (xmlNodePtr node = mappings->children;
         node != NULL;
         node = node->next)
    {
        if (node->type != XML_ELEMENT_NODE)
            continue;


        if (xmlStrcmp(
                node->name,
                BAD_CAST "mapping") != 0)
        {
            continue;
        }


        if (mapping_count == UINT16_MAX)
            goto error;


        if (compile_mapping(
                node,
                buffer) != 0)
        {
            goto error;
        }


        mapping_count++;
    }


    buffer->data[mapping_count_offset] =
        (uint8_t)(mapping_count & 0xFF);

    buffer->data[mapping_count_offset + 1] =
        (uint8_t)(mapping_count >> 8);



success:

    xmlFree(id);
    xmlFree(dlc);

    return 0;


error:

    xmlFree(id);
    xmlFree(dlc);

    return -1;
}

static const ap_plugin_compiler_t ap_can_config_plugin =
{
    .type = AP_MODULE_CAN,
    .name = "can",
    .compile = ap_can_config_compile
};


AP_PLUGIN_COMPILER_REGISTER(
    ap_can_config_plugin
);