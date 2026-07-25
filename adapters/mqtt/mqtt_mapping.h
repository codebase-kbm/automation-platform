#ifndef AP_MQTT_MAPPING_H
#define AP_MQTT_MAPPING_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    const char *topic;
    uint32_t signal_id;

} ap_mqtt_mapping_t;


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