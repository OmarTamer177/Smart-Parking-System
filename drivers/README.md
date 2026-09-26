# Hardware Peripheral Drivers

This directory houses reusable hardware and sensor device drivers for the **TM4C123GH6PM** platform.

---

## Modules

### 1. InvenSense MPU-6050 Driver (`mpu6050.c`, `mpu6050.h`)
- **Type:** 6-Axis MotionTracking IMU (3-Axis Gyroscope + 3-Axis Accelerometer).
- **Interface:** I2C (default address `0x68`).
- **Features:**
  - Device wakeup and power management configuration.
  - Digital Low-Pass Filter (DLPF) initialization.
  - Full-scale ranges: $\pm2000^\circ/\text{s}$ gyroscope, $\pm2\text{g}$ accelerometer.
  - Burst-read telemetry acquisition: `MPU6050_ReadData(...)`.
- **Automotive Safety Application:** Vehicle tilt sensing, rollover threshold detection, and high-g crash impact sensing.

### 2. TM4C Hardware I2C3 Master Driver (`i2c3_driver.c`, `i2c3_driver.h`)
- **Pins:** `PD0` (SCL), `PD1` (SDA, Open-Drain).
- **Bus Speed:** 100 kHz standard mode @ 16 MHz system clock.
- **RTOS Integration:** Uses non-blocking FreeRTOS `vTaskDelay` during bus initialization.

---

## Prototype Test Code (`test/`)
- `i2c_test.c` / `i2c_test.h`: Standalone bare-metal test bench with an isolated `main()` function used during initial hardware bring-up and logic analyzer validation.
