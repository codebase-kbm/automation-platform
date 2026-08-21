#include "peer_client.h"

#include <string.h>

/*

* ---
* Internal helpers
* ---

*/

static ap_result_t client_error(
ap_error_code_t error)
{
return AP_RESULT_MAKE(
AP_RESULT_SOURCE_PLUGIN,
AP_COMPONENT_NETWORK,
AP_PLUGIN_PEER_CLIENT,
error
);
}

static size_t frame_size(
const uint8_t *buffer,
size_t length)
{
uint32_t payload_length;


if (buffer == NULL ||
    length < AP_PEER_HEADER_SIZE)
{
    return 0;
}

/*
 * Header:
 *
 * 0..1  magic
 * 2     version
 * 3     type
 * 4..7  payload length
 */
payload_length =
    ((uint32_t)buffer[4] << 24) |
    ((uint32_t)buffer[5] << 16) |
    ((uint32_t)buffer[6] << 8)  |
    (uint32_t)buffer[7];

if (payload_length >
    AP_PEER_MAX_FRAME_SIZE - AP_PEER_HEADER_SIZE)
{
    return 0;
}

return AP_PEER_HEADER_SIZE + payload_length;


}

/*

* ---
* Initialization
* ---

*/

void ap_peer_client_init(
ap_peer_client_t *client,
uint32_t peer_id)
{
if (client == NULL)
return;


memset(
    client,
    0,
    sizeof(*client)
);

client->state =
    AP_PEER_CLIENT_DISCONNECTED;

client->peer_id =
    peer_id;


}

void ap_peer_client_reset(
ap_peer_client_t *client)
{
if (client == NULL)
return;


client->rx_length = 0;

client->state =
    AP_PEER_CLIENT_DISCONNECTED;

memset(
    client->string_buffer,
    0,
    sizeof(client->string_buffer)
);


}

ap_peer_client_state_t
ap_peer_client_get_state(
const ap_peer_client_t *client)
{
if (client == NULL)
return AP_PEER_CLIENT_DISCONNECTED;


return client->state;


}

/*

* ---
* CONNECT
* ---

*/

size_t ap_peer_client_connect(
ap_peer_client_t *client,
uint8_t *buffer,
size_t buffer_size)
{
size_t length;


if (client == NULL)
    return 0;

length =
    ap_peer_encode_connect(
        buffer,
        buffer_size,
        client->peer_id
    );

if (length == 0)
    return 0;

client->state =
    AP_PEER_CLIENT_CONNECTING;

return length;


}

/*

* ---
* Receive
* ---

*/

