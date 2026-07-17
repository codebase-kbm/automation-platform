#include "ap_event.h"

#include "ap_timestamp.h"

void ap_event_init(ap_event_t *event,
                   const ap_signal_t *signal,
                   uint16_t source)
{
    if ((event == NULL) || (signal == NULL))
        return;

    event->signal = signal;
    event->timestamp = ap_timestamp_now();
    event->source = source;
    event->flags = AP_EVENT_NONE;
}