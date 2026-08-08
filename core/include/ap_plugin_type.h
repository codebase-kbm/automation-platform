#ifndef AP_PLUGIN_TYPE_H
#define AP_PLUGIN_TYPE_H

#include <stdint.h>

typedef enum
{
    AP_PLUGIN_NONE = 0,

    #define X(name, string) name,
    #include "../defs/ap_plugin_types.def"
    #undef X

    AP_PLUGIN_COUNT

} ap_plugin_type_t;

#endif /* AP_PLUGIN_TYPE_H */