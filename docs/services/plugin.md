# Plugin Service

**⚠️ NOT IMPLEMENTED** - Plugin loading system interface does not exist in current implementation.

## Status

The Plugin Service interface (`IPluginService`) is documented in this framework but:

- No header file exists (IPluginService.h not found)
- No implementation available
- System uses static plugin linking only
- Interface NOT exposed through ICore

## Documentation Purpose

This document serves as a reference for the planned plugin architecture.
Current system does NOT support dynamic plugin loading or inter-plugin communication.

## Notes

- Plugin system is planned but not yet implemented
- Plugins are currently statically linked at build time
- Dynamic loading would require significant architectural changes
- Interface defined for future extensibility

## See Also

- [Architecture](../../docs/architecture.md) - System design
- [Core Service](../../include/SDK/PaymentProcessor/Core/ICore.h) - Available interfaces
