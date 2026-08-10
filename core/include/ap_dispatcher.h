#ifndef AP_DISPATCHER_H
#define AP_DISPATCHER_H


#include "ap_event.h"
#include "ap_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Event handler callback.
 *
 * Every adapter registers one or more callbacks to receive events.
 */
typedef ap_result_t (*ap_event_handler_t)(const ap_event_t *event);

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
ap_result_t ap_dispatcher_register(ap_event_handler_t handler);

/**
 * @brief Publish an event.
 *
 * The dispatcher forwards the event to all registered handlers.
 *
 * @param event Event to publish.
 */
ap_result_t ap_dispatcher_publish(const ap_event_t *event);
uint16_t ap_dispatcher_get_EventCount(void);
uint32_t ap_dispatcher_get_HandlerCount(void);

#ifdef __cplusplus
}
#endif

#endif /* AP_DISPATCHER_H */