# Copilot Instructions for InsuBox APS Firmware

## Project Overview
This is embedded firmware for an Artificial Pancreas System (APS) running on ESP32-S3 (Zephyr RTOS). The firmware uses an event-driven, service-oriented architecture. It can also run on `native_sim` for local development and testing 

## Architecture Fundamentals

### Service Layer Pattern
All core functionality is organized into autonomous services instantiated in `app/src/main.cpp`:
- **ControlService**: Closed-loop APS control logic
- **PumpService**: Manages insulin pump communication (wraps `IPumpDevice`)
- **HmiService**: Human-machine interface (display, keypad — wraps `IHmiDevice`)
- **BLEComm**: Handles BLE connectivity (static init, not a service class)

Services communicate exclusively through `EventDispatcher` - direct service-to-service calls are prohibited.

### Event-Driven Communication
The `EventDispatcher` (see `include/events/EventDispatcher.h`) is the central coordination mechanism. Events are plain structs defined in the relevant module headers (e.g., `BtBluetoothStateChanged`, `BtPassKeyConfirmRequest` in `include/ble/BLEComm.h`). Type identity uses a compile-time `EventID<T>` template — no separate `Events.h` or `EventType` enum exists.

```cpp
// Services subscribe to events they care about
mDispatcher.subscribe<BtBluetoothStateChanged>([&](const auto &event) {
    // Handle event
});

// Services publish events for others
mDispatcher.dispatch(BtPassKeyConfirmResponse{conn, true});
```

## Build System (Zephyr + West)

### Standard Build Commands
```bash
# Build for InsuBox hardware (ESP32-S3)
west build -b insubox_rev_zero/esp32s3/procpu --sysbuild app -DOVERLAY_CONFIG="hw_insubox.conf"

# Build for native simulator (local development)
west build -d build_native -b native_sim/native/64 app -DOVERLAY_CONFIG="boards/native_sim.conf"

# Flash to InsuBox hardware
west flash --esp-device=/dev/ttyACM0
west espressif monitor -p /dev/ttyACM0

# Convenience scripts
./run_insubox.sh      # build + flash + monitor
./run_native.sh       # build + run on native_sim
```

### Configuration Layers
- `app/prj.conf`: Base configuration (BLE, NVS, C++17, module enables)
- `app/hw_insubox.conf`: Hardware overlay for InsuBox board (enables insubox pump/HMI drivers)
- `app/boards/native_sim.conf`: Native simulator config (IDS pump)
- `app/boards/native_sim_kaleido.conf`: Kaleido pump simulator config

### Board
Custom board in `boards/insubox/insubox_rev_zero/`: ESP32-S3 dual-core SoC. Target: `insubox_rev_zero/esp32s3/procpu`.

### SDK Version
Vanilla Zephyr v4.4.0 (specified in `west.yml`).

## Testing Framework

### Unit Tests (Google Test + Zephyr Twister)
```bash
# Run all unit tests
west twister -T tests --integration --clobber-output -x=USE_CCACHE=1

# Run specific module tests
west twister -T tests/lib/<module_name> --integration --clobber-output -x=USE_CCACHE=1
```

Tests are in `tests/lib/<module>/`. Each test uses:
- **GoogleTest/GMock**: Framework (linked via `target_link_libraries(app PUBLIC gtest_main gmock_main)`)
- **Mock classes**: Located in `tests/lib/<module>/mocks/` (e.g., mock HMI device in `tests/lib/hmi/mocks/`)

Test pattern example from `tests/lib/hmi/tst_HmiService.cpp`:
```cpp
EventDispatcher mEventDispatcher;
MockHmiDevice mMockHmiDevice;
HmiService mHmiService{mEventDispatcher, mMockHmiDevice};
```

## Coding Standards (Safety-Critical)

