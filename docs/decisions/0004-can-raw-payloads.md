# ADR-0004

## Title
CAN Payload Handling

## Status
Accepted

## Decision

`AP_VALUE_BUFFER` is not used for CAN frame transport.

CAN frames are kept in plugin-local runtime state and are
decomposed into existing AP signal value types through mappings.

For TX, frame transmission is triggered by an explicitly
configured trigger object.

The Core therefore remains independent of raw CAN frame buffers.