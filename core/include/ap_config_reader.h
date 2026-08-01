#ifndef AP_CONFIG_READER_H
#define AP_CONFIG_READER_H

#include <stdint.h>

#include "ap_config.h"
#include "ap_result.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    ap_config_object_header_t header;
    const uint8_t *payload;

} ap_config_object_t;


/**
 * @brief Open and validate a binary configuration.
 */
ap_result_t ap_config_reader_open(
    const char *filename
);


/**
 * @brief Read the next object from the configuration.
 *
 * @param object Object information and payload.
 * @return AP_OK when an object was read.
 *         AP_ERROR_NOT_FOUND when the end was reached.
 */
ap_result_t ap_config_reader_next(
    ap_config_object_t *object
);


/**
 * @brief Close the configuration reader.
 */
void ap_config_reader_close(void);


#ifdef __cplusplus
}
#endif

#endif