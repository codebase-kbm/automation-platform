#include <stdio.h>

#include "ap_plugin_manager.h"
#include "ap_config_reader.h"


int main(void)
{
    printf("=============================\n");
    printf(" Plugin Manager Test\n");
    printf("=============================\n\n");

    if (ap_config_reader_open("config.bin") != AP_OK)
    {
        printf("Failed to open config.bin\n");
        return 1;
    }

    if (ap_plugin_manager_process() != AP_OK)
    {
        printf("Plugin manager processing failed\n");
        ap_config_reader_close();
        return 1;
    }

    ap_config_reader_close();

    printf("Plugin manager processing successful\n");

    return 0;
}