# ADR-0002

## Title

Signal Definition and Signal Event Separation

## Status

Accepted

## Decision

Signal Definitions describe logical data points.

Signal Events represent runtime value changes.

## Consequences

Metadata can evolve independently from runtime values.

Core services such as timeout supervision operate on Signal Definitions.