#include "ap_mapper.h"
#include "ap_dispatcher.h"
#include "ap_registry.h"

#define AP_MAX_MAPPINGS 256

static ap_mapping_t mappings[AP_MAX_MAPPINGS];
static uint32_t mapping_count;
static void ap_mapper_event_handler(const ap_event_t *event);

void ap_mapper_init(void)
{
    mapping_count = 0;
	    ap_dispatcher_register(
        ap_mapper_event_handler
    );
}

ap_result_t ap_mapper_add(uint32_t source,
                   uint32_t destination)
{
    if (mapping_count >= AP_MAX_MAPPINGS)
        return AP_ERROR_FULL;

    mappings[mapping_count].source = source;
    mappings[mapping_count].destination = destination;

    mapping_count++;

    return AP_OK;
}

uint32_t ap_mapper_process(const ap_event_t *input,
                           ap_event_t *output,
                           uint32_t max_events)
{
    uint32_t count = 0;

    if ((input == NULL) || (output == NULL))
        return 0;

    for (uint32_t i = 0; i < mapping_count; i++)
    {
        if (mappings[i].source != input->signal->id)
            continue;

        const ap_signal_t *signal =
            ap_registry_find(mappings[i].destination);

        if (signal == NULL)
            continue;
		
		if (signal->type != input->signal->type)
		continue;

        if (count >= max_events)
            break;

        output[count] = *input;
        output[count].signal = signal;

        count++;
    }

    return count;
}

static void ap_mapper_event_handler(const ap_event_t *event)
{
    if (event == NULL)
        return;

    ap_event_t outputs[AP_MAX_MAPPINGS];

    uint32_t count =
        ap_mapper_process(
            event,
            outputs,
            AP_MAX_MAPPINGS
        );

    for (uint32_t i = 0; i < count; i++)
    {
        ap_dispatcher_publish(&outputs[i]);
    }
}