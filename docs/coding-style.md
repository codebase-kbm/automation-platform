# Automation Platform Core - Coding Style & Architecture Guidelines

## 1. General Principles

The Automation Platform is designed as a small, modular and protocol-independent runtime.

Core code should:

* remain platform-independent
* remain protocol-independent
* communicate through Signals and Events
* avoid unnecessary dynamic behavior
* prefer explicit behavior over hidden magic
* keep system-wide definitions centralized
* avoid duplicating functionality across modules and plugins

The Core must not contain knowledge about specific protocols, hardware, transports or external services.

---

## 2. Signal Model

Signals are the common data model used throughout the Automation Platform.

Signals must be usable independently of their originating protocol or backend.

Signal IDs are globally unique within a running Automation Platform instance.

Signal ID `0` represents an invalid / unassigned ID and must not be registered.

There are no globally reserved ranges for Core, Plugin or User Signals.

System and Plugin Signals use the same Signal model as application data.

---

## 3. Result Handling

Functions that perform an operation should return `ap_result_t`.

Examples:

```c
ap_result_t ap_registry_register(const ap_signal_t *signal);
ap_result_t ap_mapper_add(uint32_t source, uint32_t target);
```

Functions should return meaningful errors for their respective module.

Common results include:

```text
AP_OK
AP_ERROR_INVALID_ARGUMENT
AP_ERROR_INVALID_ID
AP_ERROR_INVALID_TYPE
AP_ERROR_NOT_FOUND
AP_ERROR_ALREADY_EXISTS
AP_ERROR_FULL
```

Do not return a generic error if a more specific result exists.

### Operations

Functions that modify state normally return `ap_result_t`.

Examples:

```text
register()
add()
remove()
init()
publish()
```

### Lookups

Functions that retrieve objects may return pointers.

Example:

```c
const ap_signal_t *ap_registry_find(uint32_t id);
```

Usage:

```c
const ap_signal_t *signal = ap_registry_find(id);

if (signal == NULL) {
    /* Not found */
}
```

---

## 4. Parameter Validation

Public functions validate parameters immediately.

Preferred pattern:

```c
if (pointer == NULL) {
    return AP_ERROR_INVALID_ARGUMENT;
}
```

Signal IDs should be validated where required:

```c
if (id == 0) {
    return AP_ERROR_INVALID_ID;
}
```

Invalid input should be rejected as early as possible.

---

## 5. Memory Management

The Core should avoid unnecessary dynamic allocation.

Preferred approaches include:

* static allocation
* caller-provided buffers
* fixed-size structures
* bounded queues and buffers

Dynamic allocation may be used where required by the implementation, but should not be introduced without a reason.

This is particularly important for portability to embedded platforms.

---

## 6. Naming Convention

### Types

```text
ap_signal_t
ap_event_t
ap_result_t
```

### Functions

```text
ap_signal_create()
ap_event_init()
ap_registry_register()
```

### Constants

```text
AP_OK
AP_ERROR_FULL
AP_SIGNAL_FLOAT
```

Names should be descriptive and consistent with the existing module API.

---

## 7. Core / Plugin / Backend Separation

The architecture separates platform-independent functionality from platform-specific implementations.

### Core

The Core is responsible for:

* Signal management
* Event handling
* Event dispatching
* Routing
* Runtime logic
* Core-level supervision

The Core must not contain:

* protocol-specific logic
* hardware-specific logic
* backend-specific APIs
* external service handling

### Plugins

Plugins provide protocol-, service- or function-specific behavior.

A plugin may contain:

* plugin configuration structures
* mapping definitions
* protocol-independent plugin logic
* abstract backend APIs

The central plugin logic acts as the translator between the Automation Platform Core and the backend.

### Backends

Concrete backends implement platform- or library-specific functionality.

Examples include:

```text
SocketCAN
MQTT / libmosquitto
Linux sockets
ESP32 CAN
Serial interfaces
```

The backend-specific implementation must remain behind the plugin's abstract backend API.

The architecture should therefore follow:

```text
Automation Core
       │
       ▼
    Plugin
       │
       ▼
 Abstract Backend API
       │
       ├── Linux Backend
       ├── ESP32 Backend
       └── Other Platform Backend
```

The same plugin logic should be reusable with different backend implementations whenever practical.

---

## 8. Mapping

Mapping translates signals between sources and destinations.

Mapping is not protocol-specific Core functionality.

The Core provides the mechanisms required for signal routing and event dispatching.

Protocol- or plugin-specific mapping and conversion logic belongs to the respective plugin or module.

The Core should not need to know whether a signal originated from MQTT, CAN, Socket, GPIO or another source.

---

## 9. Configuration

Configuration is separated from runtime execution.

Configuration files are compiled into the runtime representation where appropriate.

The runtime Core should not depend on XML parsing or other configuration-file formats.

Configuration-specific parsing and compilation belongs outside the runtime Core.

Plugins define their configuration requirements, while the configuration system provides the common mechanism for loading and validating them.

---

## 10. Logging and Monitoring

Core modules must not use direct `printf()` calls for application logging.

Logging and monitoring should be provided through external components, plugins or monitoring tools.

The Core may expose runtime information through the normal Signal/Event model.

Examples include:

```text
Core.State
Core.Uptime
Core.EventsPerSecond
Core.EventsTotal
Core.DroppedEvents
```

These are examples only and do not constitute globally reserved Signal IDs.

Internal runtime information should use the same Signal/Event mechanisms as normal application data whenever practical.

---

## 11. Plugin Registration

Plugins are managed through the central Plugin Manager.

Plugin registration should remain centralized and consistent.

Adding or removing a plugin should not require unrelated Core modules to contain individual registration logic or duplicated definitions.

Plugin-specific initialization, configuration and backend setup belongs to the plugin itself.

---

## 12. Platform Independence

Platform-independent code must not directly depend on platform-specific APIs.

Platform-specific functionality belongs in backend implementations.

For example:

```text
Platform-independent:
    Plugin
    Configuration
    Mapping
    Backend API

Platform-specific:
    SocketCAN
    Linux sockets
    libmosquitto
    ESP32 CAN driver
    Hardware GPIO
```

This separation allows the same functional module to be reused across Linux, ARM and embedded targets.

---

## 13. Error and Failure Handling

Errors should be handled explicitly.

Functions should:

* validate input
* return meaningful `ap_result_t` values where applicable
* avoid silently ignoring failures
* avoid hiding backend errors
* leave objects in a well-defined state after failed operations

Failure handling should be deterministic and predictable where practical.

---

## 14. Design Rule

The most important architectural rule is:

> **The Core provides the common runtime model. Plugins provide functionality. Backends provide platform-specific implementation.**

Protocol-specific or hardware-specific knowledge must not leak into the Core.

New functionality should therefore be evaluated by asking:

1. Does this belong to the common runtime model?
2. Does this belong to a Plugin?
3. Does this belong to a concrete Backend?
4. Can an existing interface be reused instead of adding another parallel mechanism?

System-wide definitions and interfaces should remain centralized rather than being duplicated across modules.
