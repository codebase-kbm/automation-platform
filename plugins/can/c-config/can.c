#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include <libxml/tree.h>

#include "payload_helpers.h"
#include "xml_helpers.h"
#include "ap_object.h"
#include "ap_plugin_compiler.h"
#include "can.h"

#define AP_CAN_CONFIG_VERSION 1u

/* -------------------------------------------------- */
/* Encoding parser                                     */
/* -------------------------------------------------- */

static int parse_encoding(
    const xmlChar *value,
    uint8_t *result)
{
    if (value == NULL || result == NULL)
        return -1;

    if (xmlStrcasecmp(value, BAD_CAST "le") == 0)
    {
        *result = AP_CAN_ENCODING_LE;
        return 0;
    }

    if (xmlStrcasecmp(value, BAD_CAST "be") == 0)
    {
        *result = AP_CAN_ENCODING_BE;
        return 0;
    }

    if (xmlStrcasecmp(value, BAD_CAST "none") == 0)
    {
        *result = AP_CAN_ENCODING_NONE;
        return 0;
    }

    return -1;
}

/* -------------------------------------------------- */
/* Raw data parser                                     */
/* -------------------------------------------------- */

static int parse_raw_data(
    const xmlChar *value,
    ap_can_raw_data_t *raw)
{
    char *text;
    char *token;
    char *end;
    unsigned long byte_value;

    if (value == NULL || raw == NULL)
        return -1;

    memset(
        raw,
        0,
        sizeof(*raw)
    );

    text = strdup((const char *)value);

    if (text == NULL)
        return -1;

    token = strtok(
        text,
        " \t\r\n"
    );

    while (token != NULL)
    {
        if (raw->length >= sizeof(raw->data))
        {
            free(text);
            return -1;
        }

        byte_value = strtoul(
            token,
            &end,
            16
        );

        if (*end != '\0' ||
            byte_value > 0xff)
        {
            free(text);
            return -1;
        }

        raw->data[raw->length] =
            (uint8_t)byte_value;

        raw->length++;

        token = strtok(
            NULL,
            " \t\r\n"
        );
    }

    free(text);

    return 0;
}

/* -------------------------------------------------- */
/* Trigger compiler                                    */
/* -------------------------------------------------- */

static int compile_trigger(
    xmlNodePtr mapping,
    ap_can_direction_t direction,
    ap_plugin_config_buffer_t *buffer)
{
    xmlNodePtr trigger;
    xmlChar *value = NULL;
    xmlChar *object_id = NULL;

    if (mapping == NULL ||
        buffer == NULL)
    {
        return -1;
    }

    /*
     * RX mappings do not use triggers.
     */
    if (direction == AP_CAN_DIRECTION_RX)
    {
        if (find_child(
                mapping,
                "trigger") != NULL)
        {
            fprintf(
                stderr,
                "CAN: line %ld: trigger is only valid for TX mappings\n",
                xmlGetLineNo(mapping)
            );

            return -1;
        }

        if (payload_write_u8(
                buffer,
                AP_CAN_TRIGGER_NONE) != 0)
        {
            return -1;
        }

        return 0;
    }

    /*
     * TX mapping.
     */
    trigger = find_child(
        mapping,
        "trigger"
    );

    /*
     * No trigger.
     */
    if (trigger == NULL)
    {
        if (payload_write_u8(
                buffer,
                AP_CAN_TRIGGER_NONE) != 0)
        {
            return -1;
        }

        return 0;
    }

    value = get_attribute(
        trigger,
        "value"
    );

    object_id = get_attribute(
        trigger,
        "object_id"
    );

    /*
     * Value trigger.
     */
    if (value != NULL)
    {
        uint8_t trigger_value;

        if (object_id != NULL)
        {
            fprintf(
                stderr,
                "CAN: line %ld: trigger must not contain both "
                "'trigger' and 'object_id'\n",
                xmlGetLineNo(trigger)
            );

            goto error;
        }   

        if (parse_bool(
                value,
                &trigger_value) != 0)
        {
            fprintf(
                stderr,
                "CAN: line %ld: invalid trigger value '%s'\n",
                xmlGetLineNo(trigger),
                value
            );

            goto error;
        }

        if (payload_write_u8(
                buffer,
                AP_CAN_TRIGGER_VALUE) != 0 ||
            payload_write_u8(
                buffer,
                trigger_value ? 1u : 0u) != 0)
        {
            goto error;
        }

        goto success;
    }

    /*
     * Object trigger.
     */
    if (object_id != NULL)
    {
        uint32_t object_id_value;

        if (parse_u32(
                object_id,
                &object_id_value) != 0)
        {
            fprintf(
                stderr,
                "CAN: line %ld: invalid trigger object_id '%s'\n",
                xmlGetLineNo(trigger),
                object_id
            );

            goto error;
        }

        if (payload_write_u8(
                buffer,
                AP_CAN_TRIGGER_OBJECT) != 0 ||
            payload_write_u32_le(
                buffer,
                object_id_value) != 0)
        {
            goto error;
        }

        goto success;
    }

    fprintf(
        stderr,
        "CAN: line %ld: trigger requires "
        "'trigger' or 'object_id'\n",
        xmlGetLineNo(trigger)
    );

    goto error;

success:

    xmlFree(value);
    xmlFree(object_id);

    return 0;

error:

    xmlFree(value);
    xmlFree(object_id);

    return -1;
}

