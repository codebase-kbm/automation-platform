#include "ap_timestamp.h"
#define _POSIX_C_SOURCE 200809L
#include <time.h>

void ap_timestamp_init(void)
{
    /* Nothing to do on Linux */
}

uint64_t ap_timestamp_now(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)ts.tv_sec * 1000000ULL +
           (uint64_t)ts.tv_nsec / 1000ULL;
}