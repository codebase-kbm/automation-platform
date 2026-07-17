#include <stdint.h>

#include "ap_core.h"
#include "ap_dispatcher.h"
#include "ap_signal.h"
#include "ap_event.h"

#include "logger.h"

int main(void)
{
    ap_core_init();

    ap_logger_init();

    static const ap_signal_t temperature =
    {
        .id   = 100,
        .type = AP_SIGNAL_FLOAT
    };

    ap_event_t event =
    {
        .signal    = &temperature,
        .timestamp = 123456789,
        .source    = 1,
        .flags     = AP_EVENT_NONE,
        .value.f   = 21.5f
    };

    ap_dispatcher_publish(&event);

    return 0;
}