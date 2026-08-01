# Architecture

```
    Automation Core
                              │
                         Event Dispatcher
                              │
                    +---------+---------+
                    │                   │
                  Plugins            Plugins
                    │                   │
          Communication              Service
                    │                   │
                 Plugin               Plugin
                    │
                Adapter
                    │
          +---------+---------+
          │         │         │
       Protocol   Transport  Platform

The Automation Platform is built around a platform-independent Automation Core.

Plugins extend the Core with functional modules. Adapters provide the concrete protocol, transport or platform-specific implementation required by a Plugin.
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

The Core does not depend on external communication libraries or operating-system APIs.

---

### Plugins

Plugins provide functional modules for the Automation Platform.

A Plugin typically contains:

- Plugin configuration
- Configuration loading
- Object and Signal mappings
- Plugin lifecycle
- One or more adapters

Examples:

- MQTT Plugin
- SocketCAN Plugin
- Socket Plugin
- Logger Plugin
- Timeout Plugin

Plugins may be platform independent while providing platform-specific adapters.

---

### Adapters

Adapters implement the concrete external interface used by a Plugin.

They are responsible for:

- Protocol handling
- Transport handling
- Encoding and decoding
- Mapping external data to Core Objects
- Converting external messages into Signal Events
- Transmitting Core Events to external systems

Adapters may depend on external libraries or platform APIs.

For example:

MQTT Plugin
    │
    └── Linux Adapter
            │
            └── libmosquitto

or:

SocketCAN Plugin
    │
    └── Linux Adapter
            │
            └── Linux SocketCAN API

### Communication Plugins

Communication Plugins exchange data between the Automation Core and external systems.

Examples:

- MQTT
- CAN / SocketCAN
- Modbus
- OpenTherm
- Crestron ISC
- TCP / Socket

The communication protocol itself remains outside the Core.

### Service Plugins

Service Plugins consume Core Events for monitoring, logging, storage or analytics.

Examples:

- Syslog
- InfluxDB
- Logger

A Service Plugin does not necessarily provide a bidirectional communication channel.


### Transport vs Protocol

Automation Platform separates transport and protocol whenever possible.

Example:

```
TCP
 │
 ▼
ISC Protocol
 │
 ▼
Signal Events
```

or

```
SocketCAN
 │
 ▼
CAN Frame Decoder
 │
 ▼
Signal Events
```

The Core only sees the resulting Objects and Events.

### Dependency Boundaries
```
External dependencies belong to the layer that requires them.

Examples:

Automation Core
    └── no external protocol dependencies

MQTT Plugin
    └── Linux Adapter
            └── libmosquitto

Config Compiler
    └── libxml2
```
This keeps the Core portable and allows platform-specific implementations to be replaced without changing the Core architecture.