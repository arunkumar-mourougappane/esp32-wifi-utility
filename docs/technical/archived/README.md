# Archived Documentation

This directory contains documentation for features that have been removed or superseded in newer versions.

## RTOS Documentation (v4.1.0)

The following documentation relates to the FreeRTOS-based architecture that was present in v4.1.0 and removed in v4.2.0:

- **[RTOS_ARCHITECTURE.md](../RTOS_ARCHITECTURE.md)** - Complete system design and task structure
- **[RTOS_API_REFERENCE.md](../RTOS_API_REFERENCE.md)** - API documentation for RTOS components
- **[RTOS_MIGRATION_GUIDE.md](../../user-guides/RTOS_MIGRATION_GUIDE.md)** - v3.x to v4.1.0 migration guide
- **[RTOS_TUTORIAL_NEW_TASK.md](../RTOS_TUTORIAL_NEW_TASK.md)** - Tutorial for creating tasks
- **[RTOS_TUTORIAL_QUEUES.md](../RTOS_TUTORIAL_QUEUES.md)** - Tutorial for queue usage
- **[RTOS_TUTORIAL_DEBUGGING.md](../RTOS_TUTORIAL_DEBUGGING.md)** - Debugging guide
- **[RTOS_FAQ.md](../../user-guides/RTOS_FAQ.md)** - Frequently asked questions

## Why Were These Features Removed?

Version 4.2.0 removed the RTOS architecture in favor of a simplified, linear execution model:

### Reasons for Removal
- **Complexity**: RTOS added significant complexity for minimal benefit
- **Maintenance**: Harder to debug and maintain multi-threaded code
- **Code Size**: ~5000 lines of RTOS-specific code removed
- **Build Time**: 33% faster compilation without RTOS
- **Learning Curve**: Easier for contributors to understand

### What Was Retained
- ✅ All user-facing functionality
- ✅ Web server and interface
- ✅ WiFi operations
- ✅ Command interface
- ✅ LED status indicators
- ✅ Network analysis features

### What Changed
- ❌ Task-based architecture
- ❌ Queue communication
- ❌ Mutex synchronization
- ❌ Multi-core distribution
- ❌ Priority scheduling

## Migration to v4.2.0

If you have code that depends on RTOS features, please see:
- **[Migration Guide v4.2.0](../../MIGRATION_GUIDE_V4.2.0.md)** - Complete upgrade instructions

## Historical Reference

This documentation is preserved for:
1. **Historical Context**: Understanding the evolution of the project
2. **Reference**: Useful patterns and techniques
3. **Learning**: RTOS implementation examples
4. **Comparison**: Before/after architectural decisions

---

**Last Active Version:** v4.1.0  
**Archived Date:** October 25, 2025  
**Replacement:** Simplified linear execution model in v4.2.0
