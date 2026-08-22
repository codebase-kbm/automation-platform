#include <stdio.h>
#include "result_string.h"
#include "ap_result.h"

static const char *source_string(uint8_t source)
{
    switch (source)
    {
        case AP_RESULT_SOURCE_CORE:
            return "CORE";

        case AP_RESULT_SOURCE_PLUGIN:
            return "PLUGIN";

        default:
            return "UNKNOWN SOURCE";
    }
}

static const char *plugin_string(ap_plugin_type_t type)
{
    switch (type)
    {
#define X(name, string) \
        case name: return string;

#include "ap_plugin_types.def"

#undef X

        default:
            return "| UNKNOWN PLUGIN";
    }
}

static const char *component_string(ap_component_type_t component)
{
    switch (component)
    {
#define X(name, string) \
        case name: return string;

#include "ap_components.def"

#undef X

        default:
            return "UNKNOWN COMPONENT";
    }
}

static const char *error_string(ap_error_code_t error)
{
    switch (error)
    {
        #define X(name, string) \
                case name: return string;

        #include "ap_result_errors.def"

        #undef X

        default:
            return "UNKNOWN ERROR";
    }
}

const char *ap_result_string(ap_result_t result)
{
    static char buffer[256];

    snprintf(buffer, sizeof(buffer),
             "%s %s [%u] | %s | %s",
             source_string(AP_RESULT_SOURCE(result)),
             plugin_string(AP_RESULT_PLUGIN(result)),
             0,
             component_string(AP_RESULT_COMPONENT(result)),
             error_string(AP_RESULT_ERROR(result)));

    return buffer;
}