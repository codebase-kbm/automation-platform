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


## Adapter Architecture

Adapters connect external systems and platform-specific implementations to the Automation Core.
Adapters are implemented as part of Plugins. 
A Plugin provides the functional module, while an Adapter provides the concrete communication or hardware interface.

Example:

MQTT Plugin
 └── Linux Adapter
     └── libmosquitto

SocketCAN Plugin
 └── Linux Adapter
     └── SocketCAN API
	 
	 
## Communication Adapters

Communication adapters exchange Signals and Events with external systems.

They are responsible for:

Receiving external data
Parsing protocol messages
Creating Signal Events
Transmitting Signal Events
Encoding Core data into protocol formats

Examples:

MQTT
CAN / SocketCAN
Modbus
OpenTherm
TCP / Socket


## Service Adapters

Service adapters observe or export Core data without being the primary communication path.

They consume Events from the Dispatcher and provide external services.

Examples:

InfluxDB
Syslog
Loki
Prometheus


## Adapter Responsibilities

Adapters are responsible for:

Protocol handling
Transport handling
Encoding and decoding
Mapping external data to Core Objects
Mapping Core Events to external representations

Adapters may depend on external libraries or platform APIs.

Examples:

Linux MQTT Adapter → libmosquitto
Config Compiler → libxml2
SocketCAN Adapter → Linux CAN API


## Core Responsibilities

The Automation Core provides platform-independent functionality:

Object registry
Signal management
Event creation
Event dispatching
Timestamp generation
Plugin management
Configuration object handling

The Core does not know about:

MQTT
CAN
External libraries
Operating system APIs
Hardware interfaces

All protocol-specific functionality stays inside Plugins and their Adapters.