# Changelog

All notable changes to this project will be documented in this file.

## [2.0.0] - 2024-01-15

### Added
- Complete RTOS support for ESP32
- Event queue system for key events
- Hold and long-press detection
- Automatic board detection
- Multi-platform support (ESP32, ESP8266, AVR, ARM, etc.)
- Comprehensive example suite
- Thread-safe operations with mutexes
- Statistics tracking for RTOS

### Changed
- Rewritten from ground up with modern C++ practices
- Improved debouncing algorithm
- Better timing defaults per board type
- Enhanced error handling

### Fixed
- Debounce timing issues
- Millis overflow handling
- RTOS task cleanup
- Event queue overflow protection

## [1.0.0] - 2023-01-01

### Initial Release
- Basic TTP229 support
- Simple key reading
- Basic examples