/* -------------------------------------------------- */
/* Field compiler                                      */
/* -------------------------------------------------- */

static int compile_field(
    xmlNodePtr field,
    ap_plugin_config_buffer_t *buffer)
{
    xmlChar *start_bit = NULL;
    xmlChar *length = NULL;
    xmlChar *encoding = NULL;
    xmlChar *scale = NULL;
    xmlChar *offset = NULL;

    uint8_t start_bit_value = 0;
    uint8_t length_value = 0;
    uint8_t encoding_value = AP_CAN_ENCODING_NONE;
    uint8_t flags = 0;

    float scale_value = 1.0f;
    float offset_value = 0.0f;

    if (field == NULL ||
        buffer == NULL)
    {
        return -1;
    }

    /*
     * A raw field is handled by compile_frame().
     */
    start_bit = get_attribute(
        field,
        "start_bit"
    );

    length = get_attribute(
        field,
        "length"
    );

    if (start_bit == NULL)
    {
        fprintf(
            stderr,
            "CAN: line %ld: missing 'start_bit'\n",
            xmlGetLineNo(field)
        );

        goto error;
    }

    if (length == NULL)
    {
        fprintf(
            stderr,
            "CAN: line %ld: missing 'length'\n",
            xmlGetLineNo(field)
        );

        goto error;
    }

    if (parse_u8(
            start_bit,
            &start_bit_value) != 0 ||
        start_bit_value > 63)
    {
        fprintf(
            stderr,
            "CAN: line %ld: invalid start_bit '%s' "
            "(expected 0..63)\n",
            xmlGetLineNo(field),
            start_bit
        );

        goto error;
    }

    if (parse_u8(
            length,
            &length_value) != 0 ||
        length_value == 0 ||
        length_value > 64)
    {
        fprintf(
            stderr,
            "CAN: line %ld: invalid length '%s' "
            "(expected 1..64)\n",
            xmlGetLineNo(field),
            length
        );

        goto error;
    }

    if ((uint16_t)start_bit_value +
        (uint16_t)length_value > 64)
    {
        fprintf(
            stderr,
            "CAN: line %ld: field exceeds CAN payload "
            "(start_bit=%u, length=%u)\n",
            xmlGetLineNo(field),
            start_bit_value,
            length_value
        );

        goto error;
    }

    encoding = get_attribute(
        field,
        "encoding"
    );

    if (encoding != NULL)
    {
        if (parse_encoding(
                encoding,
                &encoding_value) != 0)
        {
            fprintf(
                stderr,
                "CAN: line %ld: invalid encoding '%s'\n",
                xmlGetLineNo(field),
                encoding
            );

            goto error;
        }
    }

    scale = get_attribute(
        field,
        "scale"
    );

    if (scale != NULL)
    {
        char *endptr = NULL;

        scale_value = strtof(
            (const char *)scale,
            &endptr
        );

        if (endptr == (char *)scale ||
            *endptr != '\0' ||
            !isfinite(scale_value))
        {
            fprintf(
                stderr,
                "CAN: line %ld: invalid scale '%s'\n",
                xmlGetLineNo(field),
                scale
            );

            goto error;
        }
    }

    offset = get_attribute(
        field,
        "offset"
    );

    if (offset != NULL)
    {
        char *endptr = NULL;

        offset_value = strtof(
            (const char *)offset,
            &endptr
        );

        if (endptr == (char *)offset ||
            *endptr != '\0' ||
            !isfinite(offset_value))
        {
            fprintf(
                stderr,
                "CAN: line %ld: invalid offset '%s'\n",
                xmlGetLineNo(field),
                offset
            );

            goto error;
        }
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
        fprintf(
            stderr,
            "CAN: line %ld: failed to write field configuration\n",
            xmlGetLineNo(field)
        );

        goto error;
    }

    goto success;

success:

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

/* -------------------------------------------------- */
/* Frame compiler                                      */
/* -------------------------------------------------- */

static int compile_frame(
    xmlNodePtr frame,
    ap_plugin_config_buffer_t *buffer,
    ap_can_mapping_type_t *mapping_type)
{
    xmlChar *id = NULL;
    xmlChar *dlc = NULL;
    xmlChar *data = NULL;

    uint32_t can_id;
    uint8_t dlc_value = 8;

    xmlNodePtr field;

    if (frame == NULL ||
        buffer == NULL ||
        mapping_type == NULL)
    {
        return -1;
    }

    id = get_attribute(
        frame,
        "id"
    );

    dlc = get_attribute(
        frame,
        "dlc"
    );

    if (id == NULL)
    {
        fprintf(
            stderr,
            "CAN: line %ld: missing attribute 'id'\n",
            xmlGetLineNo(frame)
        );

        goto error;
    }

    if (parse_u32_hex(
            id,
            &can_id) != 0)
    {
        fprintf(
            stderr,
            "CAN: line %ld: invalid CAN id '%s'\n",
            xmlGetLineNo(frame),
            id
        );

        goto error;
    }

    if (dlc != NULL)
    {
        if (parse_u8(
                dlc,
                &dlc_value) != 0 ||
            dlc_value > 8)
        {
            fprintf(
                stderr,
                "CAN: line %ld: invalid DLC '%s' "
                "(expected 0..8)\n",
                xmlGetLineNo(frame),
                dlc
            );

            goto error;
        }
    }

    /*
     * CAN frame header.
     */
    if (payload_write_u32_le(
            buffer,
            can_id) != 0 ||
        payload_write_u8(
            buffer,
            dlc_value) != 0)
    {
        fprintf(
            stderr,
            "CAN: line %ld: failed to write frame header\n",
            xmlGetLineNo(frame)
        );

        goto error;
    }

    /*
     * Find field.
     */
    field = find_child(
        frame,
        "field"
    );

    /*
     * No field means a raw/buffer mapping.
     */
    if (field == NULL)
    {
        *mapping_type =
            AP_CAN_MAPPING_RAW;

        goto success;
    }

    /*
     * Raw field.
     */
    data = get_attribute(
        field,
        "data"
    );

    if (data != NULL)
    {
        ap_can_raw_data_t raw;

        if (parse_raw_data(
                data,
                &raw) != 0)
        {
            fprintf(
                stderr,
                "CAN: line %ld: invalid raw CAN data '%s'\n",
                xmlGetLineNo(field),
                data
            );

            goto error;
        }

        if (raw.length > dlc_value)
        {
            fprintf(
                stderr,
                "CAN: line %ld: raw data length %u "
                "exceeds DLC %u\n",
                xmlGetLineNo(field),
                raw.length,
                dlc_value
            );

            goto error;
        }

        if (payload_write_u8(
                buffer,
                raw.length) != 0)
        {
            fprintf(
                stderr,
                "CAN: line %ld: failed to write raw data length\n",
                xmlGetLineNo(field)
            );

            goto error;
        }

        for (uint8_t i = 0;
            i < raw.length;
            i++)
        {
            if (payload_write_u8(
                    buffer,
                    raw.data[i]) != 0)
            {
                fprintf(
                    stderr,
                    "CAN: line %ld: failed to write raw CAN data\n",
                    xmlGetLineNo(field)
                );

                goto error;
            }
        }

        *mapping_type =
            AP_CAN_MAPPING_RAW;

        goto success;
    }

    /*
     * Normal bit-field mapping.
     */
    if (compile_field(
            field,
            buffer) != 0)
    {
        fprintf(
            stderr,
            "CAN: line %ld: invalid CAN field\n",
            xmlGetLineNo(field)
        );

        goto error;
    }

    *mapping_type =
        AP_CAN_MAPPING_FIELD;

success:

    xmlFree(data);
    xmlFree(id);
    xmlFree(dlc);

    return 0;

error:

    xmlFree(data);
    xmlFree(id);
    xmlFree(dlc);

    return -1;
}

/* -------------------------------------------------- */
/* Mapping compiler                                    */
/* -------------------------------------------------- */

static int compile_mapping(
    xmlNodePtr mapping,
    ap_plugin_config_buffer_t *buffer)
{
    xmlChar *object_id = NULL;
    xmlChar *direction = NULL;
    xmlChar *type = NULL;

    uint32_t object_id_value;

    uint8_t direction_value;
    uint8_t value_type;

    ap_can_mapping_type_t mapping_type;

    xmlNodePtr frame;

    if (mapping == NULL ||
        buffer == NULL)
    {
        return -1;
    }

    object_id = get_attribute(
        mapping,
        "object_id"
    );

    type = get_attribute(
        mapping,
        "type"
    );

    direction = get_attribute(
        mapping,
        "direction"
    );

    if (object_id == NULL)
    {
        fprintf(
            stderr,
            "CAN: line %ld: missing attribute 'object_id'\n",
            xmlGetLineNo(mapping)
        );

        goto error;
    }

    if (direction == NULL)
    {
        fprintf(
            stderr,
            "CAN: line %ld: missing attribute 'direction'\n",
            xmlGetLineNo(mapping)
        );

        goto error;
    }

    if (type == NULL)
    {
        fprintf(
            stderr,
            "CAN: line %ld: missing attribute 'type'\n",
            xmlGetLineNo(mapping)
        );

        goto error;
    }

    if (parse_u32(
            object_id,
            &object_id_value) != 0)
    {
        fprintf(
            stderr,
            "CAN: line %ld: invalid object_id '%s'\n",
            xmlGetLineNo(mapping),
            object_id
        );

        goto error;
    }

    if (xmlStrcasecmp(
            direction,
            BAD_CAST "rx") == 0)
    {
        direction_value =
            AP_CAN_DIRECTION_RX;
    }
    else if (xmlStrcasecmp(
                 direction,
                 BAD_CAST "tx") == 0)
    {
        direction_value =
            AP_CAN_DIRECTION_TX;
    }
    else
    {
        fprintf(
            stderr,
            "CAN: line %ld: invalid direction '%s'\n",
            xmlGetLineNo(mapping),
            direction
        );

        goto error;
    }

    if (parse_value_type(
            type,
            &value_type) != 0)
    {
        fprintf(
            stderr,
            "CAN: line %ld: invalid value type '%s'\n",
            xmlGetLineNo(mapping),
            type
        );

        goto error;
    }

    frame = find_child(
        mapping,
        "frame"
    );

    if (frame == NULL)
    {
        fprintf(
            stderr,
            "CAN: line %ld: missing <frame>\n",
            xmlGetLineNo(mapping)
        );

        goto error;
    }

    /*
     * Compile the frame into a temporary buffer
     * because mapping_type is determined by the
     * frame contents.
     */
    {
        uint8_t frame_payload[256];

        ap_plugin_config_buffer_t frame_buffer =
        {
            .data = frame_payload,
            .length = 0,
            .capacity = sizeof(frame_payload)
        };

        if (compile_frame(
                frame,
                &frame_buffer,
                &mapping_type) != 0)
        {
            goto error;
        }

        if (payload_write_u32_le(
                buffer,
                object_id_value) != 0 ||
            payload_write_u8(
                buffer,
                value_type) != 0 ||
            payload_write_u8(
                buffer,
                direction_value) != 0 ||
            payload_write_u8(
                buffer,
                (uint8_t)mapping_type) != 0)
        {
            goto error;
        }

        for (size_t i = 0;
            i < frame_buffer.length;
            i++)
        {
            if (payload_write_u8(
                    buffer,
                    frame_buffer.data[i]) != 0)
            {
                fprintf(
                    stderr,
                    "CAN: line %ld: failed to write frame payload\n",
                    xmlGetLineNo(mapping)
                );

                goto error;
            }
        }
    }

    /*
     * Trigger follows the frame payload.
     */
    if (compile_trigger(
            mapping,
            (ap_can_direction_t)direction_value,
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

/* -------------------------------------------------- */
/* CAN configuration compiler                          */
/* -------------------------------------------------- */

static int ap_can_config_compile(
    xmlNodePtr plugin,
    ap_plugin_config_buffer_t *buffer)
{
    xmlNodePtr connection;
    xmlNodePtr interface;
    xmlNodePtr mappings;

    xmlChar *name = NULL;
    xmlChar *bitrate = NULL;

    uint32_t bitrate_value;
    uint8_t mapping_count = 0;
    size_t mapping_count_offset;

    if (plugin == NULL ||
        buffer == NULL ||
        buffer->data == NULL)
    {
        return -1;
    }

    connection = find_child(
        plugin,
        "connection"
    );

    mappings = find_child(
        plugin,
        "mappings"
    );

    if (connection == NULL)
    {
        fprintf(
            stderr,
            "CAN: line %ld: missing <connection>\n",
            xmlGetLineNo(plugin)
        );

        return -1;
    }

    if (mappings == NULL)
    {
        fprintf(
            stderr,
            "CAN: line %ld: missing <mappings>\n",
            xmlGetLineNo(plugin)
        );

        return -1;
    }

    interface = find_child(
        connection,
        "interface"
    );

    if (interface == NULL)
    {
        fprintf(
            stderr,
            "CAN: line %ld: missing <interface>\n",
            xmlGetLineNo(connection)
        );

        return -1;
    }

    name = get_attribute(
        interface,
        "name"
    );

    bitrate = get_attribute(
        interface,
        "bitrate"
    );

    if (name == NULL)
    {
        fprintf(
            stderr,
            "CAN: line %ld: missing interface attribute 'name'\n",
            xmlGetLineNo(interface)
        );

        goto error;
    }

    if (bitrate == NULL)
    {
        fprintf(
            stderr,
            "CAN: line %ld: missing interface attribute 'bitrate'\n",
            xmlGetLineNo(interface)
        );

        goto error;
    }

    if (parse_u32(
            bitrate,
            &bitrate_value) != 0)
    {
        fprintf(
            stderr,
            "CAN: line %ld: invalid bitrate '%s'\n",
            xmlGetLineNo(interface),
            bitrate
        );

        goto error;
    }

    buffer->length = 0;

    if (payload_write_u8(
            buffer,
            (uint8_t)AP_PLUGIN_CAN) != 0 ||
        payload_write_u8(
            buffer,
            AP_CAN_CONFIG_VERSION) != 0 ||
        payload_write_string(
            buffer,
            (const char *)name) != 0 ||
        payload_write_u32_le(
            buffer,
            bitrate_value) != 0)
    {
        fprintf(
            stderr,
            "CAN: line %ld: failed to write interface configuration\n",
            xmlGetLineNo(interface)
        );

        goto error;
    }

    mapping_count_offset =
        buffer->length;

    if (payload_write_u8(
            buffer,
            0) != 0)
    {
        goto error;
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

        if (mapping_count == UINT8_MAX)
        {
            fprintf(
                stderr,
                "CAN: line %ld: too many mappings\n",
                xmlGetLineNo(node)
            );

            goto error;
        }

        if (compile_mapping(
                node,
                buffer) != 0)
        {
            goto error;
        }

        mapping_count++;
    }

    buffer->data[mapping_count_offset] =
        mapping_count;

    xmlFree(name);
    xmlFree(bitrate);

    return 0;

error:

    xmlFree(name);
    xmlFree(bitrate);

    buffer->length = 0;

    return -1;
}

/* -------------------------------------------------- */
/* Plugin compiler registration                        */
/* -------------------------------------------------- */

static const ap_plugin_compiler_t ap_can_config_plugin =
{
    .type = AP_PLUGIN_CAN,
    .name = "can",
    .compile = ap_can_config_compile
};

AP_PLUGIN_COMPILER_REGISTER(
    ap_can_config_plugin
);