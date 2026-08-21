#include "peer_protocol.h"

#include <string.h>
#include "ap_result.h"

/*
 * --------------------------------------------------------------------------
 * Wire helpers
 * --------------------------------------------------------------------------
 */

static void put_u16(uint8_t *buffer, uint16_t value)
{
    buffer[0] = (uint8_t)((value >> 8) & 0xffu);
    buffer[1] = (uint8_t)(value & 0xffu);
}

static void put_u32(uint8_t *buffer, uint32_t value)
{
    buffer[0] = (uint8_t)((value >> 24) & 0xffu);
    buffer[1] = (uint8_t)((value >> 16) & 0xffu);
    buffer[2] = (uint8_t)((value >> 8) & 0xffu);
    buffer[3] = (uint8_t)(value & 0xffu);
}

static void put_u64(uint8_t *buffer, uint64_t value)
{
    buffer[0] = (uint8_t)((value >> 56) & 0xffu);
    buffer[1] = (uint8_t)((value >> 48) & 0xffu);
    buffer[2] = (uint8_t)((value >> 40) & 0xffu);
    buffer[3] = (uint8_t)((value >> 32) & 0xffu);
    buffer[4] = (uint8_t)((value >> 24) & 0xffu);
    buffer[5] = (uint8_t)((value >> 16) & 0xffu);
    buffer[6] = (uint8_t)((value >> 8) & 0xffu);
    buffer[7] = (uint8_t)(value & 0xffu);
}

static uint16_t get_u16(const uint8_t *buffer)
{
    return ((uint16_t)buffer[0] << 8)
         | (uint16_t)buffer[1];
}

static uint32_t get_u32(const uint8_t *buffer)
{
    return ((uint32_t)buffer[0] << 24)
         | ((uint32_t)buffer[1] << 16)
         | ((uint32_t)buffer[2] << 8)
         | (uint32_t)buffer[3];
}

static uint64_t get_u64(const uint8_t *buffer)
{
    return ((uint64_t)buffer[0] << 56)
         | ((uint64_t)buffer[1] << 48)
         | ((uint64_t)buffer[2] << 40)
         | ((uint64_t)buffer[3] << 32)
         | ((uint64_t)buffer[4] << 24)
         | ((uint64_t)buffer[5] << 16)
         | ((uint64_t)buffer[6] << 8)
         | (uint64_t)buffer[7];
}

static size_t write_header(
    uint8_t *buffer,
    size_t buffer_size,
    ap_peer_message_type_t type,
    uint32_t payload_length)
{
    if (buffer == NULL)
        return 0;

    if (payload_length >
        AP_PEER_MAX_FRAME_SIZE - AP_PEER_HEADER_SIZE)
    {
        return 0;
    }

    if (buffer_size < AP_PEER_HEADER_SIZE + payload_length)
        return 0;

    put_u16(&buffer[0], AP_PEER_PROTOCOL_MAGIC);
    buffer[2] = AP_PEER_PROTOCOL_VERSION;
    buffer[3] = (uint8_t)type;
    put_u32(&buffer[4], payload_length);

    return AP_PEER_HEADER_SIZE;
}

static bool read_header(
    const uint8_t *buffer,
    size_t buffer_size,
    uint8_t *type,
    uint32_t *payload_length)
{
    if (buffer == NULL || type == NULL || payload_length == NULL)
        return false;

    if (buffer_size < AP_PEER_HEADER_SIZE)
        return false;

    if (get_u16(&buffer[0]) != AP_PEER_PROTOCOL_MAGIC)
        return false;

    if (buffer[2] != AP_PEER_PROTOCOL_VERSION)
        return false;

    *type = buffer[3];
    *payload_length = get_u32(&buffer[4]);

    if (*payload_length >
        AP_PEER_MAX_FRAME_SIZE - AP_PEER_HEADER_SIZE)
    {
        return false;
    }

    if (buffer_size != AP_PEER_HEADER_SIZE + *payload_length)
        return false;

    return true;
}


/*
 * --------------------------------------------------------------------------
 * CONNECT / ACCEPT / REJECT / ERROR
 * --------------------------------------------------------------------------
 */

size_t ap_peer_encode_connect(
    uint8_t *buffer,
    size_t buffer_size,
    uint32_t peer_id)
{
    const uint32_t payload_length = 5;

    if (write_header(
            buffer,
            buffer_size,
            AP_PEER_MSG_CONNECT,
            payload_length) == 0)
    {
        return 0;
    }

    buffer[8] = AP_PEER_PROTOCOL_VERSION;
    put_u32(&buffer[9], peer_id);

    return AP_PEER_HEADER_SIZE + payload_length;
}

