#ifndef AP_RESULT_H
#define AP_RESULT_H

#include <stdint.h>
#include "ap_plugin_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Result layout
 *
 * 31                16 15       10 9        8 7        0
 * +-------------------+-----------+----------+----------+
 * |    Plugin Type    | Component |  Source  |   Error  |
 * |      16 bit       |   6 bit   |   2 bit  |   8 bit  |
 * +-------------------+-----------+----------+----------+
 */

typedef uint32_t ap_result_t;

/* Success */
#define AP_OK ((ap_result_t)0)

/* Result source */
#define AP_RESULT_SOURCE_CORE    0x0u
#define AP_RESULT_SOURCE_PLUGIN  0x1u
#define AP_RESULT_SOURCE_ADAPTER 0x2u

/* Bit positions */
#define AP_RESULT_ERROR_SHIFT     0u
#define AP_RESULT_SOURCE_SHIFT    8u
#define AP_RESULT_COMPONENT_SHIFT 10u
#define AP_RESULT_PLUGIN_SHIFT    16u

/* Bit masks */
#define AP_RESULT_ERROR_MASK      0x000000FFu
#define AP_RESULT_SOURCE_MASK     0x00000300u
#define AP_RESULT_COMPONENT_MASK  0x0000FC00u
#define AP_RESULT_PLUGIN_MASK     0xFFFF0000u

/* Build result */
#define AP_RESULT_MAKE(source, component, plugin, error) \
    ( ((ap_result_t)(error)     & 0x00FFu) | \
      (((ap_result_t)(source)   & 0x0003u) << AP_RESULT_SOURCE_SHIFT) | \
      (((ap_result_t)(component) & 0x003Fu) << AP_RESULT_COMPONENT_SHIFT) | \
      (((ap_result_t)(plugin)   & 0xFFFFu) << AP_RESULT_PLUGIN_SHIFT) )

/* Extract result fields */
#define AP_RESULT_ERROR(result) \
    ((uint8_t)(((result) & AP_RESULT_ERROR_MASK) >> AP_RESULT_ERROR_SHIFT))

#define AP_RESULT_SOURCE(result) \
    ((uint8_t)(((result) & AP_RESULT_SOURCE_MASK) >> AP_RESULT_SOURCE_SHIFT))

#define AP_RESULT_COMPONENT(result) \
    ((uint8_t)(((result) & AP_RESULT_COMPONENT_MASK) >> AP_RESULT_COMPONENT_SHIFT))

#define AP_RESULT_PLUGIN(result) \
    ((uint16_t)(((result) & AP_RESULT_PLUGIN_MASK) >> AP_RESULT_PLUGIN_SHIFT))


/* Error codes */
typedef enum
{
    AP_ERROR_NONE = 0,

#define X(name, string) name,
#include "../defs/ap_result_errors.def"
#undef X

    AP_ERROR_COUNT

} ap_error_code_t;

typedef enum
{
    AP_COMPONENT_NONE = 0,

#define X(name, string) name,
#include "../defs/ap_components.def"
#undef X

    AP_COMPONENT_COUNT

} ap_component_type_t;

// Core Result-API Handler
typedef struct ap_event ap_event_t;
typedef void (*ap_result_handler_t)(ap_result_t result,const ap_event_t *event,void *context);
ap_result_t ap_result_register_handler(ap_result_handler_t handler,void *context);
ap_result_t ap_result_report(ap_result_t result, const ap_event_t *event);

#ifdef __cplusplus
}
#endif

#endif /* AP_RESULT_H */