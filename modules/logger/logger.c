#include "logger.h"

#include <stdio.h>
#include <inttypes.h>

#include "ap_dispatcher.h"

static void ap_logger_event_handler(const ap_event_t *event)
{
    if (event == NULL || event->object == NULL)
        return;

    printf("[%010" PRIu64 "] ", event->timestamp);

    printf("Object %" PRIu32 " = ",
           event->object->id);

    switch (event->object->value_type)
    {
        case AP_VALUE_BOOL:
            printf("%s",
                   event->value.b ? "true" : "false");
            break;

        case AP_VALUE_INT32:
            printf("%" PRId32,
                   event->value.i);
            break;

        case AP_VALUE_FLOAT:
            printf("%f",
                   event->value.f);
            break;

        case AP_VALUE_STRING:
            printf("%s",
                   event->value.s ? event->value.s : "(null)");
            break;

        case AP_VALUE_NONE:
        default:
            printf("<no value>");
            break;
    }

    printf("\n");
}

void ap_logger_init(void)
{
    ap_dispatcher_register(ap_logger_event_handler);
}