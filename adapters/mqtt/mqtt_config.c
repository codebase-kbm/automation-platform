#include "mqtt_config.h"

#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static char *ap_mqtt_strdup(
    const char *source
)
{
    if (source == NULL)
        return NULL;


    size_t length =
        strlen(source) + 1;


    char *copy =
        malloc(length);


    if (copy == NULL)
        return NULL;


    memcpy(
        copy,
        source,
        length
    );


    return copy;
}


ap_result_t ap_mqtt_config_load(
    const char *filename,
    ap_mqtt_config_t *config
)
{
    if (filename == NULL ||
        config == NULL)
    {
        return AP_ERROR_INVALID_ARGUMENT;
    }


    FILE *file =
        fopen(
            filename,
            "rb"
        );


    if (file == NULL)
        return AP_ERROR_NOT_FOUND;


    fseek(
        file,
        0,
        SEEK_END
    );


    long file_size =
        ftell(file);


    rewind(file);


    if (file_size <= 0)
    {
        fclose(file);

        return AP_ERROR_INVALID_ARGUMENT;
    }


    char *buffer =
        malloc(
            (size_t)file_size + 1
        );


    if (buffer == NULL)
    {
        fclose(file);

        return AP_ERROR_OUT_OF_MEMORY;
    }


    size_t read_size =
        fread(
            buffer,
            1,
            (size_t)file_size,
            file
        );


    fclose(file);


    buffer[read_size] = '\0';


    cJSON *root =
        cJSON_Parse(buffer);


    free(buffer);


    if (root == NULL)
        return AP_ERROR_INVALID_ARGUMENT;


    cJSON *host =
        cJSON_GetObjectItemCaseSensitive(
            root,
            "host"
        );


    cJSON *port =
        cJSON_GetObjectItemCaseSensitive(
            root,
            "port"
        );


    cJSON *username =
        cJSON_GetObjectItemCaseSensitive(
            root,
            "username"
        );


    cJSON *password =
        cJSON_GetObjectItemCaseSensitive(
            root,
            "password"
        );


    if (!cJSON_IsString(host) ||
        !cJSON_IsNumber(port) ||
        !cJSON_IsString(username) ||
        !cJSON_IsString(password))
    {
        cJSON_Delete(root);

        return AP_ERROR_INVALID_ARGUMENT;
    }


    config->host =
        ap_mqtt_strdup(
            host->valuestring
        );


    config->port =
        (uint16_t)port->valueint;


    config->username =
        ap_mqtt_strdup(
            username->valuestring
        );


    config->password =
        ap_mqtt_strdup(
            password->valuestring
        );


    cJSON_Delete(root);


    if (config->host == NULL ||
        config->username == NULL ||
        config->password == NULL)
    {
        ap_mqtt_config_free(
            config
        );

        return AP_ERROR_OUT_OF_MEMORY;
    }


    return AP_OK;
}


void ap_mqtt_config_free(
    ap_mqtt_config_t *config
)
{
    if (config == NULL)
        return;


    free(
        (void *)config->host
    );


    free(
        (void *)config->username
    );


    free(
        (void *)config->password
    );


    config->host = NULL;
    config->username = NULL;
    config->password = NULL;
    config->port = 0;
}