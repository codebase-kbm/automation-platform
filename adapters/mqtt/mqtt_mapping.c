#include "mqtt_mapping.h"

#include <stddef.h>
#include <string.h>


static const ap_mqtt_mapping_t mappings[] =
{
    {
        .topic = "automation/temperature",
        .signal_id = 100
    }
};


#define AP_MQTT_MAPPING_COUNT \
    (sizeof(mappings) / sizeof(mappings[0]))


const ap_mqtt_mapping_t *
ap_mqtt_mapping_find_by_topic(
    const char *topic
)
{
    if (topic == NULL)
        return NULL;


    for (uint32_t i = 0;
         i < AP_MQTT_MAPPING_COUNT;
         i++)
    {
        if (strcmp(
                mappings[i].topic,
                topic
            ) == 0)
        {
            return &mappings[i];
        }
    }


    return NULL;
}


const ap_mqtt_mapping_t *
ap_mqtt_mapping_find_by_signal(
    uint32_t signal_id
)
{
    for (uint32_t i = 0;
         i < AP_MQTT_MAPPING_COUNT;
         i++)
    {
        if (mappings[i].signal_id == signal_id)
            return &mappings[i];
    }


    return NULL;
}


uint32_t ap_mqtt_mapping_count(void)
{
    return AP_MQTT_MAPPING_COUNT;
}


const ap_mqtt_mapping_t *
ap_mqtt_mapping_get(
    uint32_t index
)
{
    if (index >= AP_MQTT_MAPPING_COUNT)
        return NULL;


    return &mappings[index];
}