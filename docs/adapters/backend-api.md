# AutomationPlatform Backend API Specification

**Status:** Current
**API Version:** 1
**Language:** C
**Scope:** Platform-independent backend interfaces and platform-specific backend implementations

---

## 1. Purpose

The AutomationPlatform backend API provides a platform-independent interface between plugins and platform-specific implementations.

A plugin shall depend only on the backend API. It shall not contain knowledge about the underlying operating system, hardware, transport library, or platform-specific implementation.

The backend implementation is selected by the build configuration.

The dependency direction is:

```
Plugin
   │
   ▼
Backend API
   │
   ▼
Platform Backend
   │
   ▼
OS / Hardware / External Library
```

The backend API is therefore an abstraction boundary between functional plugin logic and platform-specific implementation.

---

# 2. Directory Structure

Platform-independent backend API headers are located below:

```
adapters/
├── api/
│   ├── http.h
│   ├── ...
│
├── linux/
│   ├── ...
│   └── http/
│       └── http_curl.c
│
└── esp32/
    ├── ...
    └── http/
        └── http_esp.c
```

### Rules

`adapters/api/` contains:

* platform-independent API definitions
* data structures
* function pointer definitions
* result/error interfaces
* ownership contracts

Platform-specific adapter directories contain:

* operating-system-specific code
* hardware-specific code
* external library usage
* concrete backend instances

Backend API headers must never include platform-specific headers.

For example, `http.h` must not include:

```
#include <curl/curl.h>
#include <esp_http_client.h>
#include <windows.h>
```

---

# 3. Backend API Contract

Every reusable backend shall expose a platform-independent API.

The API consists of:

1. Backend-specific data types
2. Backend configuration types
3. Backend context lifecycle
4. Backend operations
5. Resource ownership rules
6. Error handling rules

A backend implementation exposes one backend instance:

```
extern const ap_<backend>_backend_t ap_<backend>_backend;
```

Example:

```
extern const ap_http_backend_t ap_http_backend;
```

The concrete platform implementation provides the instance:

```
const ap_http_backend_t ap_http_backend =
{
    ...
};
```

The plugin does not select the platform implementation.

The build system selects which implementation is linked.

---

# 4. Backend Lifecycle

The standard backend lifecycle is:

```
create
   │
   ▼
open
   │
   ▼
use
   │
   ▼
close
   │
   ▼
destroy
```

## 4.1 create()

Purpose:

* allocate and initialize the backend context
* perform local initialization only
* do not establish external connections

Typical interface:

```
ap_result_t (*create)(
    void **context
);
```

On success:

```
AP_OK
context != NULL
```

The backend owns the allocated context.

The plugin owns the context handle.

---

## 4.2 open()

Purpose:

* initialize the active backend connection
* open files, sockets, devices, sessions, etc.
* prepare the backend for normal operation

Typical interface:

```
ap_result_t (*open)(
    void *context
);
```

`open()` may fail.

If `open()` fails, the plugin shall destroy the context:

```
ap_http_backend.destroy(context);
context = NULL;
```

The backend must leave the context in a state that can safely be destroyed after an `open()` failure.

---

## 4.3 Backend Operations

Backend-specific operations are performed after successful `open()`.

Examples:

```
request()
send()
receive()
publish()
subscribe()
```

The exact operations depend on the backend.

Backend operations shall not expose platform-specific types.

For example, a HTTP backend must not expose:

```
CURL *
```

to the plugin.

It exposes:

```
void *
```

for its opaque context and platform-independent request/response structures.

---

## 4.4 close()

Purpose:

* terminate active connections
* stop backend activity
* release resources associated with an active session

Typical interface:

```
void (*close)(
    void *context
);
```

`close()` shall be safe to call during normal shutdown.

The backend context itself is not released by `close()`.

---

## 4.5 destroy()

Purpose:

* release the backend context
* release all memory owned by the backend context

Typical interface:

```
void (*destroy)(
    void *context
);
```

`destroy()` shall not require the plugin to know the internal context structure.

Normal shutdown:

```
backend.close(context);
backend.destroy(context);
context = NULL;
```

---

# 5. Context Ownership

Backend contexts are opaque.

The plugin must not access backend context internals.

Example:

```
static void *backend_context;
```

Creation:

```
result =
    ap_http_backend.create(
        &backend_context
    );
```

The backend allocates the actual context.

The plugin stores only the returned pointer.

The plugin must never:

```
malloc(sizeof(platform_context));
free(backend_context);
```

The backend is responsible for its own context memory.

The plugin is responsible for the lifetime of the context handle.

---

# 6. Resource Ownership

Every backend API shall explicitly define ownership of dynamically allocated resources.

The following general rule applies:

### Input data

Input data passed to a backend operation is owned by the caller unless explicitly documented otherwise.

Example:

```
request.body
```

is owned by the caller.

The backend may read it during the operation but must not free it.

### Output data

Dynamically allocated output data returned by a backend is owned by the backend until released through the corresponding API function.

Example:

```
response.body
```

is backend-owned.

The caller releases it through:

```
backend.response_free(...);
```

The caller must not directly call:

```
free(response.body);
```

unless the API explicitly states that this is allowed.

---

# 7. Error Handling

New backend APIs shall use:

```
ap_result_t
```

for operations that can fail.

Example:

```
ap_result_t (*open)(
    void *context
);
```

and:

```
ap_result_t (*request)(
    void *context,
    const ap_http_request_t *request,
    ap_http_response_t *response
);
```

Successful operations return:

```
AP_OK
```

Failures return an appropriate `ap_result_t`.

The plugin should propagate backend errors whenever the error is relevant to the plugin lifecycle or operation.

