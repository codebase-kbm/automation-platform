#include "peer_protocol.h"

#include <string.h>

/*
 * ==========================================================================
 * Wire helpers
 * ==========================================================================
 */

static void put_u16(
    uint8_t *buffer,
    uint16_t value)
{
    buffer[0] = (uint8_t)((value >> 8) & 0xffu);
    buffer[1] = (uint8_t)(value & 0xffu);
}


static void put_u32(
    uint8_t *buffer,
    uint32_t value)
{
    buffer[0] = (uint8_t)((value >> 24) & 0xffu);
    buffer[1] = (uint8_t)((value >> 16) & 0xffu);
    buffer[2] = (uint8_t)((value >> 8) & 0xffu);
    buffer[3] = (uint8_t)(value & 0xffu);
}


static void put_u64(
    uint8_t *buffer,
    uint64_t value)
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


static uint16_t get_u16(
    const uint8_t *buffer)
{
    return ((uint16_t)buffer[0] << 8)
         | (uint16_t)buffer[1];
}


static uint32_t get_u32(
    const uint8_t *buffer)
{
    return ((uint32_t)buffer[0] << 24)
         | ((uint32_t)buffer[1] << 16)
         | ((uint32_t)buffer[2] << 8)
         | (uint32_t)buffer[3];
}


static uint64_t get_u64(
    const uint8_t *buffer)
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


/*
 * ==========================================================================
 * Header
 * ==========================================================================
 */

static bool write_header(
    uint8_t *buffer,
    size_t buffer_size,
    ap_peer_message_type_t type,
    uint32_t payload_length)
{
    if (buffer == NULL)
        return false;

    if (payload_length >
        AP_PEER_MAX_FRAME_SIZE - AP_PEER_HEADER_SIZE)
    {
        return false;
    }

    if (buffer_size <
        AP_PEER_HEADER_SIZE + payload_length)
    {
        return false;
    }

    put_u16(
        &buffer[0],
        AP_PEER_PROTOCOL_MAGIC);

    buffer[2] =
        AP_PEER_PROTOCOL_VERSION;

    buffer[3] =
        (uint8_t)type;

    put_u32(
        &buffer[4],
        payload_length);

    return true;
}


static bool read_header(
    const uint8_t *buffer,
    size_t buffer_size,
    uint8_t *type,
    uint32_t *payload_length)
{
    if (buffer == NULL ||
        type == NULL ||
        payload_length == NULL)
    {
        return false;
    }

    if (buffer_size < AP_PEER_HEADER_SIZE)
        return false;

    if (get_u16(&buffer[0]) !=
        AP_PEER_PROTOCOL_MAGIC)
    {
        return false;
    }

    if (buffer[2] !=
        AP_PEER_PROTOCOL_VERSION)
    {
        return false;
    }

    *type =
        buffer[3];

    *payload_length =
        get_u32(&buffer[4]);

    if (*payload_length >
        AP_PEER_MAX_FRAME_SIZE - AP_PEER_HEADER_SIZE)
    {
        return false;
    }

    return true;
}


/*
 * ==========================================================================
 * Message validation
 * ==========================================================================
 */

static bool valid_value_type(
    uint8_t type)
{
    return type == AP_PEER_VALUE_BOOL ||
           type == AP_PEER_VALUE_INT32 ||
           type == AP_PEER_VALUE_FLOAT ||
           type == AP_PEER_VALUE_STRING;
}


/*
 * ==========================================================================
 * Message decoder
 * ==========================================================================
 */

static ap_peer_result_code_t decode_connect(
    const uint8_t *payload,
    size_t length,
    ap_peer_message_t *message)
{
    if (payload == NULL ||
        message == NULL)
    {
        return AP_PEER_ERROR_INVALID_ARGUMENT;
    }

    if (length != 5u)
    {
        return AP_PEER_ERROR_PROTOCOL;
    }

    message->type =
        AP_PEER_MSG_CONNECT;

    message->data.connect.protocol_version =
        payload[0];

    message->data.connect.peer_id =
        get_u32(&payload[1]);

    return AP_PEER_OK;
}


static ap_peer_result_code_t decode_accept(
    const uint8_t *payload,
    size_t length,
    ap_peer_message_t *message)
{
    if (payload == NULL ||
        message == NULL)
    {
        return AP_PEER_ERROR_INVALID_ARGUMENT;
    }

    if (length != 1u)
    {
        return AP_PEER_ERROR_PROTOCOL;
    }

    message->type =
        AP_PEER_MSG_ACCEPT;

    message->data.accept.protocol_version =
        payload[0];

    return AP_PEER_OK;
}

