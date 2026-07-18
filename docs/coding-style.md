Automation Platform Core - Coding Style Guidelines
1. General Principles

The Automation Platform Core is designed as a small, deterministic, protocol-independent runtime.

Core code must:

not contain protocol-specific logic
not depend on adapters
communicate through Signals and Events
avoid unnecessary dynamic behavior
prefer explicit behavior over hidden magic
2. Signal IDs
Reserved IDs

Signal ID 0 is always reserved.

0       Invalid / Not Assigned

1-99    Core System Signals

100+    User Space

Rules:

Signal ID 0 must never be registered.
Uninitialized signal IDs should evaluate to invalid.
User-defined signals must start at ID 100.
3. Result Handling

Functions that perform an operation should return ap_result_t.

Examples:

ap_result_t ap_registry_register(const ap_signal_t *signal);

ap_result_t ap_mapper_add(uint32_t source, uint32_t target);

Functions should return only meaningful errors for their module.

Common results:

AP_OK

AP_ERROR_INVALID_ARGUMENT
AP_ERROR_INVALID_ID
AP_ERROR_INVALID_TYPE

AP_ERROR_NOT_FOUND
AP_ERROR_ALREADY_EXISTS
AP_ERROR_FULL

Do not return generic errors if a more specific result exists.

4. Function Categories
Operations

Functions that modify state return ap_result_t.

Examples:

register()
add()
remove()
init()
publish()
Lookups

Functions that retrieve data may return pointers.

Examples:

const ap_signal_t *ap_registry_find(uint32_t id);

Usage:

const ap_signal_t *signal = ap_registry_find(id);

if (signal == NULL)
{
    /* Not found */
}
5. Parameter Validation

Public functions validate parameters immediately.

Preferred pattern:

if (pointer == NULL)
{
    return AP_ERROR_INVALID_ARGUMENT;
}

Signal IDs:

if (id == 0)
{
    return AP_ERROR_INVALID_ID;
}
6. Memory Management

The Core should avoid unnecessary dynamic allocation.

Preferred:

static allocation
caller-provided buffers
fixed-size structures

Dynamic allocation should only be used when required.

7. Naming Convention

Types:

ap_signal_t
ap_event_t
ap_result_t

Functions:

ap_signal_create()

ap_event_init()

ap_registry_register()

Constants:

AP_OK

AP_ERROR_FULL

AP_SIGNAL_FLOAT
8. Logging and Debugging

Debug output must not be implemented using direct printf() calls inside core modules.

Preferred:

Core
 |
 v
Signal/Event
 |
 v
Dispatcher
 |
 v
Logger Adapter

The Core should expose internal information through Signals where possible.

9. System Signals

The Core may expose internal status information through reserved Signals.

Examples:

Core.HeartbeatInterval

Core.State

Core.Uptime

Core.EventsPerSecond

Core.EventsTotal

Core.DroppedEvents

System information follows the same Signal/Event model as user data.

10. Adapter Separation

Adapters are responsible for:

protocol handling
hardware access
conversion
external communication

The Core is responsible for:

signal management
event routing
dispatching
runtime logic

No adapter-specific knowledge may exist inside the Core.