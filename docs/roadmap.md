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

### CAN Plugin
- [x] CAN Backend
- [x] Configuration Integration
- [ ] Mapping

### Influx DB Plugin
- [ ] HTTP Backend
- [ ] Signal Handling
- [ ] Mapping
- [ ] Configuration Integration

### Peer Server Plugin
- [ ] Peer Backend
- [ ] Multi-Instance Handling
- [ ] Signal Handling
- [ ] Configuration Integration

### Peer Client Plugin
- [ ] Peer Backend
- [ ] Multi-Instance Handling
- [ ] Signal Handling
- [ ] Configuration Integration

### Timeout Plugin
- [ ] Signal Handling
- [ ] Event Generation on Timeout
- [ ] Mapping
- [ ] Configuration Integration

### System Monitor Plugin
- [ ] AP-Core API (read only)
- [ ] Signal Handling
- [ ] Mapping
- [ ] Configuration Integration

### Crestron ISC Plugin
- [ ] Backend
- [ ] Signal Handling
- [ ] Mapping
- [ ] Configuration Integration

### Syslog Plugin
- [ ] Backend
- [ ] Signal Handling
- [ ] Mapping
- [ ] Configuration Integration

---
  
## Platform / Runtime / Tools
- [ ] ARM Testport
- [ ] Peer Client Library (C-API)
- [ ] Peer Client Library Wrappers
- [ ] Logger Monitoring Tool
- [ ] Visual Configuration Tool
- [ ] Embedded Port

 
---
 
## Ideas for Future Development
- REST API
- Web UI
- Distributed Nodes / Remote Core
 