size_t ap_peer_encode_accept(
    uint8_t *buffer,
    size_t buffer_size)
{
    const uint32_t payload_length = 1;

    if (write_header(
            buffer,
            buffer_size,
            AP_PEER_MSG_ACCEPT,
            payload_length) == 0)
    {
        return 0;
    }

    buffer[8] = AP_PEER_PROTOCOL_VERSION;

    return AP_PEER_HEADER_SIZE + payload_length;
}

size_t ap_peer_encode_reject(
    uint8_t *buffer,
    size_t buffer_size,
    uint8_t reason)
{
    const uint32_t payload_length = 1;

    if (write_header(
            buffer,
            buffer_size,
            AP_PEER_MSG_REJECT,
            payload_length) == 0)
    {
        return 0;
    }

    buffer[8] = reason;

    return AP_PEER_HEADER_SIZE + payload_length;
}

size_t ap_peer_encode_error(
    uint8_t *buffer,
    size_t buffer_size,
    ap_error_code_t error)
{
    const uint32_t payload_length = 2;

    if (write_header(
            buffer,
            buffer_size,
            AP_PEER_MSG_ERROR,
            payload_length) == 0)
    {
        return 0;
    }

    put_u16(
        &buffer[AP_PEER_HEADER_SIZE],
        (uint16_t)error);

    return AP_PEER_HEADER_SIZE + payload_length;
}


/*
 * --------------------------------------------------------------------------
 * REGISTER / UNREGISTER
 * --------------------------------------------------------------------------
 */

size_t ap_peer_encode_object_list(
    uint8_t *buffer,
    size_t buffer_size,
    ap_peer_message_type_t type,
    const ap_object_id_t *object_ids,
    size_t object_count)
{
    size_t payload_length;
    size_t offset;
    size_t i;

    if (buffer == NULL)
        return 0;

    if (object_count > 0 && object_ids == NULL)
        return 0;

    if (type != AP_PEER_MSG_REGISTER &&
        type != AP_PEER_MSG_UNREGISTER)
    {
        return 0;
    }

    if (object_count > AP_PEER_MAX_REGISTER_OBJECTS)
        return 0;

    payload_length = 2u + (object_count * sizeof(uint32_t));

    if (payload_length >
        AP_PEER_MAX_FRAME_SIZE - AP_PEER_HEADER_SIZE)
    {
        return 0;
    }

    if (write_header(
            buffer,
            buffer_size,
            type,
            (uint32_t)payload_length) == 0)
    {
        return 0;
    }

    put_u16(
        &buffer[AP_PEER_HEADER_SIZE],
        (uint16_t)object_count);

    offset = AP_PEER_HEADER_SIZE + 2u;

    for (i = 0; i < object_count; i++)
    {
        put_u32(
            &buffer[offset],
            (uint32_t)object_ids[i]);

        offset += sizeof(uint32_t);
    }

    return AP_PEER_HEADER_SIZE + payload_length;
}


/*
 * --------------------------------------------------------------------------
 * RESULT
 * --------------------------------------------------------------------------
 */

