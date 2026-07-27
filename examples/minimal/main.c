#include <stdio.h>
#include <time.h>

#include "ap_core.h"
#include "ap_dispatcher.h"
#include "ap_event.h"
#include "ap_mapper.h"
#include "ap_registry.h"

#include "logger.h"
#include "mqtt.h"
#include "mqtt_config.h"
#include "mqtt_mapping.h"


int main(void)
{
    printf("=========================================\n");
    printf(" Automation Platform - Core Test\n");
    printf("=========================================\n\n");


    if (ap_core_init() != AP_OK)
    {
        printf("Core initialization failed.\n");
        return 1;
    }


    ap_logger_init();


    printf("Core initialized.\n\n");


    /*
     * MQTT configuration
     */

    ap_mqtt_config_t mqtt_config = {0};


    ap_result_t result =
        ap_mqtt_config_load(
            "adapters/mqtt/mqtt.json",
            &mqtt_config
        );


    if (result != AP_OK)
    {
        printf(
            "MQTT configuration failed: %s\n",
            ap_result_string(result)
        );

        return 1;
    }


    /*
     * MQTT mappings
     *
     * Signals are registered here if they
     * do not already exist in the Core.
     */

    result =
        ap_mqtt_mapping_load(
            "adapters/mqtt/mqtt.json"
        );


    if (result != AP_OK)
    {
        printf(
            "MQTT mapping configuration failed: %s\n",
            ap_result_string(result)
        );

        ap_mqtt_config_free(
            &mqtt_config
        );

        return 1;
    }


    /*
     * MQTT initialization
     */

    result =
        ap_mqtt_init(
            &mqtt_config
        );


    if (result != AP_OK)
    {
        printf(
            "MQTT initialization failed: %s\n",
            ap_result_string(result)
        );

        ap_mqtt_config_free(
            &mqtt_config
        );

        return 1;
    }


    ap_mqtt_config_free(
        &mqtt_config
    );


    printf("MQTT connected.\n");


    /*
     * Core mappings
     */

    if (ap_mapper_add(100, 200) != AP_OK ||
        ap_mapper_add(100, 201) != AP_OK)
    {
        printf(
            "Mapping registration failed.\n"
        );

        return 1;
    }


    printf(
        "Mappings registered.\n\n"
    );


    /*
     * Test event
     */

    const ap_signal_t *signal =
        ap_registry_find(
            100
        );


    if (signal == NULL)
    {
        printf(
            "Signal not found\n"
        );

        return 1;
    }


    ap_event_t event;


    ap_event_init(
        &event,
        signal,
        100
    );


    event.value.f =
        21.8f;


    printf(
        "Publishing event...\n"
    );


    ap_dispatcher_publish(
        &event
    );


    while (1)
    {
        result =
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


        nanosleep(
            &delay,
            NULL
        );
    }


    ap_mqtt_shutdown();


    ap_mqtt_mapping_free();


    ap_core_shutdown();


    printf(
        "\nDone.\n"
    );


    return 0;
}