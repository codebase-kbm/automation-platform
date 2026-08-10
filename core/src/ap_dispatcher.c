#include "ap_dispatcher.h"
#include <stddef.h>

#define AP_MAX_EVENT_HANDLERS 32

static ap_event_handler_t handlers[AP_MAX_EVENT_HANDLERS];
static uint16_t handler_count = 0;
static uint16_t event_count;

void ap_dispatcher_init(void)
{
    handler_count = 0;
}

ap_result_t ap_dispatcher_register(ap_event_handler_t handler)
{
    if (handler == NULL)
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_CORE,AP_COMPONENT_DISPATCHER,AP_PLUGIN_NONE,AP_ERROR_INVALID_ARGUMENT);

    if (handler_count >= AP_MAX_EVENT_HANDLERS)
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_CORE,AP_COMPONENT_DISPATCHER,AP_PLUGIN_NONE,AP_ERROR_FULL);

    handlers[handler_count++] = handler;

    return AP_OK;
}

ap_result_t ap_dispatcher_publish(const ap_event_t *event)
{
    if (event == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_CORE,
            AP_COMPONENT_DISPATCHER,
            AP_PLUGIN_NONE,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    ap_result_t result = AP_OK;

    for (uint16_t i = 0; i < handler_count; i++)
    {
        ap_result_t handler_result = handlers[i](event);

        if (handler_result != AP_OK && result == AP_OK)
        {
            result = handler_result;
        }
    }

    event_count++;

    return result;
}

uint16_t ap_dispatcher_get_EventCount(void)
{
    return event_count;
}

uint32_t ap_dispatcher_get_HandlerCount(void)
{
    return handler_count;
}