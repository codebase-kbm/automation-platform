#ifndef AP_PEER_PROTOCOL_H
#define AP_PEER_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */
/* Protocol                                                                   */
/* -------------------------------------------------------------------------- */

#define AP_PEER_PROTOCOL_MAGIC       0x4150u
#define AP_PEER_PROTOCOL_VERSION     1u

#define AP_PEER_HEADER_SIZE          8u
#define AP_PEER_MAX_FRAME_SIZE       256u
#define AP_PEER_MAX_STRING_SIZE      256u
#define AP_PEER_MAX_LIST_ENTRIES     32u

/* -------------------------------------------------------------------------- */
/* Result                                                                     */
/* -------------------------------------------------------------------------- */

typedef enum
{
    AP_PEER_OK = 0,

    AP_PEER_ERROR_INVALID_ARGUMENT,

    /* Buffer / frame handling */
    AP_PEER_ERROR_INVALID_SIZE,
    AP_PEER_ERROR_INVALID_FRAME,
    AP_PEER_ERROR_BUFFER_FULL,

    /* Protocol validation */
    AP_PEER_ERROR_PROTOCOL,
    AP_PEER_ERROR_INVALID_TYPE,
    AP_PEER_ERROR_INVALID_VALUE

} ap_peer_result_code_t;

/* -------------------------------------------------------------------------- */
/* Message type                                                               */
/* -------------------------------------------------------------------------- */

typedef enum
{
    AP_PEER_MSG_CONNECT    = 0x01,
    AP_PEER_MSG_ACCEPT     = 0x02,
    AP_PEER_MSG_REJECT     = 0x03,
    AP_PEER_MSG_REGISTER   = 0x04,
    AP_PEER_MSG_UNREGISTER = 0x05,
    AP_PEER_MSG_EVENT      = 0x06,
    AP_PEER_MSG_RESULT     = 0x07,
    AP_PEER_MSG_ERROR      = 0x08
} ap_peer_message_type_t;

/* -------------------------------------------------------------------------- */
/* Value type                                                                 */
/* -------------------------------------------------------------------------- */

typedef enum
{
    AP_PEER_VALUE_NONE   = 0x00,
    AP_PEER_VALUE_BOOL   = 0x01,
    AP_PEER_VALUE_INT32  = 0x02,
    AP_PEER_VALUE_FLOAT  = 0x03,
    AP_PEER_VALUE_STRING = 0x04
} ap_peer_value_type_t;

/* -------------------------------------------------------------------------- */
/* Object                                                                     */
/* -------------------------------------------------------------------------- */

typedef uint32_t ap_peer_object_id_t;

/* -------------------------------------------------------------------------- */
/* Value                                                                      */
/* -------------------------------------------------------------------------- */

typedef struct
{
    ap_peer_value_type_t type;

    union
    {
        bool boolean;
        int32_t int32;
        float float32;

        struct
        {
            const char *data;
            size_t length;
        } string;

    } value;

} ap_peer_value_t;

/* -------------------------------------------------------------------------- */
/* REGISTER / UNREGISTER                                                      */
/* -------------------------------------------------------------------------- */

typedef struct
{
    ap_peer_object_id_t object_ids[AP_PEER_MAX_LIST_ENTRIES];
    size_t count;

} ap_peer_object_list_t;

/* -------------------------------------------------------------------------- */
/* RESULT                                                                     */
/* -------------------------------------------------------------------------- */

typedef struct
{
    ap_peer_object_id_t object_id;

    ap_peer_value_type_t value_type;

    uint32_t error_code;

    ap_peer_value_t value;

} ap_peer_result_entry_t;


typedef struct
{
    ap_peer_message_type_t request_type;

    const ap_peer_result_entry_t *entries;

    size_t count;

} ap_peer_result_message_t;

/* -------------------------------------------------------------------------- */
/* CONNECT / ACCEPT / REJECT / ERROR                                          */
/* -------------------------------------------------------------------------- */

typedef struct
{
    uint8_t protocol_version;
    uint32_t peer_id;

} ap_peer_connect_t;


typedef struct
{
    uint8_t protocol_version;

} ap_peer_accept_t;


