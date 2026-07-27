#ifndef AP_MQTT_CONFIG_H
#define AP_MQTT_CONFIG_H

#include "mqtt.h"

#ifdef __cplusplus
extern "C" {
#endif

ap_result_t ap_mqtt_config_load(
    const char *filename,
    ap_mqtt_config_t *config
);

void ap_mqtt_config_free(
    ap_mqtt_config_t *config
);

#ifdef __cplusplus
}
#endif

#endif /* AP_MQTT_CONFIG_H */