# Functionality Roadmap

## Core

- [x] Automation Core
- [x] Event / Signal Model
- [x] Dispatcher
- [x] Registry
- [x] Timestamp Support
- [x] Plugin Manager
- [x] Linux Build System

---

## Configuration

- [x] XML Configuration Base
- [x] Configuration Compiler
- [x] Config Dump Tool
- [x] Structural XML for easy editing with XML Editor Tools
- [ ] Config Compiler: provide detailed error locations from plugin compilers
  - [ ] Report the XML line using xmlGetLineNo() on the actual error node
  - [ ] Provide a specific error message for each plugin
  - [ ] Keep the central compiler responsible only for propagating the failure
  - [ ] More tolerance about empty Settings if not required
- [ ] Configuration Dokumentation

---

## Buildsystem

- [ ] Kconfig / `make menuconfig` Build Configuration
  - [ ] Target Platform selection
  - [ ] Plugin selection
  - [ ] Adapter selection
  - [ ] Build options
  - [ ] Saved configurations / presets
  
---

## Core - Plugins

### Core-Demo
- [ ] Signal Handling
- [ ] Minimal Core Demo Plugin + Example Application
- [ ] Test Event Generation
- [ ] Signal Mapping
- [ ] Console Event Output

### MQTT
- [x] MQTT Backend
- [x] Signal Handling
- [x] Mapping
- [x] Configuration Integration
- [ ] Configuration Dokumentation

### CAN Plugin
- [x] CAN Backend
- [x] Configuration Integration
- [ ] Mapping
 - [x] RX Mapping with LE / Bitdecoding
 - [x] RX Mapping with BE
 - [ ] TX Mapping
 - [ ] TX Bool Trigger by Object
 - [ ] TX Frame Trigger by Object
- [ ] Configuration Dokumentation


### Influx DB Plugin
- [x] HTTP Backend
- [x] Signal Handling
- [x] Mapping
- [x] Configuration Integration
- [ ] Configuration Dokumentation

### Peer Server Plugin
- [x] TCP Backend
- [x] Multi-Connection Handling
- [x] Connection Objects
- [x] Peer Registry
- [x] Peer Protocol
- [ ] Peer CONNECT / ACCEPT
- [ ] Peer REGISTER / UNREGISTER
- [ ] Peer EVENT → AP Dispatcher
- [ ] AP Event → Peer Client
- [x] Configuration Integration
- [ ] Configuration Dokumentation

### Peer Client Plugin
- [x] TCP Backend
- [ ] Multi-Instance Handling
- [ ] Signal Handling/Mapping
- [ ] Configuration Integration
- [ ] Configuration Dokumentation

### Timeout Plugin
- [ ] Signal Handling
- [ ] Event Generation on Timeout
- [ ] Mapping for Events
- [ ] Configuration Integration
- [ ] Configuration Dokumentation

### System Monitor Plugin
- [ ] AP-Core API (read only)
- [ ] Signal Handling
- [ ] Mapping for Events
- [ ] Configuration Integration
- [ ] Configuration Dokumentation

### Crestron ISC Plugin
- [ ] Backend TCP
- [ ] Backend SERIAL
- [ ] Signal Handling
- [ ] Mapping
- [ ] Configuration Integration
- [ ] Configuration Dokumentation

### Syslog Plugin
- [ ] Backend
- [ ] Signal Handling
- [ ] Mapping
- [ ] Configuration Integration
- [ ] Configuration Dokumentation

---
  
## Platform / Runtime / Tools
- [x] ARM Testport (continuous Test for Long-Time-Stability)
- [ ] Peer Client Library "libap-peer" (C-API)
- [ ] libap-peer Wrappers
- [ ] Logger Monitoring Tool
 - [ ] ap-monitor Minimal Client
- [ ] Visual Configuration Tool
- [ ] Embedded Port

 
---
 
## Ideas for Future Development
- REST API
- Web UI
- Distributed Nodes / Remote Core
 