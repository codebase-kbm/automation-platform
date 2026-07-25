#include "ap_dispatcher.h"
#include <stddef.h>

#define AP_MAX_EVENT_HANDLERS 16

static ap_event_handler_t handlers[AP_MAX_EVENT_HANDLERS];
static uint32_t handler_count = 0;

void ap_dispatcher_init(void)
{
    handler_count = 0;
}

ap_result_t ap_dispatcher_register(ap_event_handler_t handler)
{
    if (handler == NULL)
        return AP_ERROR_INVALID_ARGUMENT;

    if (handler_count >= AP_MAX_EVENT_HANDLERS)
        return AP_ERROR_FULL;

    handlers[handler_count++] = handler;

    return AP_OK;
}

void ap_dispatcher_publish(const ap_event_t *event)
{
    if (event == NULL)
        return;

    for (uint32_t i = 0; i < handler_count; i++)
    {
        handlers[i](event);
    }
}
