#include <stdio.h>
#include "ap_core.h"
#include "ap_dispatcher.h"
#include "ap_event.h"
#include "ap_registry.h"
#include "ap_signal.h"

#include "logger.h"

int main(void)
{
    printf("=========================================\n");
    printf(" Automation Platform - Minimal Example\n");
    printf("=========================================\n\n");

    printf("Initializing Core...\n");
    ap_core_init();

    printf("Initializing Logger...\n");
    ap_logger_init();

    printf("\nRegistering signals...\n");

    static const ap_signal_t temperature =
    {
        .id = 100,
        .type = AP_SIGNAL_FLOAT
    };

    static const ap_signal_t humidity =
    {
        .id = 101,
        .type = AP_SIGNAL_INT32
    };

    static const ap_signal_t alarm =
    {
        .id = 102,
        .type = AP_SIGNAL_BOOL
    };

    static const ap_signal_t status =
    {
        .id = 103,
        .type = AP_SIGNAL_STRING
    };

    printf("Temperature : %s\n",
           ap_registry_register(&temperature) ? "OK" : "FAILED");

    printf("Humidity    : %s\n",
           ap_registry_register(&humidity) ? "OK" : "FAILED");

    printf("Alarm       : %s\n",
           ap_registry_register(&alarm) ? "OK" : "FAILED");

    printf("Status      : %s\n",
           ap_registry_register(&status) ? "OK" : "FAILED");

    printf("\nDuplicate registration test...\n");

    if (!ap_registry_register(&temperature))
        printf("Duplicate detection: OK\n");
    else
        printf("Duplicate detection: FAILED\n");

    printf("\nUnknown signal test...\n");

    if (ap_registry_find(999) == NULL)
        printf("Unknown signal lookup: OK\n");
    else
        printf("Unknown signal lookup: FAILED\n");

    printf("\nPublishing events...\n\n");

    ap_event_t event;

    event.timestamp = 123456789;
    event.source = 1;
    event.flags = AP_EVENT_NONE;

    event.signal = &temperature;
    event.value.f = 21.5f;
    ap_dispatcher_publish(&event);

    event.timestamp++;
    event.signal = &humidity;
    event.value.i = 56;
    ap_dispatcher_publish(&event);

    event.timestamp++;
    event.signal = &alarm;
    event.value.b = true;
    ap_dispatcher_publish(&event);

    event.timestamp++;
    event.signal = &status;
    event.value.s = "Running";
    ap_dispatcher_publish(&event);

    printf("\nDone.\n");

    return 0;
}