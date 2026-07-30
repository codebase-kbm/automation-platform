#include "ap_event.h"
#include "ap_timestamp.h"

void ap_event_init(ap_event_t *event,
                   const ap_object_t *object,
                   ap_object_id_t source)
{
    if ((event == NULL) || (object == NULL))
        return;

    event->object = object;
    event->timestamp = ap_timestamp_now();
    event->source = source;
    event->flags = AP_EVENT_NONE;
}
