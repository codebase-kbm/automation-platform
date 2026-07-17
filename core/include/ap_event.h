#ifndef AP_EVENT_H
#define AP_EVENT_H

#include "ap_common.h"
#include "ap_signal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Event value type.
 *
 * The value type must match the signal definition.
 */
typedef union
{
    bool        b;
    int32_t     i;
    float       f;
    const char *s;

} ap_event_value_t;

/**
 * @brief Event flags.
 */
typedef enum
{
    AP_EVENT_NONE    = 0x00,
    AP_EVENT_TIMEOUT = 0x01,
    AP_EVENT_INVALID = 0x02

} ap_event_flags_t;

/**
 * @brief Runtime event.
 */
typedef struct
{
    /** Signal definition */
    const ap_signal_t *signal;

    /** Timestamp (µs since boot or epoch depending on platform) */
    uint64_t timestamp;

    /** Optional source node */
    uint32_t source;

    /** Event flags */
    uint32_t flags;

    /** Runtime value */
    ap_event_value_t value;

} ap_event_t;

void ap_event_init(ap_event_t *event,
                   const ap_signal_t *signal,
                   uint16_t source);
				   
#ifdef __cplusplus
}
#endif

#endif /* AP_EVENT_H */