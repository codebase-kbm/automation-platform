#include "mqtt_mapping.h"

#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ap_registry.h"


static ap_mqtt_mapping_t *mappings = NULL;
static uint32_t mapping_count = 0;


static char *ap_mqtt_mapping_strdup(
    const char *source
)
{
    if (source == NULL)
        return NULL;

    size_t length = strlen(source) + 1;

    char *copy = malloc(length);

    if (copy == NULL)
        return NULL;

    memcpy(copy, source, length);

    return copy;
}


static ap_result_t
ap_mqtt_mapping_parse_value_type(
    const char *type,
    ap_value_type_t *result
)
{
    if (type == NULL || result == NULL)
        return AP_ERROR_INVALID_ARGUMENT;

    if (strcmp(type, "bool") == 0)
    {
        *result = AP_VALUE_BOOL;
        return AP_OK;
    }

    if (strcmp(type, "int32") == 0)
    {
        *result = AP_VALUE_INT32;
        return AP_OK;
    }

    if (strcmp(type, "float") == 0)
    {
        *result = AP_VALUE_FLOAT;
        return AP_OK;
    }

    if (strcmp(type, "string") == 0)
    {
        *result = AP_VALUE_STRING;
        return AP_OK;
    }

    return AP_ERROR_INVALID_TYPE;
}


static ap_result_t
ap_mqtt_mapping_parse_direction(
    const char *direction,
    ap_mqtt_direction_t *result
)
{
    if (direction == NULL || result == NULL)
        return AP_ERROR_INVALID_ARGUMENT;

    if (strcmp(direction, "subscribe") == 0)
    {
        *result = AP_MQTT_DIRECTION_SUBSCRIBE;
        return AP_OK;
    }

    if (strcmp(direction, "publish") == 0)
    {
        *result = AP_MQTT_DIRECTION_PUBLISH;
        return AP_OK;
    }

    if (strcmp(direction, "both") == 0)
    {
        *result = AP_MQTT_DIRECTION_BOTH;
        return AP_OK;
    }

    return AP_ERROR_INVALID_ARGUMENT;
}


