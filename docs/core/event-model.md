# Event Model

Automation Platform distinguishes between Signal Definitions and Signal Events.

## Signal Definition

A Signal Definition describes a logical data point.

Example:

- Outside Temperature
- Boiler State
- Relay Output
- Fan Speed

A definition contains metadata such as:

- Identifier
- Name
- Data Type
- Timeout
- Unit (future)
- History (future)

---

## Signal Event

A Signal Event represents a value update.

Typical metadata:

- Signal ID
- Timestamp
- Source Node
- Source Adapter
- Sequence Number
- Quality
- Flags
- Value

Events are immutable once published.

---

## Timeout Supervision

Signals may optionally define a timeout.

If no update is received within the configured period, the Core generates a Timeout Event.

Timeout handling is protocol independent.