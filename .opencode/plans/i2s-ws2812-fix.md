# Fix: Switch WS2812 driver from RMT to I2S

## Problem
WS2812 backlight and VEML7700 brightness regulation both freeze simultaneously after hours of operation. The entire `loop2` task hangs, likely because `FastLED.show()` locks up the RMT peripheral when WiFi is active. Reboot fixes it temporarily.

## Root Cause
FastLED on ESP32 uses the RMT peripheral for WS2812 timing. The RMT shares hardware resources with WiFi interrupts. Under sustained WiFi activity, the RMT state machine can enter an unrecoverable state, causing `FastLED.show()` to hang forever. This stalls the task that also handles VEML7700 I2C reads.

## Changes

### 1. `src/variables.h` — Switch FastLED driver from RMT to I2S
```diff
-#define FASTLED_ESP32_RMT_SERIAL_INSANE
+#define FASTLED_ESP32_I2S
```
The I2S peripheral is completely independent from RMT and WiFi, eliminating the hardware conflict.

### 2. Already done (no `loop2` task)
`loop2` task has been removed. `ws2812_effect()` and VEML7700 reading are now called directly from `loop()` on Core 1 using `millis()`-based timing. This eliminates all FreeRTOS task-related issues.

## Verification
1. Build: `pio run -e in14-ota`
2. Flash: `pio run -e in14-ota -t upload --upload-port IN14-NIXIE.local`
3. Run for several hours and check if the freeze returns.
