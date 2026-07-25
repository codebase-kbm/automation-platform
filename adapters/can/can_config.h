#ifndef AP_CAN_CONFIG_H
#define AP_CAN_CONFIG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    AP_CAN_VALUE_BOOL = 0,
    AP_CAN_VALUE_UINT,
    AP_CAN_VALUE_INT,
    AP_CAN_VALUE_FLOAT

} ap_can_value_type_t;


typedef enum
{
    AP_CAN_BYTE_ORDER_BIG_ENDIAN = 0,
    AP_CAN_BYTE_ORDER_LITTLE_ENDIAN

} ap_can_byte_order_t;


typedef struct
{
    uint32_t signal_id;

    uint8_t start_bit;
    uint8_t bit_length;

    uint8_t value_type;
    uint8_t byte_order;

    int32_t scale_num;
    int32_t scale_den;
    int32_t offset;

} ap_can_mapping_t;


typedef struct
{
    uint32_t can_id;

    uint32_t mapping_count;

    /*
     * Mapping records follow directly
     * after this structure.
     */

} ap_can_frame_config_t;


#ifdef __cplusplus
}
#endif

#endif /* AP_CAN_CONFIG_H */