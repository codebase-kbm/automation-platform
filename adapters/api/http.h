#ifndef AP_HTTP_H
#define AP_HTTP_H

#include <stddef.h>
#include <stdint.h>

#include "ap_result.h"

typedef enum
{
    AP_HTTP_METHOD_GET = 0,
    AP_HTTP_METHOD_POST,
    AP_HTTP_METHOD_PUT,
    AP_HTTP_METHOD_DELETE

} ap_http_method_t;

typedef struct
{
    const char *name;
    const char *value;

} ap_http_header_t;

typedef struct
{
    ap_http_method_t method;

    const char *url;

    const ap_http_header_t *headers;
    size_t header_count;

    const uint8_t *body;
    size_t body_length;

} ap_http_request_t;

typedef struct
{
    uint16_t status_code;

    uint8_t *body;
    size_t body_length;

} ap_http_response_t;

typedef struct ap_http_backend
{
    ap_result_t (*create)(
        void **context
    );

    void (*destroy)(
        void *context
    );

    ap_result_t (*open)(
        void *context
    );

    void (*close)(
        void *context
    );

    ap_result_t (*request)(
        void *context,
        const ap_http_request_t *request,
        ap_http_response_t *response
    );

    void (*response_free)(
        void *context,
        ap_http_response_t *response
    );

} ap_http_backend_t;

extern const ap_http_backend_t ap_http_backend;

#endif /* AP_HTTP_H */