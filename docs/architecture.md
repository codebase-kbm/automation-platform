# Architecture

```
Automation Core
       │
       ▼
    Plugins
       │
       ▼
   Backend API
       │
       ▼
    Adapters
       │
       ├── Platform
       ├── Library
       ├── Transport
       └── Hardware


The Automation Platform is built around a platform-independent Automation Core.
- Plugins extend the Core with functional modules.
- Adapters provide the concrete transport, platform or hardware-specific implementation required by a Plugin.
```

## Layers

### Automation Core

The Core provides the platform-independent runtime.

Responsible for:

- Object registry
- Signal management
- Event creation
- Event dispatching
- Timestamp generation
- Plugin management
- Configuration object handling
- Statistics (future)
- Health monitoring (future)

The Core is completely protocol and platform agnostic.

The Core does not depend on external communication libraries or
operating-system APIs.

---

### Plugins

Plugins provide functional modules for the Automation Platform.

A Plugin typically contains:

- Plugin configuration
- Configuration loading
- Object and Signal mappings
- Plugin lifecycle
- Protocol-specific logic
- Encoding and decoding
- Conversion between external data and Core Objects/Events

Plugins use Backend APIs to access platform-specific functionality.

Examples:

- MQTT Plugin
- CAN Plugin
- Peer Plugin
- Timeout Plugin

Plugins may remain platform independent while using platform-specific
Adapters through their Backend APIs.

---

### Adapters

Adapters provide the concrete implementation required by a Plugin Backend API.

They are responsible for:

- Transport handling
- Connection management
- Flow control
- Platform-specific communication
- Hardware interfaces
- External library integration

Adapters may depend on external libraries or platform APIs.

For example:

```
MQTT Plugin
    │
    └── Backend API
            │
            └── Linux Adapter
                    │
                    └── libmosquitto

or:


CAN Plugin
    │
    └── Backend API
            │
            └── Linux Adapter
                    │
                    └── Linux SocketCAN
```

### Transport vs Protocol

The Automation Platform separates transport and protocol whenever
possible.

Example:
```
TCP
│
▼
Peer / Application Protocol
│
▼
Plugin
│
▼
Core Objects / Events

or:

SocketCAN
│
▼
CAN Plugin
│
▼
Core Objects / Events
```

The transport and platform-specific implementation remain outside the Core.

Protocol interpretation, encoding, decoding and mapping remain inside
the Plugin.

### Dependency Boundaries

External dependencies belong to the layer that requires them.

Examples:
```
Automation Core
    └── no external protocol dependencies

MQTT Plugin
    └── Backend API
            └── Linux Adapter
                    └── libmosquitto

Config Compiler
    └── libxml2
```
This keeps the Core portable and allows platform-specific implementations
to be replaced without changing the Core architecture.