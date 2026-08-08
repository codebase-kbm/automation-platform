#include "ap_config_reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *config_data = NULL;
static size_t config_size = 0;
static size_t config_offset = 0;


static uint16_t read_u16_le(
    const uint8_t *data)
{
    return (uint16_t)data[0]
         | ((uint16_t)data[1] << 8);
}


static uint32_t read_u32_le(
    const uint8_t *data)
{
    return (uint32_t)data[0]
         | ((uint32_t)data[1] << 8)
         | ((uint32_t)data[2] << 16)
         | ((uint32_t)data[3] << 24);
}


ap_result_t ap_config_reader_open(
    const char *filename)
{
    if (filename == NULL)
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_CORE,AP_COMPONENT_CONFIG_READER,AP_PLUGIN_NONE,AP_ERROR_INVALID_ARGUMENT);

    ap_config_reader_close();

    FILE *file = fopen(filename, "rb");

    if (file == NULL)
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_CORE,AP_COMPONENT_CONFIG_READER,AP_PLUGIN_NONE,AP_ERROR_NOT_FOUND);

    if (fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_CORE,AP_COMPONENT_CONFIG_READER,AP_PLUGIN_NONE,AP_ERROR_OPERATION_FAILED);
    }

    long file_size = ftell(file);

    if (file_size < (long)AP_CONFIG_HEADER_SIZE)
    {
        fclose(file);
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_CORE,AP_COMPONENT_CONFIG_READER,AP_PLUGIN_NONE,AP_ERROR_INVALID_SIZE);
    }

    rewind(file);

    config_data =
        malloc((size_t)file_size);

    if (config_data == NULL)
    {
        fclose(file);
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_CORE,AP_COMPONENT_CONFIG_READER,AP_PLUGIN_NONE,AP_ERROR_OUT_OF_MEMORY);
    }

    size_t read_size =
        fread(
            config_data,
            1,
            (size_t)file_size,
            file
        );

    fclose(file);

    if (read_size != (size_t)file_size)
    {
        ap_config_reader_close();
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_CORE,AP_COMPONENT_CONFIG_READER,AP_PLUGIN_NONE,AP_ERROR_OPERATION_FAILED);
    }

    config_size = read_size;
    config_offset = 0;

    /*
     * File header:
     *
     * uint16 magic
     * uint8  version
     */

    uint16_t magic = read_u16_le(config_data);

    uint8_t version = config_data[2];

    if (magic != AP_CONFIG_MAGIC)
    {
        ap_config_reader_close();
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_CORE,AP_COMPONENT_CONFIG_READER,AP_PLUGIN_NONE,AP_ERROR_INVALID_ARGUMENT);
    }

    if (version != AP_CONFIG_VERSION)
    {
        ap_config_reader_close();
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_CORE,AP_COMPONENT_CONFIG_READER,AP_PLUGIN_NONE,AP_ERROR_INVALID_ARGUMENT);
    }

    config_offset = AP_CONFIG_HEADER_SIZE;
    return AP_OK;
}


ap_result_t ap_config_reader_next(
    ap_config_object_t *object)
{
    if (object == NULL)
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_CORE,AP_COMPONENT_CONFIG_READER,AP_PLUGIN_NONE,AP_ERROR_INVALID_ARGUMENT);

    memset(
        object,
        0,
        sizeof(*object)
    );

    if (config_data == NULL)
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_CORE,AP_COMPONENT_CONFIG_READER,AP_PLUGIN_NONE,AP_ERROR_NOT_INITIALIZED);

    /*
     * No complete object header remaining.
     */
    if (config_offset >= config_size)
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_CORE,AP_COMPONENT_CONFIG_READER,AP_PLUGIN_NONE,AP_ERROR_NOT_FOUND);

    if (config_size - config_offset <
        AP_CONFIG_OBJECT_HEADER_SIZE)
    {
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_CORE,AP_COMPONENT_CONFIG_READER,AP_PLUGIN_NONE,AP_ERROR_INVALID_ARGUMENT);
    }

	const uint8_t *header =
		&config_data[config_offset];

	object->header.object_id =
		read_u32_le(header);

	object->header.object_type =
		header[4];

	object->header.payload_length =
		read_u32_le(&header[5]);

	config_offset +=
		AP_CONFIG_OBJECT_HEADER_SIZE;

    /*
     * Make sure the complete payload exists.
     */
    if (config_size - config_offset <
        object->header.payload_length)
    {
        ap_config_reader_close();
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_CORE,AP_COMPONENT_CONFIG_READER,AP_PLUGIN_NONE,AP_ERROR_INVALID_ARGUMENT);
    }

    object->payload =
        &config_data[config_offset];

    config_offset +=
        object->header.payload_length;

    return AP_OK;
}


void ap_config_reader_close(void)
{
    free(config_data);

    config_data = NULL;
    config_size = 0;
    config_offset = 0;
}