size_t ap_peer_encode_result(
    uint8_t *buffer,
    size_t buffer_size,
    ap_peer_message_type_t request_type,
    const ap_peer_registered_object_t *registered_objects,
    size_t registered_count,
    const ap_peer_failed_object_t *failed_objects,
    size_t failed_count)
{
    size_t payload_length;
    size_t offset;
    size_t i;

    if (buffer == NULL)
        return 0;

    if (request_type != AP_PEER_MSG_REGISTER &&
        request_type != AP_PEER_MSG_UNREGISTER)
    {
        return 0;
    }

    if (registered_count > AP_PEER_MAX_REGISTER_OBJECTS ||
        failed_count > AP_PEER_MAX_REGISTER_OBJECTS)
    {
        return 0;
    }

    if (registered_count > 0 && registered_objects == NULL)
        return 0;

    if (failed_count > 0 && failed_objects == NULL)
        return 0;

    payload_length =
        1u +
        2u +
        (registered_count * 5u) +
        2u +
        (failed_count * 6u);

    if (payload_length >
        AP_PEER_MAX_FRAME_SIZE - AP_PEER_HEADER_SIZE)
    {
        return 0;
    }

    if (write_header(
            buffer,
            buffer_size,
            AP_PEER_MSG_RESULT,
            (uint32_t)payload_length) == 0)
    {
        return 0;
    }

    offset = AP_PEER_HEADER_SIZE;

    /*
     * Request type.
     */
    buffer[offset++] = (uint8_t)request_type;

    /*
     * Successful objects.
     */
    put_u16(
        &buffer[offset],
        (uint16_t)registered_count);

    offset += 2u;

    for (i = 0; i < registered_count; i++)
    {
        put_u32(
            &buffer[offset],
            (uint32_t)registered_objects[i].object_id);

        offset += 4u;

        buffer[offset++] =
            (uint8_t)registered_objects[i].value_type;
    }

    /*
     * Failed objects.
     */
    put_u16(
        &buffer[offset],
        (uint16_t)failed_count);

    offset += 2u;

    for (i = 0; i < failed_count; i++)
    {
        put_u32(
            &buffer[offset],
            (uint32_t)failed_objects[i].object_id);

        offset += 4u;

        put_u16(
            &buffer[offset],
            (uint16_t)failed_objects[i].error_code);

        offset += 2u;
    }

    return AP_PEER_HEADER_SIZE + payload_length;
}


/*
 * --------------------------------------------------------------------------
 * EVENT
 * --------------------------------------------------------------------------
 */

static bool get_event_value_type(
    const ap_event_t *event,
    uint8_t *value_type)
{
    if (event == NULL ||
        event->object == NULL ||
        value_type == NULL)
    {
        return false;
    }

    switch (event->object->value_type)
    {
        case AP_VALUE_BOOL:
        case AP_VALUE_INT32:
        case AP_VALUE_FLOAT:
        case AP_VALUE_STRING:
            *value_type = (uint8_t)event->object->value_type;
            return true;

        case AP_VALUE_NONE:
        default:
            return false;
    }
}

size_t ap_peer_encode_event(
    uint8_t *buffer,
    size_t buffer_size,
    const ap_event_t *event)
{
    uint8_t value_type;
    uint32_t value_length;
    uint32_t payload_length;
    size_t offset;
    uint32_t string_length;
    uint32_t float_bits;

    if (buffer == NULL || event == NULL)
        return 0;

    if (!get_event_value_type(event, &value_type))
        return 0;

    value_length = 0;

    switch (value_type)
    {
        case AP_PEER_VALUE_BOOL:
            value_length = 1;
            break;

        case AP_PEER_VALUE_INT32:
            value_length = 4;
            break;

        case AP_PEER_VALUE_FLOAT:
            value_length = 4;
            break;

        case AP_PEER_VALUE_STRING:
            if (event->value.s == NULL)
                return 0;

            string_length = (uint32_t)strlen(event->value.s);

            if (string_length > AP_PEER_MAX_STRING_SIZE)
                return 0;

            value_length = 4 + string_length;
            break;

        default:
            return 0;
    }

    /*
     * object_id  = 4
     * timestamp  = 8
     * source     = 4
     * flags      = 1
     * value_type = 1
     */
    payload_length = 18 + value_length;

    if (payload_length >
        AP_PEER_MAX_FRAME_SIZE - AP_PEER_HEADER_SIZE)
    {
        return 0;
    }

    if (write_header(
            buffer,
            buffer_size,
            AP_PEER_MSG_EVENT,
            payload_length) == 0)
    {
        return 0;
    }

    offset = AP_PEER_HEADER_SIZE;

    put_u32(
        &buffer[offset],
        (uint32_t)event->object->id);
    offset += 4;

    put_u64(
        &buffer[offset],
        event->timestamp);
    offset += 8;

    put_u32(
        &buffer[offset],
        (uint32_t)event->source);
    offset += 4;

    buffer[offset++] = event->flags;
    buffer[offset++] = value_type;

    switch (value_type)
    {
        case AP_PEER_VALUE_BOOL:
            buffer[offset++] =
                event->value.b ? 1u : 0u;
            break;

        case AP_PEER_VALUE_INT32:
            put_u32(
                &buffer[offset],
                (uint32_t)event->value.i);
            offset += 4;
            break;

        case AP_PEER_VALUE_FLOAT:
            memcpy(
                &float_bits,
                &event->value.f,
                sizeof(float_bits));

            put_u32(
                &buffer[offset],
                float_bits);

            offset += 4;
            break;

        case AP_PEER_VALUE_STRING:
            put_u32(
                &buffer[offset],
                string_length);

            offset += 4;

            memcpy(
                &buffer[offset],
                event->value.s,
                string_length);

            offset += string_length;
            break;

        default:
            return 0;
    }

    return offset;
}


