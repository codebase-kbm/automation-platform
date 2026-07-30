#ifndef AP_REGISTRY_H
#define AP_REGISTRY_H

#include "ap_common.h"
#include "ap_object.h"
#include "ap_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the object registry.
 */
void ap_registry_init(void);

/**
 * @brief Register an object.
 *
 * @param object Object to register.
 * @return AP_OK on success or an appropriate error code.
 */
ap_result_t ap_registry_register(const ap_object_t *object);

/**
 * @brief Find an object by its ID.
 *
 * @param id Object identifier.
 * @return Pointer to the registered object, or NULL if not found.
 */
const ap_object_t *ap_registry_find(ap_object_id_t id);

#ifdef __cplusplus
}
#endif

#endif /* AP_REGISTRY_H */