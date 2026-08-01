#include "ap_plugin.h"
#include <stdio.h>


static ap_result_t test_init(void)
{
    return AP_OK;
}


static ap_result_t test_load(
    const ap_config_object_t *object)
{
    (void)object;

    printf("Test plugin load\n");

    return AP_OK;
}


static void test_shutdown(void)
{
}


static const ap_plugin_t ap_test_plugin =
{
    .type = AP_MODULE_TEST,
    .name = "test",

    .load = test_load,
    .init = test_init,
    .shutdown = test_shutdown
};


AP_PLUGIN_REGISTER(ap_test_plugin);