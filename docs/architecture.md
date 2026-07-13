# Automation Platform – Architecture v0.2

## Vision

Automation Platform is a lightweight, protocol-agnostic middleware for building and industrial automation.

Instead of connecting protocols directly (CAN ↔ MQTT ↔ Crestron ↔ Modbus), all communication is routed through a common Signal Model.

The platform focuses on deterministic execution, portability and loose coupling.

---

# Core Principles

## Everything is a Signal

The core only processes logical signals.

Examples:

- livingroom.light
- livingroom.temperature
- garage.door
- heating.setpoint

The core never knows where a signal originates from.

---

## Protocol Agnostic

The core has no knowledge about:

- MQTT
- CAN
- SocketCAN
- Crestron ISC
- Modbus
- Home Assistant
- WebSocket

Protocol handling is implemented entirely inside adapters.

---

## Adapter Based Architecture

```
                +----------------+
                | Applications   |
                +-------+--------+
                        |
                Automation Core
                        |
        +---------------+---------------+
        |               |               |
      MQTT            CAN            ISC
      Adapter        Adapter       Adapter
```

Each adapter translates between its protocol and the common Signal Model.

---

# Compile-Time Configuration

Configuration is **compiled**, not interpreted.

Human-readable configuration:

```
config.yaml
```

↓

Configuration Compiler

↓

```
config.bin
```

↓

Automation Core

The core never parses YAML, JSON or XML.

This keeps the runtime small, deterministic and platform independent.

---

# Binary Configuration

The compiled configuration contains only optimized lookup tables.

Examples:

- Signal IDs
- Topic tables
- CAN mappings
- Join mappings
- Scaling information

No strings are required during runtime except where needed by individual adapters.

---

# Signal IDs

Signals are internally identified by numeric IDs.

Example:

```
livingroom.temperature
```

↓

```
0x00000017
```

Advantages:

- fast lookup
- low RAM usage
- embedded friendly
- deterministic execution

---

# Configuration Compiler

The compiler performs all validation before deployment.

Examples:

- duplicate MQTT topics
- duplicate CAN IDs
- invalid joins
- unknown signals
- invalid mappings

Errors are detected before the runtime starts.

---

# Deployment Philosophy

Deployment replaces configuration.

Workflow:

```
config.yaml

↓

Automation Platform Compiler

↓

config.bin

↓

Deploy

↓

Restart Runtime
```

The runtime starts immediately using the precompiled configuration.

No runtime parsing is performed.

---

# Platform Independence

The core contains no platform-specific code.

Platform implementations provide:

- File access
- Networking
- Timers
- Threads
- Flash storage

Supported platforms:

- Linux
- ESP32

Future:

- STM32
- RP2040
- other embedded targets

---

# Stateless Runtime

The runtime should remain stateless.

Startup sequence:

1. Load binary configuration
2. Verify header and CRC
3. Initialize adapters
4. Start dispatcher

No configuration processing occurs after startup.

---

# Embedded First

The core is designed to run on embedded devices.

Goals:

- low RAM usage
- low flash usage
- deterministic timing
- minimal dynamic allocation

Linux should simply be a larger target running the same core.

---

# Optional Hot Reload

Hot configuration reload is intentionally not required.

Preferred workflow:

Deploy

↓

Restart Runtime

↓

Continue Operation

Future versions may support hot reload without changing the core architecture.

---

# Future Configuration Tool

Long-term the YAML file may be replaced by a graphical configuration editor.

```
Automation Studio

↓

Signal Editor

↓

Mapping Editor

↓

Compile

↓

config.bin
```

The runtime remains unchanged.

---

# Design Philosophy

The core should be as small and deterministic as possible.

Everything "smart" happens before execution:

- configuration
- validation
- optimization
- code generation

The runtime only executes.

---

# Non Goals

Automation Platform is NOT:

- a SCADA system
- a visualization platform
- a Home Assistant replacement
- a Node-RED replacement
- a database
- a historian

Automation Platform is the integration layer between these systems.

---

# Guiding Principle

> The runtime executes.
>
> The compiler thinks.
