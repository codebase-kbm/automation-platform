#ifndef AP_CAN_PLUGIN_H
#define AP_CAN_PLUGIN_H

#include <stdbool.h>
#include "ap_plugin.h"
#include "ap_object.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AP_CAN_FIELD_FLAG_SIGNED 0x01u

typedef enum
{
    AP_CAN_DIRECTION_RX = 0,
    AP_CAN_DIRECTION_TX = 1

} ap_can_direction_t;

typedef enum
{
    AP_CAN_MAPPING_FIELD = 0,
    AP_CAN_MAPPING_RAW

} ap_can_mapping_type_t;

typedef enum
{
    AP_CAN_ENCODING_NONE = 0,
    AP_CAN_ENCODING_LE,
    AP_CAN_ENCODING_BE

} ap_can_encoding_t;

typedef enum
{
    AP_CAN_TRIGGER_NONE = 0,
    AP_CAN_TRIGGER_VALUE,
    AP_CAN_TRIGGER_OBJECT

} ap_can_trigger_type_t;

typedef struct
{
    ap_can_trigger_type_t type;

    union
    {
        bool value;
        uint32_t object_id;
    };

} ap_can_trigger_t;

typedef struct
{
    uint8_t start_bit;
    uint8_t length;
    uint8_t encoding;
    uint8_t flags;

    float scale;
    float offset;

} ap_can_field_t;

typedef struct
{
    uint8_t length;
    uint8_t data[8];

} ap_can_raw_data_t;

typedef struct
{
    uint32_t object_id;

    ap_value_type_t value_type;

    ap_can_direction_t direction;
    ap_can_mapping_type_t mapping_type;

    uint32_t can_id;
    uint8_t dlc;

    ap_can_trigger_t trigger;

    union
    {
        ap_can_field_t field;
        ap_can_raw_data_t raw;
    };

} ap_can_mapping_t;

typedef struct
{
    char *interface;
    uint32_t bitrate;

    uint8_t mapping_count;
    ap_can_mapping_t *mappings;

} ap_can_config_t;


/* -------------------------------------------------- */
/* CAN Backend API                                    */
/* -------------------------------------------------- */

typedef struct
{
    uint32_t can_id;
    uint8_t dlc;
    uint8_t data[8];

} ap_can_frame_t;


typedef struct
{
    const char *interface;
    uint32_t bitrate;

} ap_can_backend_config_t;


typedef struct
{
    void *(*create)(void);

    void (*destroy)(
        void *context);

    int (*open)(
        void *context,
        const ap_can_backend_config_t *config);

    void (*close)(
        void *context);

    int (*send)(
        void *context,
        const ap_can_frame_t *frame);

    int (*receive)(
        void *context,
        ap_can_frame_t *frame);

} ap_can_backend_t;

extern const ap_can_backend_t ap_can_backend;
extern const ap_plugin_t ap_can_plugin;


#ifdef __cplusplus
}
#endif

#endif /* AP_CAN_PLUGIN_H */