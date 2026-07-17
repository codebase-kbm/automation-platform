#include "ap_dispatcher.h"
#include <stdio.h>

#include "ap_core.h"
#include "ap_event.h"
#include "ap_mapper.h"
#include "ap_registry.h"

#include "logger.h"

int main(void)
{
    printf("=========================================\n");
    printf(" Automation Platform - Minimal Example\n");
    printf("=========================================\n\n");

    /* ---------------------------------------------------------- */
    /* Initialize Core                                             */
    /* ---------------------------------------------------------- */

    ap_core_init();
    ap_logger_init();

    printf("Core initialized.\n\n");

    /* ---------------------------------------------------------- */
    /* Define Signals                                              */
    /* ---------------------------------------------------------- */

static const ap_signal_t source_temperature =
{
    .id = 100,
    .type = AP_SIGNAL_FLOAT
};

static const ap_signal_t target_temperature_1 =
{
    .id = 200,
    .type = AP_SIGNAL_FLOAT
};

static const ap_signal_t target_temperature_2 =
{
    .id = 201,
    .type = AP_SIGNAL_FLOAT
};

static const ap_signal_t target_temperature_3 =
{
    .id = 202,
    .type = AP_SIGNAL_FLOAT
};

    /* ---------------------------------------------------------- */
    /* Register Signals                                            */
    /* ---------------------------------------------------------- */

    ap_registry_register(&source_temperature);
    ap_registry_register(&target_temperature_1);
    ap_registry_register(&target_temperature_2);
    ap_registry_register(&target_temperature_3);

    printf("Signals registered.\n");

    /* ---------------------------------------------------------- */
    /* Configure Mapping                                            */
    /* ---------------------------------------------------------- */

    ap_mapper_add(100, 200);
    ap_mapper_add(100, 201);
    ap_mapper_add(100, 202);

    printf("Mappings registered.\n\n");

    /* ---------------------------------------------------------- */
    /* Create Event                                                */
    /* ---------------------------------------------------------- */

    ap_event_t event;

    ap_event_init(
        &event,
        &source_temperature,
        1);

    event.value.f = 21.5f;

    /* ---------------------------------------------------------- */
    /* Process Mapping                                             */
    /* ---------------------------------------------------------- */

    ap_event_t mapped[8];

    uint32_t count =
        ap_mapper_process(
            &event,
            mapped,
            8);

    printf("Mapped Events: %u\n\n", count);

    /* ---------------------------------------------------------- */
    /* Publish                                                     */
    /* ---------------------------------------------------------- */

    for (uint32_t i = 0; i < count; i++)
    {
        ap_dispatcher_publish(&mapped[i]);
    }

    printf("\nDone.\n");

    return 0;
}