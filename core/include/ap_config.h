#ifndef AP_CONFIG_H
#define AP_CONFIG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AP_CONFIG_MAGIC 0x41504346u /* "APCF" */

#define AP_CONFIG_VERSION 1

/* Core limits */
#ifndef AP_MAX_VARIABLES
#define AP_MAX_VARIABLES 256
#endif

#ifndef AP_MAX_EVENT_HANDLERS
#define AP_MAX_EVENT_HANDLERS 16
#endif

#ifndef AP_MAX_MAPPINGS
#define AP_MAX_MAPPINGS 256
#endif

typedef enum
{
    AP_CONFIG_SECTION_CORE = 1,
    AP_CONFIG_SECTION_CAN  = 2,
    AP_CONFIG_SECTION_MQTT = 3

} ap_config_section_type_t;


/**
 * @brief Configuration file header.
 */
typedef struct
{
    uint32_t magic;
    uint16_t version;
    uint16_t section_count;

    uint32_t total_size;

} ap_config_header_t;


/**
 * @brief Generic configuration section header.
 */
typedef struct
{
    uint16_t type;
    uint16_t version;

    uint32_t size;

} ap_config_section_header_t;


#ifdef __cplusplus
}
#endif

#endif /* AP_CONFIG_H */