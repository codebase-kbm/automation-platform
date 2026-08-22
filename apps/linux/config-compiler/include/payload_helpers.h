#ifndef PAYLOAD_HELPERS_H
#define PAYLOAD_HELPERS_H

#include "ap_plugin_compiler.h"

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int payload_write_u8(ap_plugin_config_buffer_t *buffer,uint8_t value);
int payload_write_u16_le(ap_plugin_config_buffer_t *buffer,uint16_t value);
int payload_write_u32_le(ap_plugin_config_buffer_t *buffer,uint32_t value);
int payload_write_string(ap_plugin_config_buffer_t *buffer,const char *string);
int payload_write_float(ap_plugin_config_buffer_t *buffer,float value);

#ifdef __cplusplus
}
#endif
#endif