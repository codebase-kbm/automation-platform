# ADR-0001

## Title

Protocol Agnostic Core

## Status

Accepted

## Decision

The Automation Core shall never contain protocol-specific logic.

All protocol implementations shall exist as adapters.

## Consequences

- Simple Core
- Easy testing
- Independent protocol development
- Proprietary adapters can remain outside the main repository