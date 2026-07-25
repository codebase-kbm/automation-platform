#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "ap_core.h"
#include "ap_dispatcher.h"
#include "ap_event.h"
#include "ap_mapper.h"
#include "ap_registry.h"

#include "logger.h"
#include "mqtt.h"


int main(void)
{
    printf("=========================================\n");
    printf(" Automation Platform - Core Test\n");
    printf("=========================================\n\n");

    /*
     * Core initialization
     */
    if (ap_core_init() != AP_OK)
    {
        printf("Core initialization failed.\n");
        return 1;
    }
	ap_logger_init();

    printf("Core initialized.\n\n");


ap_mqtt_config_t mqtt_config =
{
    .host = "127.0.0.1",
    .port = 1883,
    .username = "automation",
    .password = "password"
};

ap_result_t mqtt_result =
    ap_mqtt_init(&mqtt_config);

if (mqtt_result != AP_OK)
{
    printf(
        "MQTT initialization failed: %s\n",
        ap_result_string(mqtt_result)
    );

    return 1;
}


printf("MQTT connected.\n");

ap_mqtt_subscribe(
    "automation/signal/#"
);

    /*
     * Signals
     */

	ap_event_t event;
	
    ap_signal_t temperature =
    {
        .id = 100,
        .type = AP_SIGNAL_FLOAT
    };

    ap_signal_t temperature_display =
    {
        .id = 200,
        .type = AP_SIGNAL_FLOAT
    };

    ap_signal_t temperature_log =
    {
        .id = 201,
        .type = AP_SIGNAL_FLOAT
    };

    if (ap_registry_register(&temperature) != AP_OK ||
        ap_registry_register(&temperature_display) != AP_OK ||
        ap_registry_register(&temperature_log) != AP_OK)
    {
        printf("Signal registration failed.\n");
        return 1;
    }

    printf("Signals registered.\n");

    /*
     * Mappings
     */

    if (ap_mapper_add(100, 200) != AP_OK ||
        ap_mapper_add(100, 201) != AP_OK)
    {
        printf("Mapping registration failed.\n");
        return 1;
    }

    printf("Mappings registered.\n\n");

printf("Publishing event...\n");

ap_event_init(&event,
              &temperature,
              100);

event.value.f = 21.5f;
ap_dispatcher_publish(&event);

struct timespec delay =
{
    .tv_sec = 0,
    .tv_nsec = 100000000
};

nanosleep(&delay, NULL);

const ap_signal_t *signal =
    ap_registry_find(100);

if (signal == NULL)
{
    printf("Signal not found\n");
    return 0;
}

ap_event_init(
    &event,
    signal,
    0
);

event.value.f = 21.8f;

ap_mqtt_publish_event(
    &event
);

while (1)
{
    ap_result_t result =
        ap_core_process();

    if (result != AP_OK)
    {
        printf(
            "Core process failed: %s\n",
            ap_result_string(result)
        );

        break;
    }
	
	ap_mqtt_process();

    struct timespec delay =
    {
        .tv_sec = 0,
        .tv_nsec = 100000000
    };

    nanosleep(&delay, NULL);
}

    /*
     * Shutdown
     */

    ap_core_shutdown();

    printf("\nDone.\n");

    return 0;
}