static ap_peer_result_code_t decode_reject(
    const uint8_t *payload,
    size_t length,
    ap_peer_message_t *message)
{
    if (payload == NULL ||
        message == NULL)
    {
        return AP_PEER_ERROR_INVALID_ARGUMENT;
    }

    if (length != 1u)
    {
        return AP_PEER_ERROR_PROTOCOL;
    }

    message->type =
        AP_PEER_MSG_REJECT;

    message->data.reject.reason =
        payload[0];

    return AP_PEER_OK;
}


static ap_peer_result_code_t decode_error(
    const uint8_t *payload,
    size_t length,
    ap_peer_message_t *message)
{
    if (payload == NULL ||
        message == NULL)
    {
        return AP_PEER_ERROR_INVALID_ARGUMENT;
    }

    if (length != 4u)
    {
        return AP_PEER_ERROR_PROTOCOL;
    }

    message->type =
        AP_PEER_MSG_ERROR;

    message->data.error.error_code =
        get_u32(payload);

    return AP_PEER_OK;
}

static ap_peer_result_code_t decode_object_list(
    const uint8_t *payload,
    size_t length,
    ap_peer_message_type_t type,
    ap_peer_message_t *message)
{
    size_t offset = 0;
    uint16_t count;
    size_t i;


    if (length < 2u)
        return AP_PEER_ERROR_INVALID_SIZE;


    count = get_u16(payload);

    offset += 2u;


    if (count > AP_PEER_MAX_LIST_ENTRIES)
        return AP_PEER_ERROR_INVALID_SIZE;


    if (length !=
        2u + ((size_t)count * 4u))
    {
        return AP_PEER_ERROR_INVALID_SIZE;
    }


    message->type = type;

    message->data.object_list.count =
        count;


    for (i = 0; i < count; i++)
    {
        message->data.object_list.object_ids[i] =
            get_u32(&payload[offset]);

        offset += 4u;
    }


    return AP_PEER_OK;
}


/*
 * RESULT
 *
 * The current public API describes the result using arrays. The actual
 * object records are decoded into the temporary storage supplied by the
 * decoder implementation below.
 */

static ap_peer_result_code_t decode_result(
    const uint8_t *payload,
    size_t length,
    ap_peer_message_t *message,
    ap_peer_result_entry_t *entries)
{
    size_t offset;
    uint16_t count;
    size_t i;

    if (length < 3u)
        return AP_PEER_ERROR_PROTOCOL;

    offset = 0;

    message->type =
        AP_PEER_MSG_RESULT;

    message->data.result.request_type =
        (ap_peer_message_type_t)payload[offset++];

    if (message->data.result.request_type !=
            AP_PEER_MSG_REGISTER &&
        message->data.result.request_type !=
            AP_PEER_MSG_UNREGISTER)
    {
        return AP_PEER_ERROR_PROTOCOL;
    }


    count =
        get_u16(&payload[offset]);

    offset += 2u;


    if (count > AP_PEER_MAX_LIST_ENTRIES)
        return AP_PEER_ERROR_INVALID_SIZE;


    if (offset +
        ((size_t)count * 9u) !=
        length)
    {
        return AP_PEER_ERROR_PROTOCOL;
    }


    for (i = 0; i < count; i++)
    {
        entries[i].object_id =
            get_u32(&payload[offset]);

        offset += 4u;


        entries[i].value_type =
            (ap_peer_value_type_t)
            payload[offset++];

        entries[i].error_code =
            get_u32(&payload[offset]);

        offset += 4u;


        /*
         * Wert wird nur bei erfolgreicher Registrierung
         * bzw. späterer Erweiterung benutzt.
         *
         * Aktuell:
         * kein Value im RESULT Frame.
         */

        memset(
            &entries[i].value,
            0,
            sizeof(entries[i].value));
    }


    message->data.result.entries =
        entries;

    message->data.result.count =
        count;


    return AP_PEER_OK;
}