ap_result_t ap_peer_client_feed(
ap_peer_client_t *client,
const uint8_t *data,
size_t data_length)
{
size_t frame_length;
size_t consumed;
ap_peer_frame_t frame;


if (client == NULL)
    return client_error(
        AP_ERROR_INVALID_ARGUMENT
    );

if (data_length == 0)
    return AP_OK;

if (data == NULL)
    return client_error(
        AP_ERROR_INVALID_ARGUMENT
    );

if (data_length >
    sizeof(client->rx_buffer) - client->rx_length)
{
    return client_error(
        AP_ERROR_FULL
    );
}

memcpy(
    &client->rx_buffer[client->rx_length],
    data,
    data_length
);

client->rx_length += data_length;

/*
 * Consume every complete frame currently available.
 *
 * At the moment we only need to retain EVENT frames.
 * Other protocol messages are handled immediately.
 */
consumed = 0;

while (client->rx_length - consumed >=
       AP_PEER_HEADER_SIZE)
{
    frame_length =
        frame_size(
            &client->rx_buffer[consumed],
            client->rx_length - consumed
        );

    if (frame_length == 0)
    {
        /*
         * We have enough bytes for the header, therefore
         * a zero frame size means an invalid frame.
         */
        return client_error(
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    if (client->rx_length - consumed <
        frame_length)
    {
        /*
         * Partial frame. Keep it buffered.
         */
        break;
    }

    if (!ap_peer_decode_frame(
            &client->rx_buffer[consumed],
            frame_length,
            &frame))
    {
        return client_error(
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    switch (frame.type)
    {
        case AP_PEER_MSG_ACCEPT:
        {
            ap_peer_accept_t accept;

            if (!ap_peer_decode_accept(
                    &frame,
                    &accept))
            {
                return client_error(
                    AP_ERROR_INVALID_ARGUMENT
                );
            }

            if (accept.protocol_version !=
                AP_PEER_PROTOCOL_VERSION)
            {
                client->state =
                    AP_PEER_CLIENT_DISCONNECTED;

                return client_error(
                    AP_ERROR_INVALID_ARGUMENT
                );
            }

            client->state =
                AP_PEER_CLIENT_CONNECTED;

            break;
        }

        case AP_PEER_MSG_REJECT:
            client->state =
                AP_PEER_CLIENT_DISCONNECTED;
            break;

        case AP_PEER_MSG_ERROR:
            /*
             * The application can inspect protocol errors
             * through a future result/event interface.
             *
             * For now the frame is consumed.
             */
            break;

        case AP_PEER_MSG_EVENT:
            /*
             * EVENT remains in the receive buffer until
             * ap_peer_client_next_event() consumes it.
             *
             * Therefore stop processing here.
             */
            return AP_OK;

        case AP_PEER_MSG_RESULT:
            /*
             * RESULT handling will be added together with
             * the public result retrieval API.
             */
            break;

        default:
            /*
             * CONNECT/REGISTER/UNREGISTER are not expected
             * as incoming messages for the client at this
             * layer.
             */
            break;
    }

    consumed += frame_length;
}

if (consumed > 0)
{
    memmove(
        client->rx_buffer,
        &client->rx_buffer[consumed],
        client->rx_length - consumed
    );

    client->rx_length -= consumed;
}

return AP_OK;


}

/*

* ---
* EVENT
* ---

*/

bool ap_peer_client_event_available(
const ap_peer_client_t *client)
{
size_t frame_length;


if (client == NULL)
    return false;

if (client->rx_length < AP_PEER_HEADER_SIZE)
    return false;

frame_length =
    frame_size(
        client->rx_buffer,
        client->rx_length
    );

if (frame_length == 0)
    return false;

if (client->rx_length < frame_length)
    return false;

return client->rx_buffer[3] ==
       AP_PEER_MSG_EVENT;


}

bool ap_peer_client_next_event(
ap_peer_client_t *client,
ap_peer_client_event_t *event)
{
size_t frame_length;
ap_peer_frame_t frame;


if (client == NULL ||
    event == NULL)
{
    return false;
}

if (!ap_peer_client_event_available(client))
    return false;

frame_length =
    frame_size(
        client->rx_buffer,
        client->rx_length
    );

if (!ap_peer_decode_frame(
        client->rx_buffer,
        frame_length,
        &frame))
{
    return false;
}

if (!ap_peer_decode_event(
        &frame,
        &event->header,
        &event->value,
        client->string_buffer,
        sizeof(client->string_buffer)))
{
    return false;
}

memmove(
    client->rx_buffer,
    &client->rx_buffer[frame_length],
    client->rx_length - frame_length
);

client->rx_length -= frame_length;

return true;


}

/*

* ---
* REGISTER / UNREGISTER
* ---

*/

size_t ap_peer_client_register(
ap_peer_client_t *client,
uint8_t *buffer,
size_t buffer_size,
const ap_object_id_t *object_ids,
size_t object_count)
{
if (client == NULL)
return 0;


return ap_peer_encode_object_list(
    buffer,
    buffer_size,
    AP_PEER_MSG_REGISTER,
    object_ids,
    object_count
);


}

size_t ap_peer_client_unregister(
ap_peer_client_t *client,
uint8_t *buffer,
size_t buffer_size,
const ap_object_id_t *object_ids,
size_t object_count)
{
if (client == NULL)
return 0;


return ap_peer_encode_object_list(
    buffer,
    buffer_size,
    AP_PEER_MSG_UNREGISTER,
    object_ids,
    object_count
);


}

/*

* ---
* EVENT encoding
* ---

*/

size_t ap_peer_client_encode_event(
ap_peer_client_t *client,
uint8_t *buffer,
size_t buffer_size,
const ap_event_t *event)
{
if (client == NULL)
return 0;


return ap_peer_encode_event(
    buffer,
    buffer_size,
    event
);


}
