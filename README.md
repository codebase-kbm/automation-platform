# Automation Platform

A protocol-agnostic automation middleware that unifies heterogeneous building and industrial automation systems through a common Event-driven Signal Model.

The Automation Platform decouples hardware, protocols, transports and applications by introducing a lightweight Automation Core with a unified event model.

---

## Vision

Automation Platform is **not** another PLC, SCADA system or home automation platform.

Instead, it provides a common communication layer that allows independent systems to exchange information through a unified event model.

The Core connects systems — it does not replace them.

---

## 🚀 Concept

Instead of creating direct integrations

CAN ↔ MQTT ↔ Crestron ↔ Modbus ↔ Home Assistant

every protocol is translated into a common representation.

```
Signal Definition
        │
        ▼
     Signal Event
        │
        ▼
 Automation Core
        │
        ▼
     Adapters
```

The Core never knows protocol details.

Protocols only exist inside adapters.

---

## Features

- Event-driven architecture
- Protocol independent
- Platform independent (Linux, ESP32, ...)
- Distributed by design
- Configuration-driven mappings
- Timestamp support
- Signal timeout supervision
- Extensible adapter architecture

---

## Planned Adapters

### Communication

- MQTT
- CAN / SocketCAN
- Modbus
- OpenTherm
- DMX / Art-Net
- Crestron ISC (optional)

### Service

- Syslog
- Loki
- InfluxDB
- Prometheus

---

## Project Status

Early development.

The current focus is building a lightweight, reusable Automation Core.