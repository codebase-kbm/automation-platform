#ifndef AP_PEER_SERVER_H
#define AP_PEER_SERVER_H

#include <stddef.h>
#include <stdint.h>

#include "ap_object.h"
#include "ap_plugin.h"
#include "ap_result.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AP_PEER_SERVER_CONFIG_VERSION 1u

typedef struct
{
    ap_object_id_t object_id;

    char *endpoint;

} ap_peer_server_connection_config_t;


typedef struct
{
    uint16_t port;

    uint8_t connection_count;

    ap_peer_server_connection_config_t *connections;

} ap_peer_server_config_t;


extern const ap_plugin_t ap_peer_server_plugin;

#ifdef __cplusplus
}
#endif

#endif /* AP_PEER_SERVER_H */