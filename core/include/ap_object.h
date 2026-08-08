#ifndef AP_OBJECT_H
#define AP_OBJECT_H

#include <stdbool.h>
#include "ap_result.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t ap_object_id_t;

/**
 * @brief Object types within the Automation Platform.
 *
 * Objects are globally identifiable entities.
 * Only SIGNAL objects participate in the runtime data flow.
 */
typedef enum
{
    AP_OBJECT_SIGNAL,
    AP_OBJECT_NODE,
    AP_OBJECT_PLUGIN

} ap_object_type_t;

/**
 * @brief Value types supported by SIGNAL objects.
 *
 * NODE and PLUGIN objects do not carry runtime values.
 */
typedef enum
{
    AP_VALUE_NONE,
    AP_VALUE_BOOL,
    AP_VALUE_INT32,
    AP_VALUE_FLOAT,
    AP_VALUE_STRING

} ap_value_type_t;

/**
 * @brief Object definition.
 *
 * An object describes an identifiable entity within the system.
 * It does not contain a runtime value.
 */
typedef struct
{
    /** Globally unique object identifier */
    ap_object_id_t id;

    /** Object type */
    ap_object_type_t object_type;

    /** Runtime value type.
     *  AP_VALUE_NONE for NODE and PLUGIN objects.
     */
    ap_value_type_t value_type;

    /** Current object status */
    ap_result_t status;

} ap_object_t;

#ifdef __cplusplus
}
#endif

#endif /* AP_OBJECT_H */