#ifndef AP_SIGNAL_H
#define AP_SIGNAL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Supported signal data types.
 */
typedef enum
{
    AP_SIGNAL_BOOL,
    AP_SIGNAL_INT32,
    AP_SIGNAL_FLOAT,
    AP_SIGNAL_STRING

} ap_signal_type_t;

/**
 * @brief Signal definition.
 *
 * A signal describes a logical data point within the system.
 * It does not contain a runtime value.
 */
typedef struct
{
    /** Unique signal identifier */
    uint32_t id;

    /** Signal data type */
    ap_signal_type_t type;

} ap_signal_t;

#ifdef __cplusplus
}
#endif

#endif /* AP_SIGNAL_H */