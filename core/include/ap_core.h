#ifndef AP_CORE_H
#define AP_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Automation Platform Core.
 *
 * Initializes all core components.
 */
void ap_core_init(void);

/**
 * @brief Shutdown the Automation Platform Core.
 */
void ap_core_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* AP_CORE_H */