#ifndef AP_MQTT_H
#define AP_MQTT_H

#include <stdint.h>

#include "ap_event.h"
#include "ap_result.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    const char *host;
    uint16_t port;

    const char *username;
    const char *password;

} ap_mqtt_config_t;


ap_result_t ap_mqtt_init(
    const ap_mqtt_config_t *config
);


ap_result_t ap_mqtt_subscribe(
    const char *topic
);


ap_result_t ap_mqtt_publish_event(
    const ap_event_t *event
);


ap_result_t ap_mqtt_process(void);


void ap_mqtt_shutdown(void);


#ifdef __cplusplus
}
#endif

#endif /* AP_MQTT_H */