/*
 * --------------------------------------------------------------------------
 * Frame decoding
 * --------------------------------------------------------------------------
 */

bool ap_peer_decode_frame(
    const uint8_t *buffer,
    size_t buffer_size,
    ap_peer_frame_t *frame)
{
    uint8_t type;
    uint32_t payload_length;

    if (frame == NULL)
        return false;

    if (!read_header(
            buffer,
            buffer_size,
            &type,
            &payload_length))
    {
        return false;
    }

    frame->type = type;
    frame->payload = &buffer[AP_PEER_HEADER_SIZE];
    frame->payload_length = payload_length;

    return true;
}


/*
 * --------------------------------------------------------------------------
 * CONNECT / ACCEPT / REJECT / ERROR decoding
 * --------------------------------------------------------------------------
 */

bool ap_peer_decode_connect(
    const ap_peer_frame_t *frame,
    ap_peer_connect_t *connect)
{
    if (frame == NULL || connect == NULL)
        return false;

    if (frame->type != AP_PEER_MSG_CONNECT)
        return false;

    if (frame->payload_length != 5)
        return false;

    connect->protocol_version = frame->payload[0];
    connect->peer_id = get_u32(&frame->payload[1]);

    return true;
}

bool ap_peer_decode_accept(
    const ap_peer_frame_t *frame,
    ap_peer_accept_t *accept)
{
    if (frame == NULL || accept == NULL)
        return false;

    if (frame->type != AP_PEER_MSG_ACCEPT)
        return false;

    if (frame->payload_length != 1)
        return false;

    accept->protocol_version = frame->payload[0];

    return true;
}

bool ap_peer_decode_reject(
    const ap_peer_frame_t *frame,
    uint8_t *reason)
{
    if (frame == NULL || reason == NULL)
        return false;

    if (frame->type != AP_PEER_MSG_REJECT)
        return false;

    if (frame->payload_length != 1)
        return false;

    *reason = frame->payload[0];

    return true;
}

bool ap_peer_decode_error(
    const ap_peer_frame_t *frame,
    ap_error_code_t *error)
{
    if (frame == NULL || error == NULL)
        return false;

    if (frame->type != AP_PEER_MSG_ERROR)
        return false;

    if (frame->payload_length != 2u)
        return false;

    *error =
        (ap_error_code_t)get_u16(frame->payload);

    return true;
}


/*
 * --------------------------------------------------------------------------
 * REGISTER / UNREGISTER decoding
 * --------------------------------------------------------------------------
 */

bool ap_peer_decode_object_list(
    const ap_peer_frame_t *frame,
    ap_object_id_t *object_ids,
    size_t max_objects,
    size_t *object_count)
{
    uint16_t count;
    size_t expected_length;
    size_t offset;
    size_t i;

    if (frame == NULL ||
        object_count == NULL)
    {
        return false;
    }

    if (frame->type != AP_PEER_MSG_REGISTER &&
        frame->type != AP_PEER_MSG_UNREGISTER)
    {
        return false;
    }

    if (frame->payload_length < 2u)
        return false;

    count = get_u16(&frame->payload[0]);

    if ((size_t)count > max_objects)
        return false;

    if (count > 0 && object_ids == NULL)
        return false;

    expected_length =
        2u + ((size_t)count * sizeof(uint32_t));

    if (frame->payload_length != expected_length)
        return false;

    offset = 2u;

    for (i = 0; i < count; i++)
    {
        object_ids[i] =
            (ap_object_id_t)get_u32(
                &frame->payload[offset]);

        offset += sizeof(uint32_t);
    }

    *object_count = count;

    return true;
}


/*
 * --------------------------------------------------------------------------
 * RESULT decoding
 * --------------------------------------------------------------------------
 */

