## Adapter Responsibilities

Adapters are only responsible for:
- Transport
- Transport handling
- Connection management
- Flow control
- Platform-specific communication
- External library integration
- Hardware interface handling

The Core is responsible for:
- Dispatching
- Timestamp generation

## Adapter Architecture

Adapters connect external systems and platform-specific implementations
to Automation Core Plugins.

A Plugin provides the functional and protocol-specific logic, while an
Adapter provides the concrete communication, transport, and platform
interface required by the Plugins.

Adapters may depend on external libraries or platform APIs.

```
Example:

MQTT Plugin
 └── Adapter (LINUX)
     └── libmosquitto

SocketCAN Plugin
 └── Adapter (LINUX)
     └── SocketCAN
```
	 



