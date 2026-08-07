#ifndef AP_CONFIG_H
#define AP_CONFIG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Binary configuration format
 *
 * All multi-byte values are little-endian.
 *
 * File:
 *     [file header]
 *     [object header + payload]
 *     [object header + payload]
 *     ...
 */

/* "AP" */
#define AP_CONFIG_MAGIC       			0x4150u
#define AP_CONFIG_VERSION     			1u
#define AP_CONFIG_HEADER_SIZE 			3u
#define AP_CONFIG_OBJECT_HEADER_SIZE 	9u

/*
 * Configuration object types
 */

#define AP_CONFIG_OBJECT_NODE      1u
#define AP_CONFIG_OBJECT_PLUGIN    2u

/**
 * @brief Binary configuration file header.
 *
 * Size: 3 bytes
 */
typedef struct
{
    uint16_t magic;
    uint8_t  version;

} ap_config_header_t;


/**
 * @brief Generic object header.
 *
 * Size: 9 bytes
 */
typedef struct
{
    uint32_t object_id;
    uint8_t  object_type;
    uint32_t payload_length;

} ap_config_object_header_t;



#ifdef __cplusplus
}
#endif

#endif /* AP_CONFIG_H */