static ap_peer_result_code_t decode_event(
    const uint8_t *payload,
    size_t length,
    ap_peer_message_t *message,
    char *string_buffer,
    size_t string_buffer_size)
{
    size_t offset;
    uint8_t value_type;
    uint32_t string_length;
    uint32_t float_bits;


    if (payload == NULL ||
        message == NULL)
    {
        return AP_PEER_ERROR_INVALID_ARGUMENT;
    }


    /*
     * Fixed part:
     *
     * object_id   4 byte
     * timestamp   8 byte
     * source      4 byte
     * flags       1 byte
     * value type  1 byte
     *
     * = 18 byte
     */
    if (length < 18u)
    {
        return AP_PEER_ERROR_PROTOCOL;
    }


    offset = 0;


    message->type =
        AP_PEER_MSG_EVENT;


    message->data.event.object_id =
        get_u32(&payload[offset]);

    offset += 4u;


    message->data.event.timestamp =
        get_u64(&payload[offset]);

    offset += 8u;


    message->data.event.source =
        get_u32(&payload[offset]);

    offset += 4u;


    message->data.event.flags =
        payload[offset++];


    value_type =
        payload[offset++];


    if (!valid_value_type(value_type))
    {
        return AP_PEER_ERROR_PROTOCOL;
    }


    message->data.event.value.type =
        (ap_peer_value_type_t)value_type;


    switch (value_type)
    {
        case AP_PEER_VALUE_BOOL:

            if (length - offset != 1u)
            {
                return AP_PEER_ERROR_PROTOCOL;
            }

            if (payload[offset] > 1u)
            {
                return AP_PEER_ERROR_PROTOCOL;
            }

            message->data.event.value.value.boolean =
                (payload[offset] != 0);

            break;


        case AP_PEER_VALUE_INT32:

            if (length - offset != 4u)
            {
                return AP_PEER_ERROR_PROTOCOL;
            }

            message->data.event.value.value.int32 =
                (int32_t)get_u32(&payload[offset]);

            break;


        case AP_PEER_VALUE_FLOAT:

            if (length - offset != 4u)
            {
                return AP_PEER_ERROR_PROTOCOL;
            }

            float_bits =
                get_u32(&payload[offset]);

            memcpy(
                &message->data.event.value.value.float32,
                &float_bits,
                sizeof(float_bits));

            break;


        case AP_PEER_VALUE_STRING:

            if (length - offset < 4u)
            {
                return AP_PEER_ERROR_PROTOCOL;
            }


            string_length =
                get_u32(&payload[offset]);

            offset += 4u;


            if (string_length >
                AP_PEER_MAX_STRING_SIZE)
            {
                return AP_PEER_ERROR_PROTOCOL;
            }


            if (length - offset != string_length)
            {
                return AP_PEER_ERROR_PROTOCOL;
            }


            if (string_buffer == NULL ||
                string_buffer_size <= string_length)
            {
                return AP_PEER_ERROR_BUFFER_FULL;
            }


            memcpy(
                string_buffer,
                &payload[offset],
                string_length);


            string_buffer[string_length] =
                '\0';


            message->data.event.value.value.string.data =
                string_buffer;


            message->data.event.value.value.string.length =
                string_length;

            break;


        default:
            return AP_PEER_ERROR_PROTOCOL;
    }


    return AP_PEER_OK;
}


/*
 * ==========================================================================
 * Frame decoder
 * ==========================================================================
 */

