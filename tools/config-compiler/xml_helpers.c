#include <libxml/tree.h>
#include <stdint.h>
#include <stdlib.h>
#include "xml_helpers.h"
#include "ap_object.h"

xmlNodePtr find_child(xmlNodePtr parent,const char *name)
{
    for (xmlNodePtr node = parent->children;
         node != NULL;
         node = node->next)
    {
        if (node->type != XML_ELEMENT_NODE)
            continue;
        if (xmlStrcmp(node->name,BAD_CAST name) == 0)
        {
            return node;
        }
    }
    return NULL;
}

xmlChar *get_attribute(xmlNodePtr node,const char *name)
{
    return xmlGetProp(node,BAD_CAST name);
}

int parse_u8(const xmlChar *value,uint8_t *result)
{
    uint32_t number;

    if (parse_u32(value, &number) != 0)
        return -1;

    if (number > UINT8_MAX)
        return -1;

    *result = (uint8_t)number;

    return 0;
}

int parse_u16(const xmlChar *value,uint16_t *result)
{
    char *end;
    unsigned long number;

    if (value == NULL || result == NULL)
        return -1;

    number = strtoul(
        (const char *)value,
        &end,
        10
    );

    if (*end != '\0' ||
        number > UINT16_MAX)
    {
        return -1;
    }

    *result = (uint16_t)number;

    return 0;
}


int parse_u32(const xmlChar *value,uint32_t *result)
{
    char *end;
    unsigned long number;

    if (value == NULL || result == NULL)
        return -1;

    number = strtoul(
        (const char *)value,
        &end,
        10
    );

    if (*end != '\0' ||
        number > UINT32_MAX)
    {
        return -1;
    }

    *result = (uint32_t)number;

    return 0;
}

int parse_u32_hex(const xmlChar *value,uint32_t *result)
{
    char *end;
    unsigned long number;

    if (value == NULL || result == NULL)
        return -1;

    number = strtoul(
        (const char *)value,
        &end,
        0
    );

    if (*end != '\0' ||
        number > UINT32_MAX)
    {
        return -1;
    }

    *result = (uint32_t)number;

    return 0;
}

int parse_bool(const xmlChar *value,uint8_t *result)
{
    if (value == NULL || result == NULL)
        return -1;

    if (xmlStrcasecmp(
            value,
            BAD_CAST "true") == 0 ||
        xmlStrcmp(
            value,
            BAD_CAST "1") == 0)
    {
        *result = 1;
        return 0;
    }

    if (xmlStrcasecmp(
            value,
            BAD_CAST "false") == 0 ||
        xmlStrcmp(
            value,
            BAD_CAST "0") == 0)
    {
        *result = 0;
        return 0;
    }

    return -1;
}

int parse_value_type(const xmlChar *value,uint8_t *result)
{
    if (value == NULL || result == NULL)
        return -1;

    if (xmlStrcmp(value, BAD_CAST "bool") == 0)
        *result = AP_VALUE_BOOL;

    else if (xmlStrcmp(value, BAD_CAST "int32") == 0)
        *result = AP_VALUE_INT32;

    else if (xmlStrcmp(value, BAD_CAST "float") == 0)
        *result = AP_VALUE_FLOAT;

    else if (xmlStrcmp(value, BAD_CAST "string") == 0)
        *result = AP_VALUE_STRING;

    else
        return -1;

    return 0;
}