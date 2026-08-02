#include "can.h"

#include "ap_plugin_compiler.h"

#include <stdio.h>


static int ap_can_config_compile(
    xmlNodePtr module,
    ap_plugin_config_buffer_t *buffer)
{
    if (module == NULL ||
        buffer == NULL ||
        buffer->data == NULL)
    {
        return -1;
    }

    printf("CAN config compiler called\n");

    return 0;
}


static const ap_plugin_compiler_t ap_can_config_plugin =
{
    .type = AP_MODULE_CAN,
    .name = "can",
    .compile = ap_can_config_compile
};


AP_PLUGIN_COMPILER_REGISTER(
    ap_can_config_plugin
);