ap_result_t ap_mqtt_mapping_load(
    const char *filename
)
{
    if (filename == NULL)
        return AP_ERROR_INVALID_ARGUMENT;

    ap_mqtt_mapping_free();

    FILE *file = fopen(filename, "rb");

    if (file == NULL)
        return AP_ERROR_NOT_FOUND;

    fseek(file, 0, SEEK_END);

    long file_size = ftell(file);

    rewind(file);

    if (file_size <= 0)
    {
        fclose(file);
        return AP_ERROR_INVALID_ARGUMENT;
    }

    char *buffer = malloc((size_t)file_size + 1);

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

    cJSON *root = cJSON_Parse(buffer);

    free(buffer);

    if (root == NULL)
        return AP_ERROR_INVALID_ARGUMENT;

    cJSON *mapping_array =
        cJSON_GetObjectItemCaseSensitive(
            root,
            "mappings"
        );

    if (!cJSON_IsArray(mapping_array))
    {
        cJSON_Delete(root);
        return AP_ERROR_INVALID_ARGUMENT;
    }

    int count = cJSON_GetArraySize(mapping_array);

    if (count <= 0)
    {
        cJSON_Delete(root);
        return AP_OK;
    }

    mappings =
        calloc(
            (size_t)count,
            sizeof(ap_mqtt_mapping_t)
        );

    if (mappings == NULL)
    {
        cJSON_Delete(root);
        return AP_ERROR_OUT_OF_MEMORY;
    }


    for (int i = 0; i < count; i++)
    {
        cJSON *item =
            cJSON_GetArrayItem(
                mapping_array,
                i
            );

        cJSON *topic =
            cJSON_GetObjectItemCaseSensitive(
                item,
                "topic"
            );

        cJSON *object_id =
            cJSON_GetObjectItemCaseSensitive(
                item,
                "object_id"
            );

        cJSON *type =
            cJSON_GetObjectItemCaseSensitive(
                item,
                "type"
            );

        cJSON *direction =
            cJSON_GetObjectItemCaseSensitive(
                item,
                "direction"
            );
			
		cJSON *qos =
			cJSON_GetObjectItemCaseSensitive(
				item,
				"qos"
			);

		cJSON *retain =
			cJSON_GetObjectItemCaseSensitive(
				item,
				"retain"
			);


        if (!cJSON_IsString(topic) ||
            !cJSON_IsNumber(object_id) ||
            !cJSON_IsString(type) ||
            !cJSON_IsString(direction))
        {
            ap_mqtt_mapping_free();
            cJSON_Delete(root);

            return AP_ERROR_INVALID_ARGUMENT;
        }


        ap_result_t direction_result =
            ap_mqtt_mapping_parse_direction(
                direction->valuestring,
                &mappings[i].direction
            );

        if (direction_result != AP_OK)
        {
            ap_mqtt_mapping_free();
            cJSON_Delete(root);

            return direction_result;
        }
		
		/* Defaults */
		mappings[i].qos = 0;
		mappings[i].retain = false;

		if (qos != NULL)
		{
			if (!cJSON_IsNumber(qos) ||
				qos->valueint < 0 ||
				qos->valueint > 2)
			{
				ap_mqtt_mapping_free();
				cJSON_Delete(root);

				return AP_ERROR_INVALID_ARGUMENT;
			}

			mappings[i].qos = qos->valueint;
		}

		if (retain != NULL)
		{
			if (!cJSON_IsBool(retain))
			{
				ap_mqtt_mapping_free();
				cJSON_Delete(root);

				return AP_ERROR_INVALID_ARGUMENT;
			}

			mappings[i].retain = cJSON_IsTrue(retain);
		}


        mappings[i].topic =
            ap_mqtt_mapping_strdup(
                topic->valuestring
            );

        if (mappings[i].topic == NULL)
        {
            ap_mqtt_mapping_free();
            cJSON_Delete(root);

            return AP_ERROR_OUT_OF_MEMORY;
        }


        mappings[i].object.id =
            (ap_object_id_t)object_id->valueint;

        mappings[i].object.object_type =
            AP_OBJECT_SIGNAL;

        mappings[i].object.status =
            AP_OK;


        ap_result_t type_result =
            ap_mqtt_mapping_parse_value_type(
                type->valuestring,
                &mappings[i].object.value_type
            );

        if (type_result != AP_OK)
        {
            ap_mqtt_mapping_free();
            cJSON_Delete(root);

            return type_result;
        }


        const ap_object_t *existing_object =
            ap_registry_find(
                mappings[i].object.id
            );


        if (existing_object != NULL)
        {
            if (existing_object->object_type !=
                    AP_OBJECT_SIGNAL ||
                existing_object->value_type !=
                    mappings[i].object.value_type)
            {
                ap_mqtt_mapping_free();
                cJSON_Delete(root);

                return AP_ERROR_INVALID_TYPE;
            }
        }
        else
        {
            ap_result_t register_result =
                ap_registry_register(
                    &mappings[i].object
                );

            if (register_result != AP_OK)
            {
                ap_mqtt_mapping_free();
                cJSON_Delete(root);

                return register_result;
            }
        }
    }


    mapping_count = (uint32_t)count;

    cJSON_Delete(root);

    return AP_OK;
}


void ap_mqtt_mapping_free(void)
{
    if (mappings == NULL)
    {
        mapping_count = 0;
        return;
    }

    for (uint32_t i = 0;
         i < mapping_count;
         i++)
    {
        free((void *)mappings[i].topic);
    }

    free(mappings);

    mappings = NULL;
    mapping_count = 0;
}


const ap_mqtt_mapping_t *
ap_mqtt_mapping_find_by_topic(
    const char *topic
)
{
    if (topic == NULL)
        return NULL;

    for (uint32_t i = 0;
         i < mapping_count;
         i++)
    {
        if (strcmp(
                mappings[i].topic,
                topic
            ) == 0)
        {
            return &mappings[i];
        }
    }

    return NULL;
}


const ap_mqtt_mapping_t *
ap_mqtt_mapping_find_by_object(
    ap_object_id_t object_id
)
{
    for (uint32_t i = 0;
         i < mapping_count;
         i++)
    {
        if (mappings[i].object.id == object_id)
            return &mappings[i];
    }

    return NULL;
}


uint32_t ap_mqtt_mapping_count(void)
{
    return mapping_count;
}


const ap_mqtt_mapping_t *
ap_mqtt_mapping_get(
    uint32_t index
)
{
    if (index >= mapping_count)
        return NULL;

    return &mappings[index];
}