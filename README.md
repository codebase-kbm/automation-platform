# Automation Platform

A protocol-agnostic automation middleware that unifies heterogeneous
building and industrial automation systems through a common,
event-driven Signal Model.

The Automation Platform decouples hardware, protocols, transports and
applications by introducing a lightweight Automation Core with a unified
event model.

---

## Vision

Automation Platform is **not** another PLC, SCADA system or home automation platform.

Instead, it provides a common communication layer that allows independent systems to exchange information through a unified event model.

The Core connects systems — it does not replace them.

---

## 🚀 Concept

Instead of creating direct integrations:

    CAN ↔ MQTT ↔ Crestron ↔ Modbus ↔ Home Assistant

external protocols are translated into a common representation by Plugins.

```
    External System
          │
          ▼
       Adapter
          │
          ▼
        Plugin
          │
          ▼
   Automation Core
          │
          ▼
        Plugin
          │
          ▼
       Adapter
          │
          ▼
    External System
```

The Core only works with platform-independent Objects and Events.

Plugins contain protocol-specific logic, including mapping, encoding
and decoding.

Adapters provide the concrete transport, platform, hardware or external
library implementation required by a Plugin.

## Features

- Event-driven architecture
- Protocol independent Core
- Platform independent
- Distributed by design
- Configuration-driven mappings
- Timestamp support
- Signal timeout supervision
- Extensible Plugin architecture
- Platform-specific Adapter architecture

## Planned Plugins

### Communication

- MQTT
- CAN
- OpenTherm
- DMX / Art-Net
- Crestron ISC
- Peer

### Services

- Syslog
- Loki
- InfluxDB

## Planned Adapters

Adapters provide platform- and technology-specific implementations for
Plugin Backend APIs.

Examples:

- Linux / SocketCAN
- Linux / libmosquitto
- Linux / TCP
- ESP32 / network stack
- ESP32 / CAN

## Project Status

Early development.

The current focus is building a lightweight, reusable Automation Core
and establishing the Plugin and Adapter architecture.

## Current Status

The minimal Automation Core is operational.

Implemented components:

- Signal model
- Event model
- Event dispatcher
- Minimal example application