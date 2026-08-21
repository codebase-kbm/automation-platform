#ifndef AP_PEER_PROTOCOL_H
#define AP_PEER_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ap_event.h"
#include "ap_object.h"
#include "ap_result.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AP_PEER_PROTOCOL_MAGIC       0x4150u
#define AP_PEER_PROTOCOL_VERSION     1u

#define AP_PEER_MAX_FRAME_SIZE       4096u
#define AP_PEER_MAX_STRING_SIZE      1024u
#define AP_PEER_MAX_REGISTER_OBJECTS 256u

/**
 * @brief Peer protocol message type.
 */
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

/**
 * @brief Connection reject reason.
 */
typedef enum
{
    AP_PEER_REJECT_UNAUTHORIZED = 0x01,
    AP_PEER_REJECT_VERSION      = 0x02,
    AP_PEER_REJECT_INVALID      = 0x03

} ap_peer_reject_reason_t;

/**
 * @brief Protocol value type.
 *
 * This is the wire representation of the AP event value type.
 */
typedef enum
{
    AP_PEER_VALUE_BOOL   = 0x01,
    AP_PEER_VALUE_INT32  = 0x02,
    AP_PEER_VALUE_FLOAT  = 0x03,
    AP_PEER_VALUE_STRING = 0x04

} ap_peer_value_type_t;

/**
 * @brief Generic protocol frame header.
 *
 * Wire format:
 *
 *   magic   : 2 bytes
 *   version : 1 byte
 *   type    : 1 byte
 *   length  : 4 bytes
 *
 * All multi-byte values are encoded big endian.
 */
#define AP_PEER_HEADER_SIZE 8u

/**
 * @brief Decoded frame.
 *
 * The payload points into the supplied receive buffer and is therefore
 * only valid as long as that buffer remains unchanged.
 */
typedef struct
{
    uint8_t type;

    const uint8_t *payload;
    uint32_t payload_length;

} ap_peer_frame_t;

/**
 * @brief Connect message.
 */
typedef struct
{
    uint8_t  protocol_version;
    uint32_t peer_id;

} ap_peer_connect_t;

/**
 * @brief Accept message.
 */
typedef struct
{
    uint8_t protocol_version;

} ap_peer_accept_t;

/**
 * @brief Registered remote object.
 *
 * Returned by the server for successfully registered objects.
 */
typedef struct
{
    ap_object_id_t      object_id;
    ap_peer_value_type_t value_type;

} ap_peer_registered_object_t;

/**
 * @brief Failed object operation.
 *
 * Returned by the server for objects which could not be processed.
 */
typedef struct
{
    ap_object_id_t object_id;
    ap_error_code_t error_code;

} ap_peer_failed_object_t;

/**
 * @brief Event wire representation.
 *
 * The value is decoded separately because STRING has variable length.
 */
typedef struct
{
    ap_object_id_t object_id;
    uint64_t       timestamp;
    ap_object_id_t source;
    uint8_t        flags;
    uint8_t        value_type;

} ap_peer_event_header_t;


/**
 * @brief Encode a CONNECT message.
 *
 * @return Number of bytes written, or 0 on error.
 */
size_t ap_peer_encode_connect(
    uint8_t *buffer,
    size_t buffer_size,
    uint32_t peer_id);

/**
 * @brief Encode an ACCEPT message.
 */
size_t ap_peer_encode_accept(
    uint8_t *buffer,
    size_t buffer_size);

/**
 * @brief Encode a REJECT message.
 */
size_t ap_peer_encode_reject(
    uint8_t *buffer,
    size_t buffer_size,
    uint8_t reason);

/**
 * @brief Encode a protocol error.
 */
size_t ap_peer_encode_error(
    uint8_t *buffer,
    size_t buffer_size,
    ap_error_code_t error);

/**
 * @brief Encode REGISTER/UNREGISTER request.
 *
 * The request contains only object identifiers.
 */
size_t ap_peer_encode_object_list(
    uint8_t *buffer,
    size_t buffer_size,
    ap_peer_message_type_t type,
    const ap_object_id_t *object_ids,
    size_t object_count);

/**
 * @brief Encode a RESULT message.
 *
 * Successfully processed objects contain their resolved value type.
 * Failed objects contain the corresponding error code.
 */
size_t ap_peer_encode_result(
    uint8_t *buffer,
    size_t buffer_size,
    ap_peer_message_type_t request_type,
    const ap_peer_registered_object_t *registered_objects,
    size_t registered_count,
    const ap_peer_failed_object_t *failed_objects,
    size_t failed_count);

/**
 * @brief Encode an event.
 */
size_t ap_peer_encode_event(
    uint8_t *buffer,
    size_t buffer_size,
    const ap_event_t *event);

/**
 * @brief Decode a complete frame.
 *
 * The caller must provide exactly one complete frame.
 */
bool ap_peer_decode_frame(
    const uint8_t *buffer,
    size_t buffer_size,
    ap_peer_frame_t *frame);

/**
 * @brief Decode CONNECT payload.
 */
bool ap_peer_decode_connect(
    const ap_peer_frame_t *frame,
    ap_peer_connect_t *connect);

/**
 * @brief Decode ACCEPT payload.
 */
bool ap_peer_decode_accept(
    const ap_peer_frame_t *frame,
    ap_peer_accept_t *accept);

/**
 * @brief Decode REJECT payload.
 */
bool ap_peer_decode_reject(
    const ap_peer_frame_t *frame,
    uint8_t *reason);

/**
 * @brief Decode ERROR payload.
 */
bool ap_peer_decode_error(
    const ap_peer_frame_t *frame,
    ap_error_code_t *error);

/**
 * @brief Decode REGISTER/UNREGISTER request.
 *
 * object_ids are written into the supplied array.
 */
bool ap_peer_decode_object_list(
    const ap_peer_frame_t *frame,
    ap_object_id_t *object_ids,
    size_t max_objects,
    size_t *object_count);

/**
 * @brief Decode a RESULT message.
 */
bool ap_peer_decode_result(
    const ap_peer_frame_t *frame,
    ap_peer_message_type_t *request_type,
    ap_peer_registered_object_t *registered_objects,
    size_t max_registered_objects,
    size_t *registered_count,
    ap_peer_failed_object_t *failed_objects,
    size_t max_failed_objects,
    size_t *failed_count);

/**
 * @brief Decode an EVENT payload.
 *
 * String values are copied into string_buffer.
 */
bool ap_peer_decode_event(
    const ap_peer_frame_t *frame,
    ap_peer_event_header_t *header,
    ap_event_value_t *value,
    char *string_buffer,
    size_t string_buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* AP_PEER_PROTOCOL_H */