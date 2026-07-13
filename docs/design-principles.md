# Design Principles

## Event-driven

Everything inside the platform is processed as events.

---

## Protocol Agnostic

The Core never contains protocol-specific logic.

---

## Transport Independent

Protocols should be independent from their transport whenever possible.

---

## Embedded First

The platform shall run on small embedded systems as well as Linux.

---

## Distributed by Design

Nodes may communicate directly.

MQTT is optional, never mandatory.

---

## Configuration over Code

Mappings belong into configuration, not source code.

Future versions will provide a configuration compiler.

---

## Reliability First

The Core provides common services such as timestamps and timeout supervision.

---

## Keep the Core Small

Only functionality required by every adapter belongs into the Core.

Everything else should become an adapter or plugin.