static ap_peer_result_code_t decode_frame(
    const uint8_t *buffer,
    size_t frame_length,
    ap_peer_message_t *message,
    ap_peer_result_entry_t *entries,
    char *string_buffer,
    size_t string_buffer_size)
{
    uint8_t type;
    uint32_t payload_length;


    if (buffer == NULL ||
        message == NULL)
    {
        return AP_PEER_ERROR_INVALID_ARGUMENT;
    }


    if (!read_header(
            buffer,
            frame_length,
            &type,
            &payload_length))
    {
        return AP_PEER_ERROR_PROTOCOL;
    }


    if (frame_length !=
        AP_PEER_HEADER_SIZE + payload_length)
    {
        return AP_PEER_ERROR_PROTOCOL;
    }


    memset(
        message,
        0,
        sizeof(*message));


    switch ((ap_peer_message_type_t)type)
    {
        case AP_PEER_MSG_CONNECT:

            return decode_connect(
                &buffer[AP_PEER_HEADER_SIZE],
                payload_length,
                message);


        case AP_PEER_MSG_ACCEPT:

            return decode_accept(
                &buffer[AP_PEER_HEADER_SIZE],
                payload_length,
                message);


        case AP_PEER_MSG_REJECT:

            return decode_reject(
                &buffer[AP_PEER_HEADER_SIZE],
                payload_length,
                message);


        case AP_PEER_MSG_REGISTER:
        case AP_PEER_MSG_UNREGISTER:

            return decode_object_list(
                &buffer[AP_PEER_HEADER_SIZE],
                payload_length,
                (ap_peer_message_type_t)type,
                message);


        case AP_PEER_MSG_RESULT:
            return decode_result(
                &buffer[AP_PEER_HEADER_SIZE],
                payload_length,
                message,
                entries);


        case AP_PEER_MSG_EVENT:

            return decode_event(
                &buffer[AP_PEER_HEADER_SIZE],
                payload_length,
                message,
                string_buffer,
                string_buffer_size);


        case AP_PEER_MSG_ERROR:

            return decode_error(
                &buffer[AP_PEER_HEADER_SIZE],
                payload_length,
                message);


        default:

            return AP_PEER_ERROR_PROTOCOL;
    }
}


/*
 * ==========================================================================
 * Decoder API
 * ==========================================================================
 */

void ap_peer_decoder_init(
    ap_peer_decoder_t *decoder)
{
    if (decoder == NULL)
    {
        return;
    }

    memset(
        decoder,
        0,
        sizeof(*decoder));
}


ap_peer_result_code_t ap_peer_decoder_feed(
    ap_peer_decoder_t *decoder,
    const uint8_t *data,
    size_t data_size,
    ap_peer_result_code_t (*callback)(
        const ap_peer_message_t *message,
        void *context),
    void *context)
{
    size_t offset;
    uint32_t payload_length;
    size_t frame_length;

    ap_peer_message_t message;

    if (decoder == NULL ||
        callback == NULL)
    {
        return AP_PEER_ERROR_INVALID_ARGUMENT;
    }

    if (data == NULL && data_size != 0)
        return AP_PEER_ERROR_INVALID_ARGUMENT;

    if (data_size == 0)
        return AP_PEER_OK;

    if (data_size >
        AP_PEER_MAX_FRAME_SIZE - decoder->length)
    {
        return AP_PEER_ERROR_BUFFER_FULL;
    }

    memcpy(
        &decoder->buffer[decoder->length],
        data,
        data_size);

    decoder->length +=
        data_size;

    offset = 0;

    while (decoder->length - offset >=
           AP_PEER_HEADER_SIZE)
    {
        if (get_u16(
                &decoder->buffer[offset]) !=
            AP_PEER_PROTOCOL_MAGIC)
        {
            return AP_PEER_ERROR_PROTOCOL;
        }

        if (decoder->buffer[offset + 2] !=
            AP_PEER_PROTOCOL_VERSION)
        {
            return AP_PEER_ERROR_PROTOCOL;
        }

        payload_length =
            get_u32(
                &decoder->buffer[offset + 4]);

        if (payload_length >
            AP_PEER_MAX_FRAME_SIZE -
            AP_PEER_HEADER_SIZE)
        {
            return AP_PEER_ERROR_INVALID_SIZE;
        }

        frame_length =
            AP_PEER_HEADER_SIZE +
            (size_t)payload_length;

        if (decoder->length - offset <
            frame_length)
        {
            break;
        }

        /*
         * Decode one complete frame.
         */
        ap_peer_result_code_t result =
            decode_frame(
                &decoder->buffer[offset],
                frame_length,
                &message,
                decoder->result_entries,
                decoder->string_buffer,
                sizeof(decoder->string_buffer));

        if (result != AP_PEER_OK)
            return result;

        result =
            callback(
                &message,
                context);

        if (result != AP_PEER_OK)
            return result;

        offset +=
            frame_length;
    }

    /*
     * Keep incomplete bytes.
     */
    if (offset > 0)
    {
        size_t remaining =
            decoder->length - offset;

        if (remaining > 0)
        {
            memmove(
                decoder->buffer,
                &decoder->buffer[offset],
                remaining);
        }

        decoder->length =
            remaining;
    }

    return AP_PEER_OK;
}


/*
 * ==========================================================================
 * Encoder helpers
 * ==========================================================================
 */

