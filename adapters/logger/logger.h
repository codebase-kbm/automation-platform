#ifndef AP_LOGGER_H
#define AP_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the logger adapter.
 *
 * Registers the logger at the dispatcher.
 */
void ap_logger_init(void);

#ifdef __cplusplus
}
#endif

#endif /* AP_LOGGER_H */