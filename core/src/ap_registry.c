#include "ap_registry.h"

#define AP_MAX_SIGNALS 256

static const ap_signal_t *signals[AP_MAX_SIGNALS];
static uint32_t signal_count;

void ap_registry_init(void)
{
    signal_count = 0;
}

ap_result_t ap_registry_register(const ap_signal_t *signal)
{
    if (signal == NULL)
        return AP_ERROR_INVALID_ID;

    /* Duplicate ID? */
    if (ap_registry_find(signal->id) != NULL)
        return AP_ERROR_ALREADY_EXISTS;

    if (signal_count >= AP_MAX_SIGNALS)
        return AP_ERROR_FULL;

    signals[signal_count++] = signal;

    return AP_OK;
}

const ap_signal_t *ap_registry_find(uint32_t id)
{
    for (uint32_t i = 0; i < signal_count; i++)
    {
        if (signals[i]->id == id)
            return signals[i];
    }

    return NULL;
}