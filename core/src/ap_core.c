#include "ap_core.h"
#include "ap_dispatcher.h"
#include "ap_registry.h"
#include "ap_timestamp.h"


#include <stddef.h>

static bool g_core_initialized = false;
static uint64_t g_core_start_time = 0;

ap_result_t ap_core_init(void)
{
    if (g_core_initialized)
        return AP_ERROR_ALREADY_EXISTS;

    ap_timestamp_init();
    ap_dispatcher_init();
	ap_registry_init();

    g_core_start_time = ap_timestamp_now();

    g_core_initialized = true;

    return AP_OK;
}

ap_result_t ap_core_process(void)
{
    if (!g_core_initialized)
        return AP_ERROR_INVALID_ARGUMENT;

    /*
     * Currently the dispatcher operates synchronously.
     *
     * Future:
     * - queued events
     * - scheduled processing
     * - system signals
     */

    return AP_OK;
}

void ap_core_shutdown(void)
{
    if (!g_core_initialized)
        return;

    g_core_initialized = false;
}