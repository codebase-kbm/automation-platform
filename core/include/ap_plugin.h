#ifndef AP_PLUGIN_H
#define AP_PLUGIN_H

#include <stddef.h>

#include "ap_result.h"
#include "ap_config_reader.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    AP_PLUGIN_NONE = 0,

    #define X(name, string) name,
    #include "../defs/plugin_types.def"
    #undef X

    AP_PLUGIN_COUNT

} ap_plugin_type_t;

const char *ap_plugin_type_name(ap_plugin_type_t type);

ap_plugin_type_t ap_plugin_type_from_name(const char *name);

typedef enum
{
    AP_PLUGIN_DEPENDENCY_OPTIONAL = 0,
    AP_PLUGIN_DEPENDENCY_REQUIRED

} ap_plugin_dependency_type_t;

typedef struct
{
    ap_plugin_type_t type;
    ap_plugin_dependency_type_t dependency_type;

} ap_plugin_dependency_t;

typedef struct ap_plugin
{
    ap_plugin_type_t type;

    const char *name;

    const ap_plugin_dependency_t *dependencies;
    size_t dependency_count;

    ap_result_t (*load)(const ap_config_object_t *object);
    ap_result_t (*init)(void);
    ap_result_t (*process)(void);
    void (*shutdown)(void);

} ap_plugin_t;


#define AP_PLUGIN_REGISTER(plugin) \
    static const ap_plugin_t * const \
    __ap_plugin_##plugin \
    __attribute__((used, section("ap_plugins"))) = &(plugin)


#ifdef __cplusplus
}
#endif

#endif