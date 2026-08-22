#ifndef AP_INFLUX_PLUGIN_H
#define AP_INFLUX_PLUGIN_H

#include <stdint.h>

#include "ap_plugin.h"
#include "ap_result.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint32_t object_id;
    uint8_t value_type;

    char *name;

} ap_influx_mapping_t;


typedef struct
{
    char *host;
    uint16_t port;

    char *org;
    char *token;
    char *bucket;

    uint8_t mapping_count;
    ap_influx_mapping_t *mappings;

} ap_influx_config_t;

#ifdef __cplusplus
}
#endif

#endif /* AP_INFLUX_PLUGIN_H */

