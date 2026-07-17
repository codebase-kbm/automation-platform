#include <stdio.h>

#include "ap_core.h"
#include "ap_dispatcher.h"
#include "ap_event.h"
#include "ap_mapper.h"
#include "ap_registry.h"

#include "logger.h"

int main(void)
{
    printf("=========================================\n");
    printf(" Automation Platform - Core Test\n");
    printf("=========================================\n\n");

    /* ---------------------------------------------------------- */
    /* Initialize                                                  */
    /* ---------------------------------------------------------- */

    ap_core_init();
    ap_logger_init();

    printf("Core initialized.\n\n");

    /* ---------------------------------------------------------- */
    /* Signal Definitions                                          */
    /* ---------------------------------------------------------- */

    static const ap_signal_t sig_bool =
    {
        .id   = 100,
        .type = AP_SIGNAL_BOOL
    };

    static const ap_signal_t sig_int =
    {
        .id   = 101,
        .type = AP_SIGNAL_INT32
    };

    static const ap_signal_t sig_float =
    {
        .id   = 102,
        .type = AP_SIGNAL_FLOAT
    };

    static const ap_signal_t sig_string =
    {
        .id   = 103,
        .type = AP_SIGNAL_STRING
    };

    /* BOOL targets */

    static const ap_signal_t bool_out1 = { .id = 200, .type = AP_SIGNAL_BOOL };
    static const ap_signal_t bool_out2 = { .id = 201, .type = AP_SIGNAL_BOOL };

    /* INT targets */

    static const ap_signal_t int_out1 = { .id = 210, .type = AP_SIGNAL_INT32 };
    static const ap_signal_t int_out2 = { .id = 211, .type = AP_SIGNAL_INT32 };

    /* FLOAT targets */

    static const ap_signal_t float_out1 = { .id = 220, .type = AP_SIGNAL_FLOAT };
    static const ap_signal_t float_out2 = { .id = 221, .type = AP_SIGNAL_FLOAT };
    static const ap_signal_t float_out3 = { .id = 222, .type = AP_SIGNAL_FLOAT };

    /* STRING targets */

    static const ap_signal_t string_out1 = { .id = 230, .type = AP_SIGNAL_STRING };
    static const ap_signal_t string_out2 = { .id = 231, .type = AP_SIGNAL_STRING };

    /* ---------------------------------------------------------- */
    /* Registry                                                    */
    /* ---------------------------------------------------------- */

    ap_registry_register(&sig_bool);
    ap_registry_register(&sig_int);
    ap_registry_register(&sig_float);
    ap_registry_register(&sig_string);

    ap_registry_register(&bool_out1);
    ap_registry_register(&bool_out2);

    ap_registry_register(&int_out1);
    ap_registry_register(&int_out2);

    ap_registry_register(&float_out1);
    ap_registry_register(&float_out2);
    ap_registry_register(&float_out3);

    ap_registry_register(&string_out1);
    ap_registry_register(&string_out2);

    printf("Signals registered.\n");

    /* ---------------------------------------------------------- */
    /* Mapping                                                     */
    /* ---------------------------------------------------------- */

    ap_mapper_add(100, 200);
    ap_mapper_add(100, 201);

    ap_mapper_add(101, 210);
    ap_mapper_add(101, 211);

    ap_mapper_add(102, 220);
    ap_mapper_add(102, 221);
    ap_mapper_add(102, 222);

    ap_mapper_add(103, 230);
    ap_mapper_add(103, 231);

    printf("Mappings registered.\n\n");

    /* ---------------------------------------------------------- */
    /* Helper Buffer                                               */
    /* ---------------------------------------------------------- */

    ap_event_t event;
    ap_event_t mapped[8];

    uint32_t count;

    /* ---------------------------------------------------------- */
    /* BOOL                                                        */
    /* ---------------------------------------------------------- */

    printf("=== BOOL TEST ===\n");

    ap_event_init(&event, &sig_bool, 1);
    event.value.b = true;

    count = ap_mapper_process(&event, mapped, 8);

    for (uint32_t i = 0; i < count; i++)
        ap_dispatcher_publish(&mapped[i]);

    /* ---------------------------------------------------------- */
    /* INT32                                                       */
    /* ---------------------------------------------------------- */

    printf("\n=== INT32 TEST ===\n");

    ap_event_init(&event, &sig_int, 1);
    event.value.i = 4711;

    count = ap_mapper_process(&event, mapped, 8);

    for (uint32_t i = 0; i < count; i++)
        ap_dispatcher_publish(&mapped[i]);

    /* ---------------------------------------------------------- */
    /* FLOAT                                                       */
    /* ---------------------------------------------------------- */

    printf("\n=== FLOAT TEST ===\n");

    ap_event_init(&event, &sig_float, 1);
    event.value.f = 21.5f;

    count = ap_mapper_process(&event, mapped, 8);

    for (uint32_t i = 0; i < count; i++)
        ap_dispatcher_publish(&mapped[i]);

    /* ---------------------------------------------------------- */
    /* STRING                                                      */
    /* ---------------------------------------------------------- */

    printf("\n=== STRING TEST ===\n");

    ap_event_init(&event, &sig_string, 1);
    event.value.s = "Automation Platform";

    count = ap_mapper_process(&event, mapped, 8);

    for (uint32_t i = 0; i < count; i++)
        ap_dispatcher_publish(&mapped[i]);

    printf("\nDone.\n");

    return 0;
}