#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <curl/curl.h>

#include "http.h"
#include "ap_result.h"

typedef struct
{
    uint8_t *data;
    size_t length;
    size_t capacity;

} ap_http_curl_buffer_t;

typedef struct
{
    CURL *curl;

} ap_http_curl_context_t;

static size_t ap_http_curl_write_callback(
    char *ptr,
    size_t size,
    size_t count,
    void *userdata)
{
    ap_http_curl_buffer_t *buffer =
        (ap_http_curl_buffer_t *)userdata;

    size_t length = size * count;

    if (buffer == NULL ||
        ptr == NULL)
    {
        return 0;
    }

    if (length == 0)
        return 0;

    if (buffer->length > SIZE_MAX - length)
        return 0;

    size_t required =
        buffer->length + length;

    if (required > buffer->capacity)
    {
        size_t new_capacity =
            buffer->capacity == 0
                ? 1024
                : buffer->capacity;

        while (new_capacity < required)
        {
            if (new_capacity >
                SIZE_MAX / 2)
            {
                new_capacity = required;
                break;
            }

            new_capacity *= 2;
        }

        uint8_t *data =
            realloc(
                buffer->data,
                new_capacity
            );

        if (data == NULL)
            return 0;

        buffer->data = data;
        buffer->capacity = new_capacity;
    }

    memcpy(
        &buffer->data[buffer->length],
        ptr,
        length
    );

    buffer->length += length;

    return length;
}

static struct curl_slist *
ap_http_curl_build_headers(
    const ap_http_request_t *request)
{
    struct curl_slist *headers = NULL;

    for (size_t i = 0;
         i < request->header_count;
         i++)
    {
        const ap_http_header_t *header =
            &request->headers[i];

        if (header->name == NULL ||
            header->value == NULL)
        {
            curl_slist_free_all(headers);
            return NULL;
        }

        size_t name_length =
            strlen(header->name);

        size_t value_length =
            strlen(header->value);

        if (name_length >
                SIZE_MAX - value_length - 3)
        {
            curl_slist_free_all(headers);
            return NULL;
        }

        size_t length =
            name_length +
            value_length +
            2;

        char *line =
            malloc(length + 1);

        if (line == NULL)
        {
            curl_slist_free_all(headers);
            return NULL;
        }

        memcpy(
            line,
            header->name,
            name_length
        );

        line[name_length] = ':';
        line[name_length + 1] = ' ';

        memcpy(
            &line[name_length + 2],
            header->value,
            value_length
        );

        line[length] = '\0';

        struct curl_slist *new_headers =
            curl_slist_append(
                headers,
                line
            );

        free(line);

        if (new_headers == NULL)
        {
            curl_slist_free_all(headers);
            return NULL;
        }

        headers = new_headers;
    }

    return headers;
}

static ap_result_t ap_http_curl_create(
    void **context)
{
    if (context == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_ADAPTER,
            AP_COMPONENT_INIT,
            0,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    ap_http_curl_context_t *ctx =
        calloc(
            1,
            sizeof(*ctx)
        );

    if (ctx == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_ADAPTER,
            AP_COMPONENT_INIT,
            0,
            AP_ERROR_OUT_OF_MEMORY
        );
    }

    *context = ctx;

    return AP_OK;
}

static void ap_http_curl_destroy(
    void *context)
{
    ap_http_curl_context_t *ctx =
        (ap_http_curl_context_t *)context;

    if (ctx == NULL)
        return;

    free(ctx);
}

static ap_result_t ap_http_curl_open(
    void *context)
{
    ap_http_curl_context_t *ctx =
        (ap_http_curl_context_t *)context;

    if (ctx == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_ADAPTER,
            AP_COMPONENT_INIT,
            0,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    if (ctx->curl != NULL)
        return AP_OK;

    CURLcode result =
        curl_global_init(
            CURL_GLOBAL_DEFAULT
        );

    if (result != CURLE_OK)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_ADAPTER,
            AP_COMPONENT_INIT,
            0,
            AP_ERROR_OPERATION_FAILED
        );
    }

    ctx->curl =
        curl_easy_init();

    if (ctx->curl == NULL)
    {
        curl_global_cleanup();

        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_ADAPTER,
            AP_COMPONENT_INIT,
            0,
            AP_ERROR_OPERATION_FAILED
        );
    }

    return AP_OK;
}

static void ap_http_curl_close(
    void *context)
{
    ap_http_curl_context_t *ctx =
        (ap_http_curl_context_t *)context;

    if (ctx == NULL)
        return;

    if (ctx->curl != NULL)
    {
        curl_easy_cleanup(
            ctx->curl
        );

        ctx->curl = NULL;
    }

    curl_global_cleanup();
}