static ap_peer_result_code_t encode_object_list(
    const ap_peer_message_t *message,
    uint8_t *buffer,
    size_t buffer_size,
    size_t *written)
{
    size_t count;
    size_t payload_length;
    size_t offset;
    size_t i;

    count =
        message->data.object_list.count;

    if (count >
        AP_PEER_MAX_LIST_ENTRIES)
    {
        return AP_PEER_ERROR_INVALID_SIZE;
    }

    payload_length =
        2u + (count * 4u);

    if (!write_header(
            buffer,
            buffer_size,
            message->type,
            (uint32_t)payload_length))
    {
        return AP_PEER_ERROR_INVALID_SIZE;
    }

    offset =
        AP_PEER_HEADER_SIZE;

    put_u16(
        &buffer[offset],
        (uint16_t)count);

    offset += 2u;

    for (i = 0; i < count; i++)
    {
        put_u32(
            &buffer[offset],
            message->data.object_list.object_ids[i]);

        offset += 4u;
    }

    *written =
        offset;

    return AP_PEER_OK;
}


static ap_peer_result_code_t encode_event(
    const ap_peer_event_t *event,
    uint8_t *buffer,
    size_t buffer_size,
    size_t *written)
{
    size_t value_length;
    size_t payload_length;
    size_t offset;
    uint32_t float_bits;

    if (event == NULL)
        return AP_PEER_ERROR_INVALID_ARGUMENT;

    switch (event->value.type)
    {
        case AP_PEER_VALUE_BOOL:
            value_length = 1u;
            break;

        case AP_PEER_VALUE_INT32:
            value_length = 4u;
            break;

        case AP_PEER_VALUE_FLOAT:
            value_length = 4u;
            break;

        case AP_PEER_VALUE_STRING:
            if (event->value.value.string.data == NULL)
                return AP_PEER_ERROR_INVALID_ARGUMENT;

            if (event->value.value.string.length >
                AP_PEER_MAX_STRING_SIZE)
            {
                return AP_PEER_ERROR_INVALID_SIZE;
            }

            value_length =
                4u +
                event->value.value.string.length;

            break;

        default:
            return AP_PEER_ERROR_PROTOCOL;
    }

    payload_length =
        18u + value_length;

    if (!write_header(
            buffer,
            buffer_size,
            AP_PEER_MSG_EVENT,
            (uint32_t)payload_length))
    {
        return AP_PEER_ERROR_INVALID_SIZE;
    }

    offset =
        AP_PEER_HEADER_SIZE;

    put_u32(
        &buffer[offset],
        event->object_id);

    offset += 4u;

    put_u64(
        &buffer[offset],
        event->timestamp);

    offset += 8u;

    put_u32(
        &buffer[offset],
        event->source);

    offset += 4u;

    buffer[offset++] =
        event->flags;

    buffer[offset++] =
        (uint8_t)event->value.type;

    switch (event->value.type)
    {
        case AP_PEER_VALUE_BOOL:
            buffer[offset++] =
                event->value.value.boolean ? 1u : 0u;
            break;

        case AP_PEER_VALUE_INT32:
            put_u32(
                &buffer[offset],
                (uint32_t)
                event->value.value.int32);

            offset += 4u;
            break;

        case AP_PEER_VALUE_FLOAT:
            memcpy(
                &float_bits,
                &event->value.value.float32,
                sizeof(float_bits));

            put_u32(
                &buffer[offset],
                float_bits);

            offset += 4u;
            break;

        case AP_PEER_VALUE_STRING:
            put_u32(
                &buffer[offset],
                (uint32_t)
                event->value.value.string.length);

            offset += 4u;

            memcpy(
                &buffer[offset],
                event->value.value.string.data,
                event->value.value.string.length);

            offset +=
                event->value.value.string.length;

            break;

        default:
            return AP_PEER_ERROR_PROTOCOL;
    }

    *written =
        offset;

    return AP_PEER_OK;
}


/*
 * ==========================================================================
 * Encoder API
 * ==========================================================================
 */