bool ap_peer_decode_result(
    const ap_peer_frame_t *frame,
    ap_peer_message_type_t *request_type,
    ap_peer_registered_object_t *registered_objects,
    size_t max_registered_objects,
    size_t *registered_count,
    ap_peer_failed_object_t *failed_objects,
    size_t max_failed_objects,
    size_t *failed_count)
{
    size_t offset;
    uint16_t count;
    size_t i;

    if (frame == NULL ||
        request_type == NULL ||
        registered_count == NULL ||
        failed_count == NULL)
    {
        return false;
    }

    if (frame->type != AP_PEER_MSG_RESULT)
        return false;

    if (frame->payload == NULL ||
        frame->payload_length < 5u)
    {
        return false;
    }

    offset = 0;

    *request_type =
        (ap_peer_message_type_t)frame->payload[offset++];

    if (*request_type != AP_PEER_MSG_REGISTER &&
        *request_type != AP_PEER_MSG_UNREGISTER)
    {
        return false;
    }

    /*
     * Successful objects.
     */
    count = get_u16(
        &frame->payload[offset]);

    offset += 2u;

    if ((size_t)count > max_registered_objects)
        return false;

    if (count > 0 && registered_objects == NULL)
        return false;

    if (offset + ((size_t)count * 5u) + 2u >
        frame->payload_length)
    {
        return false;
    }

    for (i = 0; i < count; i++)
    {
        registered_objects[i].object_id =
            (ap_object_id_t)get_u32(
                &frame->payload[offset]);

        offset += 4u;

        registered_objects[i].value_type =
            (ap_peer_value_type_t)
            frame->payload[offset++];
    }

    *registered_count = count;

    /*
     * Failed objects.
     */
    count = get_u16(
        &frame->payload[offset]);

    offset += 2u;

    if ((size_t)count > max_failed_objects)
        return false;

    if (count > 0 && failed_objects == NULL)
        return false;

    if (offset + ((size_t)count * 6u) !=
        frame->payload_length)
    {
        return false;
    }

    for (i = 0; i < count; i++)
    {
        failed_objects[i].object_id =
            (ap_object_id_t)get_u32(
                &frame->payload[offset]);

        offset += 4u;

        failed_objects[i].error_code =
            (ap_error_code_t)get_u16(
                &frame->payload[offset]);

        offset += 2u;
    }

    *failed_count = count;

    return true;
}


/*
 * --------------------------------------------------------------------------
 * EVENT decoding
 * --------------------------------------------------------------------------
 */

bool ap_peer_decode_event(
    const ap_peer_frame_t *frame,
    ap_peer_event_header_t *header,
    ap_event_value_t *value,
    char *string_buffer,
    size_t string_buffer_size)
{
    size_t offset;
    uint32_t value_length;
    uint32_t float_bits;
    uint32_t string_length;

    if (frame == NULL || header == NULL || value == NULL)
        return false;

    if (frame->type != AP_PEER_MSG_EVENT)
        return false;

    if (frame->payload_length < 18u)
        return false;

    offset = 0;

    header->object_id =
        (ap_object_id_t)get_u32(
            &frame->payload[offset]);
    offset += 4;

    header->timestamp =
        get_u64(
            &frame->payload[offset]);
    offset += 8;

    header->source =
        (ap_object_id_t)get_u32(
            &frame->payload[offset]);
    offset += 4;

    header->flags = frame->payload[offset++];
    header->value_type = frame->payload[offset++];

    switch (header->value_type)
    {
        case AP_PEER_VALUE_BOOL:
            value_length = 1;

            if (frame->payload_length - offset != value_length)
                return false;

            if (frame->payload[offset] > 1)
                return false;

            value->b =
                frame->payload[offset] != 0;

            return true;

        case AP_PEER_VALUE_INT32:
            value_length = 4;

            if (frame->payload_length - offset != value_length)
                return false;

            value->i =
                (int32_t)get_u32(
                    &frame->payload[offset]);

            return true;

        case AP_PEER_VALUE_FLOAT:
            value_length = 4;

            if (frame->payload_length - offset != value_length)
                return false;

            float_bits =
                get_u32(
                    &frame->payload[offset]);

            memcpy(
                &value->f,
                &float_bits,
                sizeof(value->f));

            return true;

        case AP_PEER_VALUE_STRING:
            if (frame->payload_length - offset < 4u)
                return false;

            string_length =
                get_u32(
                    &frame->payload[offset]);

            offset += 4u;

            if (string_length > AP_PEER_MAX_STRING_SIZE)
                return false;

            if (frame->payload_length - offset != string_length)
                return false;

            if (string_buffer == NULL)
                return false;

            if (string_buffer_size <= string_length)
                return false;

            memcpy(
                string_buffer,
                &frame->payload[offset],
                string_length);

            string_buffer[string_length] = '\0';

            value->s = string_buffer;

            return true;

        default:
            return false;
    }
}