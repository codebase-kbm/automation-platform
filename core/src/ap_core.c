#include "ap_core.h"

#include "ap_dispatcher.h"

void ap_core_init(void)
{
    ap_dispatcher_init();
}

void ap_core_shutdown(void)
{
    /* Reserved for future use */
}