ap_peer_result_code_t ap_peer_encode(
    const ap_peer_message_t *message,
    uint8_t *buffer,
    size_t buffer_size,
    size_t *written)
{
    size_t payload_length;

    if (message == NULL ||
        buffer == NULL ||
        written == NULL)
    {
        return AP_PEER_ERROR_INVALID_ARGUMENT;
    }

    *written = 0;

    switch (message->type)
    {
        case AP_PEER_MSG_CONNECT:
            payload_length = 5u;

            if (!write_header(
                    buffer,
                    buffer_size,
                    AP_PEER_MSG_CONNECT,
                    payload_length))
            {
                return AP_PEER_ERROR_INVALID_SIZE;
            }

            buffer[8] =
                AP_PEER_PROTOCOL_VERSION;

            put_u32(
                &buffer[9],
                message->data.connect.peer_id);

            *written =
                AP_PEER_HEADER_SIZE +
                payload_length;

            return AP_PEER_OK;


        case AP_PEER_MSG_ACCEPT:
            payload_length = 1u;

            if (!write_header(
                    buffer,
                    buffer_size,
                    AP_PEER_MSG_ACCEPT,
                    payload_length))
            {
                return AP_PEER_ERROR_INVALID_SIZE;
            }

            buffer[8] =
                AP_PEER_PROTOCOL_VERSION;

            *written =
                AP_PEER_HEADER_SIZE +
                payload_length;

            return AP_PEER_OK;


        case AP_PEER_MSG_REJECT:
            payload_length = 1u;

            if (!write_header(
                    buffer,
                    buffer_size,
                    AP_PEER_MSG_REJECT,
                    payload_length))
            {
                return AP_PEER_ERROR_INVALID_SIZE;
            }

            buffer[8] =
                message->data.reject.reason;

            *written =
                AP_PEER_HEADER_SIZE +
                payload_length;

            return AP_PEER_OK;


        case AP_PEER_MSG_REGISTER:
        case AP_PEER_MSG_UNREGISTER:
            return encode_object_list(
                message,
                buffer,
                buffer_size,
                written);


        case AP_PEER_MSG_EVENT:
            return encode_event(
                &message->data.event,
                buffer,
                buffer_size,
                written);


        case AP_PEER_MSG_ERROR:
            payload_length = 2u;

            if (!write_header(
                    buffer,
                    buffer_size,
                    AP_PEER_MSG_ERROR,
                    payload_length))
            {
                return AP_PEER_ERROR_INVALID_SIZE;
            }

            put_u16(
                &buffer[8],
                (uint16_t)
                message->data.error.error_code);

            *written =
                AP_PEER_HEADER_SIZE +
                payload_length;

            return AP_PEER_OK;


        case AP_PEER_MSG_RESULT:
            /*
             * RESULT encoding is implemented below through the
             * dedicated result serializer.
             */
            break;


        default:
            return AP_PEER_ERROR_PROTOCOL;
    }

    /*
    * RESULT
    */
    {
        const ap_peer_result_message_t *result =
            &message->data.result;

        size_t offset;
        size_t i;

        size_t payload_length =
            1u +
            2u +
            (result->count *
            sizeof(ap_peer_result_entry_t));   // später genauer definieren

        if (result->request_type != AP_PEER_MSG_REGISTER &&
            result->request_type != AP_PEER_MSG_UNREGISTER)
        {
            return AP_PEER_ERROR_INVALID_TYPE;
        }

        if (result->count >
            AP_PEER_MAX_LIST_ENTRIES)
        {
            return AP_PEER_ERROR_INVALID_SIZE;
        }

        if (result->count > 0 &&
            result->entries == NULL)
        {
            return AP_PEER_ERROR_INVALID_ARGUMENT;
        }


        if (!write_header(
                buffer,
                buffer_size,
                AP_PEER_MSG_RESULT,
                (uint32_t)payload_length))
        {
            return AP_PEER_ERROR_INVALID_SIZE;
        }


        offset =
            AP_PEER_HEADER_SIZE;


        buffer[offset++] =
            (uint8_t)result->request_type;


        put_u16(
            &buffer[offset],
            (uint16_t)result->count);

        offset += 2u;


        for (i = 0; i < result->count; i++)
        {
            put_u32(
                &buffer[offset],
                result->entries[i].object_id);

            offset += 4u;


            buffer[offset++] =
                (uint8_t)result->entries[i].value_type;


            put_u32(
                &buffer[offset],
                result->entries[i].error_code);

            offset += 4u;
        }


        *written =
            offset;

        return AP_PEER_OK;
    }
}


/*
 * ==========================================================================
 * Constructors
 * ==========================================================================
 */

