#include <stdio.h>
#include <stdint.h>

#include "ap_config.h"
#include "ap_object.h"
#include "modules.h"

static void write_u16_le(FILE *file, uint16_t value)
{
    uint8_t data[2];

    data[0] = (uint8_t)(value & 0xFFu);
    data[1] = (uint8_t)((value >> 8) & 0xFFu);

    fwrite(data, 1, sizeof(data), file);
}

static void write_u32_le(FILE *file, uint32_t value)
{
    uint8_t data[4];

    data[0] = (uint8_t)(value & 0xFFu);
    data[1] = (uint8_t)((value >> 8) & 0xFFu);
    data[2] = (uint8_t)((value >> 16) & 0xFFu);
    data[3] = (uint8_t)((value >> 24) & 0xFFu);

    fwrite(data, 1, sizeof(data), file);
}

static void write_object(
    FILE *file,
    uint32_t object_id,
    uint8_t object_type,
    const uint8_t *payload,
    uint32_t payload_length)
{
    write_u32_le(file, object_id);

    fputc(object_type, file);

    write_u32_le(file, payload_length);

    if (payload_length > 0 && payload != NULL)
        fwrite(payload, 1, payload_length, file);
}

static void write_module(
    FILE *file,
    uint32_t object_id,
    ap_module_type_t module_type,
    const uint8_t *payload,
    uint32_t payload_length)
{
    write_u32_le(file, object_id);

    fputc(AP_OBJECT_MODULE, file);

    write_u32_le(file, payload_length + 1u);

    fputc((uint8_t)module_type, file);

    if (payload_length > 0 && payload != NULL)
        fwrite(payload, 1, payload_length, file);
}

int main(void)
{
    FILE *file = fopen("test.bin", "wb");

    if (file == NULL)
    {
        perror("test.bin");
        return 1;
    }

    /*
     * File header
     */
    write_u16_le(file, AP_CONFIG_MAGIC);
    fputc(AP_CONFIG_VERSION, file);

    /*
     * NODE
     *
     * ID 1
     * No value type
     * No payload
     */
	write_object(
		file,
		1,
		AP_OBJECT_NODE,
		NULL,
		0
	);

    /*
     * MODULE
     *
     * ID 500
     * Dummy payload: 01 02 03 04
     */
	const uint8_t module_payload[] =
	{
		0x01, 0x02, 0x03, 0x04
	};

	write_module(
		file,
		500,
		AP_MODULE_MQTT,
		module_payload,
		sizeof(module_payload)
	);

    fclose(file);

    printf("Created test.bin\n");

    return 0;
}