typedef struct
{
    uint8_t reason;

} ap_peer_reject_t;


typedef struct
{
    uint32_t error_code;

} ap_peer_error_t;

/* -------------------------------------------------------------------------- */
/* EVENT                                                                      */
/* -------------------------------------------------------------------------- */

typedef struct
{
    ap_peer_object_id_t object_id;
    uint64_t timestamp;
    ap_peer_object_id_t source;
    uint8_t flags;
    ap_peer_value_t value;

} ap_peer_event_t;

/* -------------------------------------------------------------------------- */
/* Message                                                                    */
/* -------------------------------------------------------------------------- */

typedef struct
{
    ap_peer_message_type_t type;

    union
    {
        ap_peer_connect_t connect;
        ap_peer_accept_t accept;
        ap_peer_reject_t reject;

        ap_peer_object_list_t object_list;

        ap_peer_event_t event;

        ap_peer_result_message_t result;

        ap_peer_error_t error;

    } data;

} ap_peer_message_t;

/* -------------------------------------------------------------------------- */
/* Decoder                                                                    */
/* -------------------------------------------------------------------------- */

typedef struct
{
    uint8_t buffer[AP_PEER_MAX_FRAME_SIZE];

    size_t length;


    /*
     * Temporary decode storage.
     *
     * Lifetime:
     * Valid until the next call to ap_peer_decoder_feed().
     */
    ap_peer_result_entry_t result_entries[AP_PEER_MAX_LIST_ENTRIES];

    char string_buffer[AP_PEER_MAX_STRING_SIZE + 1u];

} ap_peer_decoder_t;


/**
 * @brief Initialize a Peer stream decoder.
 *
 * The decoder owns its internal receive buffer.
 */
void ap_peer_decoder_init(
    ap_peer_decoder_t *decoder);


/**
 * @brief Feed raw TCP data into the decoder.
 *
 * The supplied data may contain:
 *
 * - a partial frame
 * - one complete frame
 * - multiple complete frames
 * - complete and partial frames
 *
 * Incomplete data is retained internally.
 *
 * The callback is invoked once for every complete frame.
 *
 * The message and all pointers contained in it are only valid
 * until the next call to ap_peer_decoder_feed().
 */
ap_peer_result_code_t ap_peer_decoder_feed(
    ap_peer_decoder_t *decoder,
    const uint8_t *data,
    size_t data_size,
    ap_peer_result_code_t (*callback)(
        const ap_peer_message_t *message,
        void *context),
    void *context);

/* -------------------------------------------------------------------------- */
/* Encoder                                                                    */
/* -------------------------------------------------------------------------- */

/**
 * @brief Encode one complete Peer message.
 *
 * The caller owns the output buffer.
 */
ap_peer_result_code_t ap_peer_encode(
    const ap_peer_message_t *message,
    uint8_t *buffer,
    size_t buffer_size,
    size_t *written);

/* -------------------------------------------------------------------------- */
/* Message constructors                                                       */
/* -------------------------------------------------------------------------- */

ap_peer_result_code_t ap_peer_make_connect(
    ap_peer_message_t *message,
    uint32_t peer_id);


ap_peer_result_code_t ap_peer_make_accept(
    ap_peer_message_t *message);


ap_peer_result_code_t ap_peer_make_reject(
    ap_peer_message_t *message,
    uint8_t reason);


ap_peer_result_code_t ap_peer_make_register(
    ap_peer_message_t *message,
    const ap_peer_object_id_t *object_ids,
    size_t count);


ap_peer_result_code_t ap_peer_make_unregister(
    ap_peer_message_t *message,
    const ap_peer_object_id_t *object_ids,
    size_t count);


ap_peer_result_code_t ap_peer_make_event(
    ap_peer_message_t *message,
    ap_peer_object_id_t object_id,
    uint64_t timestamp,
    ap_peer_object_id_t source,
    uint8_t flags,
    const ap_peer_value_t *value);


ap_peer_result_code_t ap_peer_make_error(
    ap_peer_message_t *message,
    uint32_t error_code);

#ifdef __cplusplus
}
#endif

#endif /* AP_PEER_PROTOCOL_H */