static ap_result_t ap_http_curl_request(
    void *context,
    const ap_http_request_t *request,
    ap_http_response_t *response)
{
    ap_http_curl_context_t *ctx =
        (ap_http_curl_context_t *)context;

    if (ctx == NULL ||
        request == NULL ||
        response == NULL ||
        request->url == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_ADAPTER,
            AP_COMPONENT_PROCESS,
            0,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    if (ctx->curl == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_ADAPTER,
            AP_COMPONENT_PROCESS,
            0,
            AP_ERROR_NOT_INITIALIZED
        );
    }

    memset(
        response,
        0,
        sizeof(*response)
    );

    ap_http_curl_buffer_t buffer =
    {
        .data = NULL,
        .length = 0,
        .capacity = 0
    };

    struct curl_slist *headers = NULL;

    CURL *curl = ctx->curl;

    curl_easy_reset(curl);

    if (curl_easy_setopt(
            curl,
            CURLOPT_URL,
            request->url) != CURLE_OK)
    {
        goto error;
    }

    if (curl_easy_setopt(
            curl,
            CURLOPT_WRITEFUNCTION,
            ap_http_curl_write_callback) != CURLE_OK)
    {
        goto error;
    }

    if (curl_easy_setopt(
            curl,
            CURLOPT_WRITEDATA,
            &buffer) != CURLE_OK)
    {
        goto error;
    }

    headers =
        ap_http_curl_build_headers(
            request
        );

    if (request->header_count != 0 &&
        headers == NULL)
    {
        goto error;
    }

    if (headers != NULL)
    {
        if (curl_easy_setopt(
                curl,
                CURLOPT_HTTPHEADER,
                headers) != CURLE_OK)
        {
            goto error;
        }
    }

    switch (request->method)
    {
        case AP_HTTP_METHOD_GET:

            if (curl_easy_setopt(
                    curl,
                    CURLOPT_HTTPGET,
                    1L) != CURLE_OK)
            {
                goto error;
            }

            break;

        case AP_HTTP_METHOD_POST:

            if (curl_easy_setopt(
                    curl,
                    CURLOPT_POST,
                    1L) != CURLE_OK ||
                curl_easy_setopt(
                    curl,
                    CURLOPT_POSTFIELDS,
                    request->body) != CURLE_OK ||
                curl_easy_setopt(
                    curl,
                    CURLOPT_POSTFIELDSIZE,
                    (long)request->body_length) != CURLE_OK)
            {
                goto error;
            }

            break;

        case AP_HTTP_METHOD_PUT:

            if (curl_easy_setopt(
                    curl,
                    CURLOPT_CUSTOMREQUEST,
                    "PUT") != CURLE_OK ||
                curl_easy_setopt(
                    curl,
                    CURLOPT_POSTFIELDS,
                    request->body) != CURLE_OK ||
                curl_easy_setopt(
                    curl,
                    CURLOPT_POSTFIELDSIZE,
                    (long)request->body_length) != CURLE_OK)
            {
                goto error;
            }

            break;

        case AP_HTTP_METHOD_DELETE:

            if (curl_easy_setopt(
                    curl,
                    CURLOPT_CUSTOMREQUEST,
                    "DELETE") != CURLE_OK)
            {
                goto error;
            }

            if (request->body != NULL &&
                request->body_length != 0)
            {
                if (curl_easy_setopt(
                        curl,
                        CURLOPT_POSTFIELDS,
                        request->body) != CURLE_OK ||
                    curl_easy_setopt(
                        curl,
                        CURLOPT_POSTFIELDSIZE,
                        (long)request->body_length) != CURLE_OK)
                {
                    goto error;
                }
            }

            break;

        default:

            goto error;
    }

    CURLcode result = curl_easy_perform(curl);

    if (result != CURLE_OK)
    {
        fprintf(
            stderr,
            "AP_HTTP: curl error: %s\n",
            curl_easy_strerror(result)
        );

        goto error;
    }

    long status_code = 0;

    if (curl_easy_getinfo(
            curl,
            CURLINFO_RESPONSE_CODE,
            &status_code) != CURLE_OK)
    {
        goto error;
    }

    if (status_code != 204)
    {
        fprintf(
            stderr,
            "AP_HTTP: HTTP status: %ld\n",
            status_code
        );

        if (buffer.data != NULL &&
            buffer.length > 0)
        {
            fprintf(
                stderr,
                "AP_HTTP: response: %.*s\n",
                (int)buffer.length,
                (const char *)buffer.data
            );
        }
    }

    response->status_code =
        (uint16_t)status_code;

    response->body =
        buffer.data;

    response->body_length =
        buffer.length;

    curl_slist_free_all(headers);

    return AP_OK;

error:

    curl_slist_free_all(headers);
    free(buffer.data);

    memset(
        response,
        0,
        sizeof(*response)
    );

    return AP_RESULT_MAKE(
        AP_RESULT_SOURCE_ADAPTER,
        AP_COMPONENT_PROCESS,
        0,
        AP_ERROR_OPERATION_FAILED
    );
}

static void ap_http_curl_response_free(
    void *context,
    ap_http_response_t *response)
{
    (void)context;

    if (response == NULL)
        return;

    free(response->body);

    response->body = NULL;
    response->body_length = 0;
    response->status_code = 0;
}

const ap_http_backend_t ap_http_backend =
{
    .create = ap_http_curl_create,
    .destroy = ap_http_curl_destroy,

    .open = ap_http_curl_open,
    .close = ap_http_curl_close,

    .request = ap_http_curl_request,

    .response_free = ap_http_curl_response_free
};
