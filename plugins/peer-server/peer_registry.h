#ifndef AP_PEER_REGISTRY_H
#define AP_PEER_REGISTRY_H

#include <stdbool.h>
#include <stddef.h>

#include "ap_result.h"
#include "ap_object.h"
#include "peer_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AP_PEER_MAX_OBJECTS_PER_CONNECTION 256u

typedef struct
{
    bool active;
    void *connection;
    ap_object_id_t connection_object_id;
    ap_object_id_t object_ids[AP_PEER_MAX_OBJECTS_PER_CONNECTION];
    size_t object_count;
    uint8_t rx_buffer[AP_PEER_MAX_FRAME_SIZE];
    size_t rx_length;

} ap_peer_connection_t;

/**
 * @brief Initialize the Peer connection registry.
 *
 * The registry does not allocate the connection storage.
 * The caller owns the supplied runtime connection array.
 *
 * @param connections Runtime connection array.
 * @param connection_count Number of runtime connections.
 */
void ap_peer_registry_init(ap_peer_connection_t *connections,size_t connection_count);

/**
 * @brief Allocate a free Peer runtime connection.
 *
 * @param connection Receives the allocated connection.
 *
 * @return AP_OK on success or an appropriate error code.
 */
ap_result_t ap_peer_registry_add(ap_peer_connection_t **connection);

/**
 * @brief Remove a Peer runtime connection.
 *
 * The runtime connection is cleared.
 * Network resources are not closed here.
 *
 * @param connection Runtime connection.
 *
 * @return AP_OK on success or an appropriate error code.
 */
ap_result_t ap_peer_registry_remove(ap_peer_connection_t *connection);

/**
 * @brief Find a Peer runtime connection by Connection Object ID.
 *
 * @param connection_object_id Connection Object ID.
 *
 * @return Runtime connection or NULL if not found.
 */
ap_peer_connection_t *ap_peer_registry_find(ap_object_id_t connection_object_id);

/**
 * @brief Find a registered object for a Peer connection.
 *
 * @param connection Runtime connection.
 * @param object_id Object identifier.
 *
 * @return true if the object is registered.
 */
bool ap_peer_registry_is_registered(const ap_peer_connection_t *connection, ap_object_id_t object_id);

#ifdef __cplusplus
}
#endif

#endif /* AP_PEER_REGISTRY_H */