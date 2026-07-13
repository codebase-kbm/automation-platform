# ADR-0003

## Title

Common Core Services

## Status

Accepted

## Decision

Common functionality required by all adapters belongs into the Core.

Initial services:

- Event Dispatcher
- Timestamp Management
- Timeout Supervision

Future services:

- Statistics
- Health Monitoring

## Consequences

Adapters remain lightweight and protocol focused.