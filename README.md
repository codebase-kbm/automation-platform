# Automation Platform

A protocol-agnostic automation middleware that unifies heterogeneous building and industrial systems into a single Signal-based architecture.

The Automation Platform decouples hardware, protocols, and applications by introducing a central Signal Model and an event-driven core.

---

## 🚀 Concept

Instead of integrating systems directly:

CAN ↔ MQTT ↔ Crestron ↔ Modbus ↔ Home Assistant

everything is normalized into a unified abstraction:

**Signal → Core → Adapter**

The core does not know any protocol details.  
It only processes signals and routes events between adapters.

---

## 🧠 Core Principles

- Everything is a Signal
- No protocol knowledge inside the core
- Fully event-driven architecture
- Loose coupling via Adapters
- Configuration-driven mappings (no hardcoding)
- Platform independent (Linux, ESP32, etc.)
