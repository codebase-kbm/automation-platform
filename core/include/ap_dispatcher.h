#ifndef AP_DISPATCHER_H
#define AP_DISPATCHER_H

#include "ap_common.h"
#include "ap_event.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Event handler callback.
 *
 * Every adapter registers one or more callbacks to receive events.
 */
typedef void (*ap_event_handler_t)(const ap_event_t *event);

/**
 * @brief Initialize the dispatcher.
 */
void ap_dispatcher_init(void);

/**
 * @brief Register an event handler.
 *
 * @param handler Event callback.
 * @return true on success.
 */
bool ap_dispatcher_register(ap_event_handler_t handler);

/**
 * @brief Publish an event.
 *
 * The dispatcher forwards the event to all registered handlers.
 *
 * @param event Event to publish.
 */
void ap_dispatcher_publish(const ap_event_t *event);

#ifdef __cplusplus
}
#endif

#endif /* AP_DISPATCHER_H */