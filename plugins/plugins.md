## Plugins

Plugins observe, consume, or export Core data without being part of
the Core itself.

They consume Events from the Dispatcher and provide external services or
communication interfaces.

Examples:

- System
- MQTT
- CAN
- InfluxDB
- Peer
- Syslog
- ...

## Plugin Responsibilities

Plugins are responsible for protocol-specific functionality at the
Core boundary:

- Encoding and decoding external data
- Mapping external data to Core Objects
- Mapping Core Events to external representations
- Handling plugin-specific configuration
- Managing protocol-specific communication semantics

Plugins must remain platform-independent.

## Adapter Responsibilities

Adapters provide the platform- or technology-specific implementation
required by a Plugin.

They are responsible for:

- Operating system APIs
- Hardware interfaces
- External libraries
- Platform-specific resources
- Establishing and managing the underlying connection

Adapters must not contain plugin-specific mapping or Core configuration
logic.

The Plugin defines **what** it needs from an Adapter; the Adapter defines
**how** this is implemented on the target platform.

## Core Responsibilities

The Automation Core provides platform-independent functionality:

- Object registry
- Signal management
- Event creation
- Event dispatching
- Timestamp generation
- Plugin management
- Configuration object handling

The Core does not know about:

- MQTT
- CAN
- External libraries
- Operating system APIs
- Hardware interfaces

All protocol-specific functionality stays inside Plugins for mapping,
encoding, decoding, and protocol semantics.

Platform- and hardware-specific functionality is implemented by Adapters.