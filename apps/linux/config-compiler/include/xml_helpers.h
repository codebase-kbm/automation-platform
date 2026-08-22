#ifndef XML_HELPERS_H
#define XML_HELPERS_H



#ifdef __cplusplus
extern "C" {
#endif

xmlNodePtr find_child(xmlNodePtr parent,const char *name);
xmlChar *get_attribute(xmlNodePtr node,const char *name);
int parse_u8(const xmlChar *value,uint8_t *result);
int parse_u16(const xmlChar *value,uint16_t *result);
int parse_u32(const xmlChar *value,uint32_t *result);
int parse_u32_hex(const xmlChar *value,uint32_t *result);
int parse_bool(const xmlChar *value,uint8_t *result);
int parse_value_type(const xmlChar *value,uint8_t *result);

#ifdef __cplusplus
}
#endif
#endif