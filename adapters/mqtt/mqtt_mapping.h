#ifndef AP_MQTT_MAPPING_H
#define AP_MQTT_MAPPING_H

#include <stdint.h>

#include "ap_result.h"
#include "ap_signal.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef enum
{
    AP_MQTT_DIRECTION_SUBSCRIBE,
    AP_MQTT_DIRECTION_PUBLISH,
    AP_MQTT_DIRECTION_BOTH

} ap_mqtt_direction_t;

typedef struct
{
    const char *topic;
    ap_signal_t signal;
	ap_mqtt_direction_t direction;

} ap_mqtt_mapping_t;


ap_result_t ap_mqtt_mapping_load(
    const char *filename
);


void ap_mqtt_mapping_free(void);


const ap_mqtt_mapping_t *
ap_mqtt_mapping_find_by_topic(
    const char *topic
);


const ap_mqtt_mapping_t *
ap_mqtt_mapping_find_by_signal(
    uint32_t signal_id
);


uint32_t ap_mqtt_mapping_count(void);


const ap_mqtt_mapping_t *
ap_mqtt_mapping_get(
    uint32_t index
);


#ifdef __cplusplus
}
#endif

#endif /* AP_MQTT_MAPPING_H */