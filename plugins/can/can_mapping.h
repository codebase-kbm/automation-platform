#ifndef AP_CAN_MAPPING_H
#define AP_CAN_MAPPING_H

#include "can.h"
#include "ap_result.h"

ap_result_t ap_can_mapping_rx(
    const ap_can_frame_t *frame,
    const ap_can_config_t *config);

#endif /* AP_CAN_MAPPING_H */