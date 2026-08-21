#ifndef AP_PEER_CLIENT_H
#define AP_PEER_CLIENT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "peer_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AP_PEER_CLIENT_RX_BUFFER_SIZE AP_PEER_MAX_FRAME_SIZE

/**

* @brief Client state.
  */
  typedef enum
  {
  AP_PEER_CLIENT_DISCONNECTED = 0,
  AP_PEER_CLIENT_CONNECTING,
  AP_PEER_CLIENT_CONNECTED

} ap_peer_client_state_t;

/**

* @brief Decoded event supplied by the library.
*
* The string pointer is only valid until the next event is requested
* or the client is destroyed.
  */
  typedef struct
  {
  ap_peer_event_header_t header;
  ap_event_value_t       value;

} ap_peer_client_event_t;

/**

* @brief Client context.
  */
  typedef struct
  {
  uint8_t rx_buffer[AP_PEER_CLIENT_RX_BUFFER_SIZE];

  size_t rx_length;

  ap_peer_client_state_t state;

  uint32_t peer_id;

  char string_buffer[AP_PEER_MAX_STRING_SIZE + 1u];

} ap_peer_client_t;

/**

* @brief Initialize a client context.
  */
  void ap_peer_client_init(
  ap_peer_client_t *client,
  uint32_t peer_id);

/**

* @brief Reset the client.
*
* Clears all buffered receive data and returns the client
* to the disconnected state.
  */
  void ap_peer_client_reset(
  ap_peer_client_t *client);

/**

* @brief Get the current client state.
  */
  ap_peer_client_state_t ap_peer_client_get_state(
  const ap_peer_client_t *client);

/**

* @brief Build a CONNECT frame.
*
* The returned frame can be sent directly to the transport.
*
* @return Number of bytes written, or 0 on error.
  */
  size_t ap_peer_client_connect(
  ap_peer_client_t *client,
  uint8_t *buffer,
  size_t buffer_size);

/**

* @brief Feed received TCP data into the client.
*
* Data may contain:
*
* * a partial frame
* * exactly one frame
* * multiple frames
*
* The data is internally buffered until complete frames are available.
*
* @return AP_OK on success.
  */
  ap_result_t ap_peer_client_feed(
  ap_peer_client_t *client,
  const uint8_t *data,
  size_t data_length);

/**

* @brief Check whether a complete event is available.
  */
  bool ap_peer_client_event_available(
  const ap_peer_client_t *client);

/**

* @brief Retrieve the next decoded event.
*
* The returned event remains valid until the next call which
* modifies the client's receive state.
  */
  bool ap_peer_client_next_event(
  ap_peer_client_t *client,
  ap_peer_client_event_t *event);

/**

* @brief Build a REGISTER frame.
  */
  size_t ap_peer_client_register(
  ap_peer_client_t *client,
  uint8_t *buffer,
  size_t buffer_size,
  const ap_object_id_t *object_ids,
  size_t object_count);

/**

* @brief Build an UNREGISTER frame.
  */
  size_t ap_peer_client_unregister(
  ap_peer_client_t *client,
  uint8_t *buffer,
  size_t buffer_size,
  const ap_object_id_t *object_ids,
  size_t object_count);

/**

* @brief Build an EVENT frame.
  */
  size_t ap_peer_client_encode_event(
  ap_peer_client_t *client,
  uint8_t *buffer,
  size_t buffer_size,
  const ap_event_t *event);

#ifdef __cplusplus
}
#endif

#endif /* AP_PEER_CLIENT_H */
