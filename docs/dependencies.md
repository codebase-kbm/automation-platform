Dependencies
============

Core
----
No external dependencies.
The automation core is platform independent and only provides:
- event handling
- object registry
- dispatcher
- plugin management
- configuration object handling


Plugins
-------
Plugins may introduce platform or library dependencies.

MQTT Plugin
- Core part: no external dependencies
- Linux adapter: libmosquitto


Config Compiler
---------------
The config compiler is a build-time tool.

Dependencies:
- libxml2

Input:
- config/config.xml

Output:
- build/config.bin