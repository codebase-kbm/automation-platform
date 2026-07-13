# Architecture

```
                    Automation Core
                           │
                    Event Dispatcher
                           │
        +------------------+------------------+
        │                                     │
 Communication Adapters             Service Adapters
        │                                     │
 MQTT                                Syslog
 CAN                                 Loki
 Modbus                              InfluxDB
 OpenTherm                           Prometheus
 ISC
```

## Layers

### Automation Core

Responsible for:

- Event dispatching
- Timestamp generation
- Timeout supervision
- Statistics (future)
- Health monitoring (future)

The Core is completely protocol agnostic.

---

### Communication Adapters

Communication adapters translate protocol specific messages into Signal Events and vice versa.

Examples:

- MQTT
- CAN
- Modbus
- OpenTherm
- Crestron ISC

---

### Service Adapters

Service adapters consume events for monitoring, logging or analytics.

Examples:

- Syslog
- Loki
- InfluxDB
- Prometheus

---

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