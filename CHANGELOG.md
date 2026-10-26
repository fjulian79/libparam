# Changelog

All notable changes to this project will be documented in this file.
The format is based on Keep a Changelog.

## [v1.2.0] - UNRELEASED
### Changed
- Removed the dependency to libgeneric, the CRC is calculated by libparam itself. The algorithm is unchanged, stored data stays valid.
- The library is declared for ESP32 and Raspberry Pi Pico.
- `write()` and `discard()` return `false` if writing to the flash failed.
### Fixed
- Fixed the parameters not being stored on the Raspberry Pi Pico.
- Fixed parameters at a start address other than zero not being stored on ESP32 and Raspberry Pi Pico.
- Fixed start addresses above 255 being truncated.
- Fixed the format of the debug output.
### Added
- Added this changelog and documented the API in the README.

## [v1.1.0] - 2022-03-11
First tagged version, supporting ESP32.
