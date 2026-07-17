#ifndef AP_MAPPER_H
#define AP_MAPPER_H

#include "ap_common.h"
#include "ap_event.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint32_t source;
    uint32_t destination;

} ap_mapping_t;

/* Initialize mapper */
void ap_mapper_init(void);

/* Add mapping rule */
bool ap_mapper_add(uint32_t source,
                   uint32_t destination);

/* Process one event */
uint32_t ap_mapper_process(const ap_event_t *input,
                           ap_event_t *output,
                           uint32_t max_events);

#ifdef __cplusplus
}
#endif

#endif /* AP_MAPPER_H */