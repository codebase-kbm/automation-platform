#include <string.h>
#include "payload_helpers.h"


int payload_write_u8(ap_plugin_config_buffer_t *buffer,uint8_t value)
{
    if (buffer->length + 1 > buffer->capacity)
        return -1;

    buffer->data[buffer->length++] = value;

    return 0;
}

int payload_write_u16_le(ap_plugin_config_buffer_t *buffer,uint16_t value)
{
    if (buffer->length + 2 > buffer->capacity)
        return -1;
    buffer->data[buffer->length++] = (uint8_t)(value & 0xffu);
    buffer->data[buffer->length++] = (uint8_t)((value >> 8) & 0xffu);
    return 0;
}

int payload_write_u32_le(ap_plugin_config_buffer_t *buffer,uint32_t value)
{
    if (buffer->length + 4 > buffer->capacity)
        return -1;
    buffer->data[buffer->length++] = (uint8_t)(value & 0xffu);
    buffer->data[buffer->length++] = (uint8_t)((value >> 8) & 0xffu);
    buffer->data[buffer->length++] = (uint8_t)((value >> 16) & 0xffu);
    buffer->data[buffer->length++] = (uint8_t)((value >> 24) & 0xffu);
    return 0;
}


int payload_write_string(ap_plugin_config_buffer_t *buffer,const char *string)
{
    size_t length;

    if (string == NULL)
        return -1;

    length = strlen(string);

    if (length > UINT8_MAX)
        return -1;

    if (payload_write_u8(buffer, (uint8_t)length) != 0)
        return -1;

    if (buffer->length + length > buffer->capacity)
        return -1;

    memcpy(&buffer->data[buffer->length],string,length);
    buffer->length += length;

    return 0;
}

int payload_write_float(ap_plugin_config_buffer_t *buffer,float value)
{
    uint32_t raw;
    memcpy(&raw,&value,sizeof(raw));
    return payload_write_u32_le(buffer,raw);
}