1. **Bounded loops**: Every loop must have statically provable upper bound
2. **No dynamic allocation**: After initialization, no `malloc`/`new` (heap monitored via `CONFIG_SYS_HEAP_RUNTIME_STATS`)
3. **Function length**: Max 60 lines per function
4. **Assertions**: Min 2 assertions per function; must check anomalous conditions and return error codes
5. **Scope minimization**: Declare variables at smallest scope
6. **Return value checking**: Always check non-void return values and validate function parameters
7. **Preprocessor limits**: Only `#include` and simple macros; no token pasting, variadic macros, or recursive macros
8. **Pointer restrictions**: Maximum one level of dereference; no hidden dereferences in macros/typedefs

### Naming Conventions
- `lowerCamelCase`: Variables and functions
- `UpperCamelCase`: Structs and classes
- `MACRO_CASE`: Macros and constants (including `constexpr`)

Example:
```cpp
constexpr size_t MAX_BUFFER_SIZE = 256;  // MACRO_CASE
class PumpService { ... };               // UpperCamelCase
void startBolus() { ... }               // lowerCamelCase
```

## External Dependencies

### Zephyr
- Version: v4.4.0 (vanilla, from GitHub `zephyrproject-rtos/zephyr`, specified in `west.yml`)
- Update: Modify `west.yml` revision → `west update` → verify tests

### GoogleTest
- Pulled directly from GitHub (`google/googletest`, `main` branch) via `west.yml`

## Common Workflows

### Adding a New Service
1. Create header in `include/<module>/` and implementation in `lib/<module>/`
2. Add `CMakeLists.txt` with `CONFIG_IB_<MODULE>` guard
3. Update `lib/CMakeLists.txt`: `add_subdirectory_ifdef(CONFIG_IB_<MODULE> <module>)`
4. Add `CONFIG_IB_<MODULE>=y` to `app/prj.conf`
5. Instantiate in `app/src/main.cpp` with `EventDispatcher` reference
6. Subscribe to relevant events in constructor/init
7. Create unit tests in `tests/lib/<module>/`

### Memory Analysis
- **Stack**: `CONFIG_THREAD_ANALYZER=y` is enabled in `prj.conf` — prints periodic usage
- **Heap**: `CONFIG_SYS_HEAP_RUNTIME_STATS=y` enabled in `prj.conf`

### Debugging
- **Serial monitor**: `west espressif monitor -p /dev/ttyACM0` (or `./run_insubox.sh`)
- **GDB**: `west debug` or via VS Code
- **Native sim**: run locally with `./run_native.sh`; supports host BLE via `hci0`

## Project-Specific Patterns

### Kconfig-Based Modularity
All libraries use `CONFIG_IB_*` Kconfig guards (see `lib/CMakeLists.txt`):
```cmake
add_subdirectory_ifdef(CONFIG_IB_PUMP pump)
```
Hardware-specific implementations are selected via additional Kconfig options (e.g., `CONFIG_IB_PUMP_INSUBOX=y` in `hw_insubox.conf`).

### Device Abstraction
Services take interface references (`IPumpDevice`, `IHmiDevice`, `IControlDevice`) allowing hardware and virtual (simulator) implementations to be swapped via Kconfig:
```cpp
PumpService(EventDispatcher &dispatcher);               // selects device via Kconfig
PumpService(EventDispatcher &dispatcher, IPumpDevice &pumpDevice);  // inject explicitly (tests)
```

### Data Types
- **SFloat**: IEEE-11073 16-bit floating point (see `include/utils/sfloat.h`)
- Use `std::optional<T>` for potentially-absent values

### Error Handling
- Return codes preferred over exceptions (exceptions disabled in embedded context)
- Assertions for invariants (see safety rules above)

## Files to Reference
- **Architecture**: `app/src/main.cpp`, `include/events/EventDispatcher.h`
- **Build**: `west.yml`, `app/prj.conf`, `app/CMakeLists.txt`
- **Board defs**: `boards/insubox/insubox_rev_zero/`