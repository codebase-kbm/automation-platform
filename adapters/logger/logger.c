#include "logger.h"

#include <stdio.h>
#include <inttypes.h>

#include "ap_dispatcher.h"

static void ap_logger_event_handler(const ap_event_t *event)
{
    if (event == NULL || event->signal == NULL)
        return;

    printf("[%010" PRIu64 "] ", event->timestamp);

    printf("Signal %u = ",
           event->signal->id);

    switch (event->signal->type)
    {
        case AP_SIGNAL_BOOL:
            printf("%s",
                   event->value.b ? "true" : "false");
            break;

        case AP_SIGNAL_INT32:
            printf("%" PRId32,
                   event->value.i);
            break;

        case AP_SIGNAL_FLOAT:
            printf("%f",
                   event->value.f);
            break;

        case AP_SIGNAL_STRING:
            printf("%s",
                   event->value.s ? event->value.s : "(null)");
            break;

        default:
            printf("<unknown>");
            break;
    }

    printf("\n");
}

void ap_logger_init(void)
{
    ap_dispatcher_register(ap_logger_event_handler);
}