The plugin shall not reinterpret a backend-specific error without a concrete reason.

---

# 8. Platform Independence

Backend API headers must remain completely platform-independent.

A backend API may use:

```
stdint.h
stddef.h
stdbool.h
ap_result.h
```

and other platform-independent AutomationPlatform interfaces.

A backend API must not expose:

```
Linux types
Windows types
ESP-IDF types
POSIX types
external library types
hardware driver types
```

Examples of forbidden API exposure:

```
CURL *
socket_fd
esp_http_client_handle_t
HANDLE
```

These types belong exclusively inside the platform-specific implementation.

---

# 9. Backend Implementation

A platform-specific implementation includes the backend API:

```
#include "http.h"
```

and provides the concrete backend instance.

Example:

```
const ap_http_backend_t ap_http_backend =
{
    .create = ap_http_curl_create,
    .destroy = ap_http_curl_destroy,
    .open = ap_http_curl_open,
    .close = ap_http_curl_close,
    .request = ap_http_curl_request,
    .response_free = ap_http_curl_response_free
};
```

The implementation may use any platform-specific library internally.

For example:

```
http.h
    │
    └── http_curl.c
            │
            └── libcurl
```

or:

```
http.h
    │
    └── http_esp.c
            │
            └── ESP-IDF HTTP client
```

The API presented to the plugin remains identical.

---

# 10. Plugin Responsibilities

A plugin using a backend is responsible for:

* loading its configuration
* creating the backend context
* opening the backend
* using the backend
* closing the backend
* destroying the backend context
* releasing backend-owned resources through the defined API
* translating plugin-level functionality into backend operations

The plugin is not responsible for:

* platform-specific resource management
* external library initialization details
* hardware access
* transport implementation
* backend context internals

---

# 11. Backend Responsibilities

The backend is responsible for:

* platform-specific implementation
* transport handling
* connection handling
* platform resource management
* external library interaction
* backend context allocation
* backend context destruction
* backend-owned output buffers

The backend is not responsible for:

* plugin configuration semantics
* AutomationPlatform object mapping
* plugin-specific protocol semantics
* dispatcher operation
* registry management

---

# 12. HTTP Backend Example

The HTTP backend is defined by:

```
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
```

The public backend instance is:

```
extern const ap_http_backend_t ap_http_backend;
```

The HTTP API contains no platform-specific implementation details.

---

# 13. HTTP Ownership Contract

For the HTTP API:

```
typedef struct
{
    ap_http_method_t method;

    const char *url;

    const ap_http_header_t *headers;
    size_t header_count;

    const uint8_t *body;
    size_t body_length;

} ap_http_request_t;
```

The caller owns:

```
url
headers
header values
body
```

The backend must not free these objects.

The response:

```
typedef struct
{
    uint16_t status_code;

    uint8_t *body;
    size_t body_length;

} ap_http_response_t;
```

is backend-owned.

The caller releases the response body with:

```
ap_http_backend.response_free(
    context,
    &response
);
```

---

# 14. Backend Selection

Backend selection is a build-system responsibility.

Example:

```
Linux build:

    Influx Plugin
          │
          ▼
      http.h
          │
          ▼
      http_curl.c
          │
          ▼
        libcurl
```

ESP32:

```
ESP32 build:

    Influx Plugin
          │
          ▼
      http.h
          │
          ▼
       http_esp.c
          │
          ▼
      ESP HTTP API
```

The plugin source remains unchanged.

---

# 15. API Consistency Rules

When adding a new backend:

1. Define the platform-independent API first.
2. Place the API header below `adapters/api/`.
3. Define opaque backend context handling.
4. Use the standard lifecycle:
   `create → open → use → close → destroy`.
5. Use `ap_result_t` for fallible operations.
6. Define ownership for every pointer and dynamically allocated object.
7. Do not expose platform-specific types.
8. Provide one exported backend instance.
9. Implement platform-specific code only below the API boundary.
10. Do not add platform-selection logic to the plugin.
11. Do not duplicate backend functionality inside plugins.
12. Keep the API contract identical across platform implementations.

---

# 16. API Change Policy

An API change affects all users and implementations of that API.

Before changing an existing backend API:

1. Update the API definition.
2. Update this specification.
3. Update all existing implementations.
4. Update all affected plugins.
5. Build all currently supported targets.

Existing APIs shall not be changed merely to accommodate a single plugin if the requirement can be solved within the plugin or backend implementation.

---

# 17. Reference Pattern

The canonical backend pattern is:

```
static void *backend_context;

static ap_result_t plugin_init(void)
{
    ap_result_t result;

    result =
        backend.create(
            &backend_context
        );

    if (result != AP_OK)
        return result;

    result =
        backend.open(
            backend_context
        );

    if (result != AP_OK)
    {
        backend.destroy(
            backend_context
        );

        backend_context = NULL;

        return result;
    }

    return AP_OK;
}


static void plugin_shutdown(void)
{
    if (backend_context != NULL)
    {
        backend.close(
            backend_context
        );

        backend.destroy(
            backend_context
        );

        backend_context = NULL;
    }
}
```

This pattern is the reference implementation for new reusable backend integrations.

---

# 18. Design Principle

The backend abstraction exists to isolate **platform dependency**, not to hide plugin functionality.

The intended separation is:

```
                    Platform-independent
                    ─────────────────────

             Plugin
                │
                ▼
          Backend API
                │
                ▼

                    Platform-dependent
                    ───────────────────

          Platform Backend
                │
                ▼
       OS / Hardware / Library
```

A plugin should therefore be portable as long as an implementation of the required backend API exists for the target platform.
