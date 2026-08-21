# ADR-0004

## Title
Peer Protocol Result Handling

## Status
Accepted

## Decision

Peer protocol requests return a result response.

For operations affecting multiple Object IDs, processing is performed independently for each requested Object.

If all requested Objects are processed successfully, the server returns:
´´´
AP_OK
´´´
If one or more Objects fail, the server returns an appropriate error result together with the list of failed Object IDs.

Successfully processed Object IDs are not returned.

Partial success does not cause a rollback.

Example
´´´
REGISTER [98, 99, 100, 104]

If Object 100 does not exist:

RESULT
    code: AP_ERROR_NOT_FOUND
    failed:
        100

The resulting runtime registration is:

98
99
104

Object 100 is not registered.
´´´
# Event Handling

Events are transmitted one-way.

The receiver does not return an ap_result_t for every received Event.

Transport or connection errors are handled through the connection itself and its associated Connection Object status.

# Rationale

The protocol remains lightweight and allows efficient implementation of the future Peer Client Library.

The client already knows which Object IDs it requested. Therefore, only failed Object IDs need to be reported by the server.

Partial processing avoids unnecessary rollback logic and allows valid registrations to remain active even when individual Object IDs are invalid.