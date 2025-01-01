# ESP32 MCP230XX expander driver/library

This is a single-file header-only C++ style library (see files in `main/include/`) for driving the **MCP230XX** MicroChip's GPIO expander from an **ESP32** microcontroller.

Currently supported are:
- MCP23008
- MCP23009
- MCP23016
- MCP23017
- MCP23018


It is based on another library:
- https://github.com/NSBum/esp32-mcp23008


## Installation & usage
See example in `main/main.cpp`.

- Move the files from `main/include/` to your include directory.
- #include the `MCP230XX.h` in your code.
- Create an object.
- Done!
