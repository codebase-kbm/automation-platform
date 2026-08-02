#include <stdlib.h>
//#include <stdint.h>
//#include <stddef.h>
#include <string.h>
#include "ap_config_payload_reader.h"

/* -------------------------------------------------- */
/* Config reader                                      */
/* -------------------------------------------------- */

int ap_config_read_u8(ap_config_payload_reader_t *reader,uint8_t *value)
{
    if (reader == NULL ||
        value == NULL ||
        reader->offset + 1 > reader->length)
    {
        return -1;
    }
    *value = reader->data[reader->offset++];
    return 0;
}

int ap_config_read_u16_le(ap_config_payload_reader_t *reader,uint16_t *value)
{
    uint16_t result;
    if (reader == NULL || value == NULL || reader->offset + 2 > reader->length)
    {
        return -1;
    }
    result = (uint16_t)reader->data[reader->offset] | ((uint16_t)reader->data[reader->offset + 1] << 8);
    reader->offset += 2;
    *value = result;
    return 0;
}

int ap_config_read_u32_le(ap_config_payload_reader_t *reader,uint32_t *value)
{
    uint32_t result;
    if (reader == NULL || value == NULL || reader->offset + 4 > reader->length)
    {
        return -1;
    }
    result =
        (uint32_t)reader->data[reader->offset] |
        ((uint32_t)reader->data[reader->offset + 1] << 8) |
        ((uint32_t)reader->data[reader->offset + 2] << 16) |
        ((uint32_t)reader->data[reader->offset + 3] << 24);

    reader->offset += 4;
    *value = result;
    return 0;
}

int ap_config_read_string(ap_config_payload_reader_t *reader,char **value)
{
    uint8_t length;
    char *string;

    if (reader == NULL || value == NULL)
    {
        return -1;
    }

    if (ap_config_read_u8(reader, &length) != 0)
        return -1;

    if (reader->offset + length > reader->length)
        return -1;

    string = malloc((size_t)length + 1);

    if (string == NULL)
        return -1;

    memcpy(string,&reader->data[reader->offset],length);
    string[length] = '\0';
    reader->offset += length;
    *value = string;
    return 0;
}

int ap_config_read_float(ap_config_payload_reader_t *reader,float *value)
{
    if (reader == NULL || value == NULL ||
        reader->offset + sizeof(float) > reader->length)
    {
        return -1;
    }
    memcpy(value,&reader->data[reader->offset],sizeof(float));
    reader->offset += sizeof(float);
    return 0;
}