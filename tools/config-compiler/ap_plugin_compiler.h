#ifndef AP_PLUGIN_COMPILER_H
#define AP_PLUGIN_COMPILER_H

#include <stdint.h>
#include <libxml/tree.h>
#include "ap_plugin.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    uint8_t *data;
    uint32_t length;
    uint32_t capacity;

} ap_plugin_config_buffer_t;


typedef int (*ap_plugin_compile_fn)(
    xmlNodePtr node,
    ap_plugin_config_buffer_t *buffer
);


typedef struct
{
    ap_plugin_type_t type;

    const char *name;

    ap_plugin_compile_fn compile;

} ap_plugin_compiler_t;


#define AP_PLUGIN_COMPILER_REGISTER(plugin) \
    static const ap_plugin_compiler_t * const \
    __ap_plugin_compiler_##plugin \
    __attribute__((used, section("ap_plugin_compilers"))) = &(plugin)


#ifdef __cplusplus
}
#endif

#endif /* AP_PLUGIN_COMPILER_H */