#ifndef AP_REGISTRY_H
#define AP_REGISTRY_H

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
 * @brief Get an object by its ID.
 *
 * @param id Object identifier.
 * @return Pointer to the registered object, or NULL if not found.
 */
const ap_object_t *ap_registry_get(ap_object_id_t id);

/**
 * @brief Get an object by its ID or create and register it.
 *
 * If the object already exists, its value type must match.
 * If it does not exist, a new object is created and registered.
 *
 * @param id Object identifier.
 * @param value_type Required object value type.
 * @param object Receives a pointer to the registered object.
 *
 * @return AP_OK on success or an appropriate error code.
 */
ap_result_t ap_registry_get_or_create(ap_object_id_t id,ap_value_type_t value_type,const ap_object_t **object);

/**
 * @brief Sets an object Status by the given Error_Code.
 *
 * @param id Object identifier.
 * @param code Status Code.
 *
 * @return AP_OK on success or an appropriate error code.
 */
ap_result_t ap_registry_set_objectstatus(ap_object_id_t id,ap_error_code_t code);

/**
 * @brief Sets the runtime context of an object.
 *
 * @param id      Object identifier.
 * @param context Pointer to array of context data.
 * @param size    Number of context bytes.
 *
 * @return AP_OK on success or an appropriate error code.
 */
ap_result_t ap_registry_set_objectstatuscontext(ap_object_id_t id, const uint8_t *context, uint8_t size);

/**
 * @brief Get an object Status Error_Code.
 *
 * @param id Object identifier.
 * @param status Pointer to receive the Status Code of the Object.
 *
 * @return AP_OK on success or an appropriate error code.
 */
ap_result_t ap_registry_get_objectstatus(ap_object_id_t id,ap_error_code_t *code);

/**
 * @brief Gets the runtime context of an object.
 *
 * @param id      Object identifier.
 * @param context Pointer to receive context data.
 * @param size    Number of context bytes available.
 *
 * @return AP_OK on success or an appropriate error code.
 */
ap_result_t ap_registry_get_objectstatuscontext(ap_object_id_t id, uint8_t *context, uint8_t size);

#ifdef __cplusplus
}
#endif

#endif /* AP_REGISTRY_H */