static ap_peer_result_code_t make_simple(
    ap_peer_message_t *message,
    ap_peer_message_type_t type)
{
    if (message == NULL)
        return AP_PEER_ERROR_INVALID_ARGUMENT;

    memset(
        message,
        0,
        sizeof(*message));

    message->type =
        type;

    return AP_PEER_OK;
}


ap_peer_result_code_t ap_peer_make_connect(
    ap_peer_message_t *message,
    uint32_t peer_id)
{
    ap_peer_result_code_t result =
        make_simple(
            message,
            AP_PEER_MSG_CONNECT);

    if (result != AP_PEER_OK)
        return result;

    message->data.connect.protocol_version =
        AP_PEER_PROTOCOL_VERSION;

    message->data.connect.peer_id =
        peer_id;

    return AP_PEER_OK;
}


ap_peer_result_code_t ap_peer_make_accept(
    ap_peer_message_t *message)
{
    ap_peer_result_code_t result =
        make_simple(
            message,
            AP_PEER_MSG_ACCEPT);

    if (result != AP_PEER_OK)
        return result;

    message->data.accept.protocol_version =
        AP_PEER_PROTOCOL_VERSION;

    return AP_PEER_OK;
}


ap_peer_result_code_t ap_peer_make_reject(
    ap_peer_message_t *message,
    uint8_t reason)
{
    ap_peer_result_code_t result =
        make_simple(
            message,
            AP_PEER_MSG_REJECT);

    if (result != AP_PEER_OK)
        return result;

    message->data.reject.reason =
        reason;

    return AP_PEER_OK;
}


ap_peer_result_code_t ap_peer_make_register(
    ap_peer_message_t *message,
    const ap_peer_object_id_t *object_ids,
    size_t object_count)
{
    ap_peer_result_code_t result;

    if (object_count > 0 &&
        object_ids == NULL)
    {
        return AP_PEER_ERROR_INVALID_ARGUMENT;
    }

    if (object_count >
        AP_PEER_MAX_LIST_ENTRIES)
    {
        return AP_PEER_ERROR_INVALID_SIZE;
    }

    result =
        make_simple(
            message,
            AP_PEER_MSG_REGISTER);

    if (result != AP_PEER_OK)
        return result;

memcpy(
    message->data.object_list.object_ids,
    object_ids,
    object_count * sizeof(ap_peer_object_id_t));

    message->data.object_list.count =
        object_count;

    return AP_PEER_OK;
}


ap_peer_result_code_t ap_peer_make_unregister(
    ap_peer_message_t *message,
    const ap_peer_object_id_t *object_ids,
    size_t object_count)
{
    ap_peer_result_code_t result;

    if (object_count > 0 &&
        object_ids == NULL)
    {
        return AP_PEER_ERROR_INVALID_ARGUMENT;
    }

    if (object_count >
        AP_PEER_MAX_LIST_ENTRIES)
    {
        return AP_PEER_ERROR_INVALID_SIZE;
    }

    result =
        make_simple(
            message,
            AP_PEER_MSG_UNREGISTER);

    if (result != AP_PEER_OK)
        return result;

    memcpy(
    message->data.object_list.object_ids,
    object_ids,
    object_count * sizeof(ap_peer_object_id_t));

    message->data.object_list.count =
        object_count;

    return AP_PEER_OK;
}


ap_peer_result_code_t ap_peer_make_event(
    ap_peer_message_t *message,
    ap_peer_object_id_t object_id,
    uint64_t timestamp,
    ap_peer_object_id_t source,
    uint8_t flags,
    const ap_peer_value_t *value)
{
    ap_peer_result_code_t result;

    if (value == NULL)
        return AP_PEER_ERROR_INVALID_ARGUMENT;

    result =
        make_simple(
            message,
            AP_PEER_MSG_EVENT);

    if (result != AP_PEER_OK)
        return result;

    message->data.event.object_id =
        object_id;

    message->data.event.timestamp =
        timestamp;

    message->data.event.source =
        source;

    message->data.event.flags =
        flags;

    message->data.event.value =
        *value;

    return AP_PEER_OK;
}


ap_peer_result_code_t ap_peer_make_error(
    ap_peer_message_t *message,
    uint32_t error_code)
{
    ap_peer_result_code_t result =
        make_simple(
            message,
            AP_PEER_MSG_ERROR);

    if (result != AP_PEER_OK)
        return result;

    message->data.error.error_code =
        error_code;

    return AP_PEER_OK;
}
