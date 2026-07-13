# Adapter Architecture

Adapters connect external systems to the Automation Core.

## Communication Adapters

Receive data from external systems and publish Signal Events.

They also consume Signal Events and transmit them back to external systems.

Examples:

- MQTT
- CAN
- Modbus
- OpenTherm

---

## Service Adapters

Observe Signal Events without changing communication.

Examples:

- Syslog
- InfluxDB
- Loki
- Prometheus

---

## Adapter Responsibilities

Adapters are responsible for:

- Protocol parsing
- Encoding
- Decoding
- Transport

The Core is responsible for:

- Dispatching
- Timestamp generation
- Timeout supervision