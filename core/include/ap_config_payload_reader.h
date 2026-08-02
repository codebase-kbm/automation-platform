#ifndef AP_CONFIG_PAYLOAD_READER_H
#define AP_CONFIG_PAYLOAD_READER_H

#include <stdint.h>

#include "ap_config.h"
#include "ap_result.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    const uint8_t *data;
    size_t length;
    size_t offset;

} ap_config_payload_reader_t;

int ap_config_read_u8(ap_config_payload_reader_t *reader,uint8_t *value);
int ap_config_read_u16_le(ap_config_payload_reader_t *reader,uint16_t *value);
int ap_config_read_u32_le(ap_config_payload_reader_t *reader,uint32_t *value);
int ap_config_read_float(ap_config_payload_reader_t *reader,float *value);
int ap_config_read_string(ap_config_payload_reader_t *reader,char **value);

#ifdef __cplusplus
}
#endif

#endif