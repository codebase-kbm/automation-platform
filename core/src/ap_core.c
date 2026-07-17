#include "ap_core.h"
#include "ap_dispatcher.h"
#include "ap_registry.h"
#include "ap_timestamp.h"
#include "ap_mapper.h"

void ap_core_init(void)
{
    ap_dispatcher_init();
	ap_registry_init();
	ap_mapper_init();
	ap_timestamp_init();
}

void ap_core_shutdown(void)
{
    /* Reserved for future use */
}