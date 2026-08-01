#include <stdio.h>

#include "ap_plugin_manager.h"
#include "ap_config_reader.h"
#include "ap_result.h"


int main(int argc, char **argv)
{
    printf("=============================\n");
    printf(" Plugin Manager Test\n");
    printf("=============================\n\n");

    const char *config_path = "build/config.bin";

    if (argc > 1)
    {
        config_path = argv[1];
    }

    if (ap_config_reader_open(config_path) != AP_OK)
    {
        printf("Failed to open config.bin\n");
        return 1;
    }

	ap_result_t result;
	result = ap_plugin_manager_process();

    if (result != AP_OK)
    {
        printf("Plugin manager processing failed: %d\n",result);
        ap_config_reader_close();
        return 1;
    }

    ap_config_reader_close();

    printf("Plugin manager processing successful\n");

    return 0;
}