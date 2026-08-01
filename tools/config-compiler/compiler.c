#include "compiler.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "ap_config.h"
#include "ap_object.h"
#include "ap_module.h"
#include "ap_plugin_compiler_manager.h"


static int write_u8(
    FILE *file,
    uint8_t value)
{
    return fputc(value, file) == EOF ? -1 : 0;
}


static int write_u16_le(
    FILE *file,
    uint16_t value)
{
    uint8_t data[2] =
    {
        (uint8_t)(value & 0xff),
        (uint8_t)((value >> 8) & 0xff)
    };

    return fwrite(data, 1, sizeof(data), file) == sizeof(data)
        ? 0
        : -1;
}


static int write_u32_le(
    FILE *file,
    uint32_t value)
{
    uint8_t data[4] =
    {
        (uint8_t)(value & 0xff),
        (uint8_t)((value >> 8) & 0xff),
        (uint8_t)((value >> 16) & 0xff),
        (uint8_t)((value >> 24) & 0xff)
    };

    return fwrite(data, 1, sizeof(data), file) == sizeof(data)
        ? 0
        : -1;
}


static int write_object(
    FILE *file,
    uint32_t object_id,
    ap_object_type_t object_type,
    const uint8_t *payload,
    uint32_t payload_length)
{
    if (write_u32_le(file, object_id) != 0)
        return -1;

    if (write_u8(file, (uint8_t)object_type) != 0)
        return -1;

    if (write_u32_le(file, payload_length) != 0)
        return -1;

    if (payload_length > 0)
    {
        if (payload == NULL)
            return -1;

        if (fwrite(
                payload,
                1,
                payload_length,
                file) != payload_length)
        {
            return -1;
        }
    }

    return 0;
}


static uint32_t parse_id(
    const xmlChar *value)
{
    if (value == NULL)
        return 0;

    return (uint32_t)strtoul(
        (const char *)value,
        NULL,
        10
    );
}


static int compile_module(
    FILE *file,
    xmlNodePtr node)
{
    xmlChar *id =
        xmlGetProp(node, BAD_CAST "id");

    xmlChar *type =
        xmlGetProp(node, BAD_CAST "type");

    uint32_t module_id =
        parse_id(id);

    const ap_plugin_compiler_t *plugin =
        ap_plugin_compiler_manager_find_by_name(
            (const char *)type
        );

    if (plugin == NULL)
    {
        fprintf(
            stderr,
            "ERROR: unknown module type '%s'\n",
            type != NULL
                ? (const char *)type
                : "(missing)"
        );

        xmlFree(id);
        xmlFree(type);

        return -1;
    }

    ap_module_type_t module_type =
        plugin->type;

    printf(
        "Module: %u\n"
        "  Type: %s\n"
        "  Type ID: %u\n",
        module_id,
        type != NULL
            ? (const char *)type
            : "(missing)",
        module_type
    );

    if (plugin->compile == NULL)
    {
        fprintf(
            stderr,
            "ERROR: no config compiler registered for module '%s'\n",
            type != NULL
                ? (const char *)type
                : "(missing)"
        );

        xmlFree(id);
        xmlFree(type);

        return -1;
    }

    uint8_t payload[1024];

    ap_plugin_config_buffer_t buffer =
    {
        .data = payload,
        .length = 0,
        .capacity = sizeof(payload)
    };

    if (plugin->compile(node, &buffer) != 0)
    {
        fprintf(
            stderr,
            "ERROR: module configuration compilation failed\n"
        );

        xmlFree(id);
        xmlFree(type);

        return -1;
    }

    int result =
        write_object(
            file,
            module_id,
            AP_OBJECT_MODULE,
            buffer.data,
            buffer.length
        );

    xmlFree(id);
    xmlFree(type);

    return result;
}


static int compile_node(
    FILE *file,
    xmlNodePtr node)
{
    xmlChar *id =
        xmlGetProp(node, BAD_CAST "id");

    uint32_t node_id =
        parse_id(id);

    printf(
        "Node: %u\n",
        node_id
    );

    /*
     * Node objects currently have
     * no payload.
     */
    if (write_object(
            file,
            node_id,
            AP_OBJECT_NODE,
            NULL,
            0) != 0)
    {
        xmlFree(id);
        return -1;
    }

    xmlFree(id);

    for (xmlNodePtr child = node->children;
         child != NULL;
         child = child->next)
    {
        if (child->type != XML_ELEMENT_NODE)
            continue;

        if (xmlStrcmp(
                child->name,
                BAD_CAST "module") == 0)
        {
            if (compile_module(file, child) != 0)
                return -1;
        }
    }

    return 0;
}


int ap_config_compile(
    const char *input_file,
    const char *output_file)
{
    if (input_file == NULL ||
        output_file == NULL)
    {
        return 1;
    }

    xmlDocPtr doc =
        xmlReadFile(
            input_file,
            NULL,
            XML_PARSE_NOBLANKS
        );

    if (doc == NULL)
    {
        fprintf(
            stderr,
            "ERROR: cannot read '%s'\n",
            input_file
        );

        return 1;
    }

    xmlNodePtr root =
        xmlDocGetRootElement(doc);

    if (root == NULL ||
        xmlStrcmp(
            root->name,
            BAD_CAST "automation") != 0)
    {
        fprintf(
            stderr,
            "ERROR: root element must be <automation>\n"
        );

        xmlFreeDoc(doc);
        return 1;
    }

    FILE *file =
        fopen(output_file, "wb");

    if (file == NULL)
    {
        fprintf(
            stderr,
            "ERROR: cannot create '%s'\n",
            output_file
        );

        xmlFreeDoc(doc);
        return 1;
    }

    /*
     * File header
     */
    if (write_u16_le(
            file,
            AP_CONFIG_MAGIC) != 0 ||
        write_u8(
            file,
            AP_CONFIG_VERSION) != 0)
    {
        fclose(file);
        xmlFreeDoc(doc);
        return 1;
    }

    printf("Root: automation\n");

    for (xmlNodePtr node = root->children;
         node != NULL;
         node = node->next)
    {
        if (node->type != XML_ELEMENT_NODE)
            continue;

        if (xmlStrcmp(
                node->name,
                BAD_CAST "node") == 0)
        {
            if (compile_node(file, node) != 0)
            {
                fclose(file);
                xmlFreeDoc(doc);
                return 1;
            }
        }
    }

    fclose(file);
    xmlFreeDoc(doc);

    printf(
        "Configuration written to %s\n",
        output_file
    );

    return 0;
}