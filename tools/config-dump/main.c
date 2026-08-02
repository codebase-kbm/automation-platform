#include <stdio.h>

#include "ap_config_reader.h"
#include "ap_module.h"
#include "ap_object.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <config.bin>\n", argv[0]);
        return 1;
    }

    ap_result_t result =
        ap_config_reader_open(argv[1]);

    if (result != AP_OK)
    {
        printf("Config open failed: %d\n", result);
        return 1;
    }

    ap_config_object_t object;

    while (1)
    {
        result =
            ap_config_reader_next(&object);

        if (result == AP_ERROR_NOT_FOUND)
            break;

        if (result != AP_OK)
        {
            printf("Config read failed: %d\n", result);
            ap_config_reader_close();
            return 1;
        }

		if (object.header.object_type == AP_OBJECT_MODULE &&
			object.header.payload_length > 0)
		{
			uint8_t module_type = object.payload[0];

			printf(
				"Object %u: type=%u module=%u payloadlength=%u\n",
				object.header.object_id,
				object.header.object_type,
				module_type,
				object.header.payload_length
			);
            printf("Payload:\n");
            for (uint32_t i = 0; i < object.header.payload_length; i++)
            {
                printf("%02X ",object.payload[i]);
            }
            printf("\n\n");
		}
		else
		{
			printf(
				"Object %u: type=%u payloadlength=%u\n\n",
				object.header.object_id,
				object.header.object_type,
				object.header.payload_length
			);
		}
    }

    ap_config_reader_close();

    return 0;
}