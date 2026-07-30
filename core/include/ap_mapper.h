#ifndef AP_MAPPER_H
#define AP_MAPPER_H

#include "ap_common.h"
#include "ap_event.h"
#include "ap_object.h"
#include "ap_result.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    ap_object_id_t source;
    ap_object_id_t destination;

} ap_mapping_t;

/* Initialize mapper */
void ap_mapper_init(void);

/* Add mapping rule */
ap_result_t ap_mapper_add(ap_object_id_t source,
                          ap_object_id_t destination);

/* Process one event */
uint32_t ap_mapper_process(const ap_event_t *input,
                            ap_event_t *output,
                            uint32_t max_events);

#ifdef __cplusplus
}
#endif

#endif /* AP_MAPPER_H */