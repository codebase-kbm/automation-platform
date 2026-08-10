#include <stddef.h>
#include "ap_result.h"

#ifndef AP_MAX_RESULT_HANDLERS
#define AP_MAX_RESULT_HANDLERS 8
#endif

typedef struct {
    ap_result_handler_t handler;
    void *context;
} ap_result_handler_entry_t;

static ap_result_handler_entry_t handlers[AP_MAX_RESULT_HANDLERS];
static uint8_t handler_count = 0;


ap_result_t ap_result_register_handler(
    ap_result_handler_t handler,
    void *context)
{
    if (handler == NULL) {
        return AP_ERROR_INVALID_ARGUMENT;
    }

    if (handler_count >= AP_MAX_RESULT_HANDLERS) {
        return AP_ERROR_OUT_OF_MEMORY;
    }

    handlers[handler_count].handler = handler;
    handlers[handler_count].context = context;

    handler_count++;

    return AP_OK;
}


ap_result_t ap_result_report(ap_result_t result, const ap_event_t *event)
{
    if (result != AP_OK)
    {
        for (uint8_t i = 0; i < handler_count; i++) {
            if (handlers[i].handler != NULL) {
                handlers[i].handler(
                    result,
                    event,
                    handlers[i].context
                );
            }
        }
    }
    return result;
}