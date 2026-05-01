/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * config.h: Hardware configuration and device settings
 */

#pragma once

#include <Arduino.h>

#define FW_VERSION "0.2.1"

// Hardware selection
// Select exactly one hardware target. Keep board-specific pin assignments in
// the matching block below so firmware behavior follows the selected hardware.
// #define AT_REFLOW_MK1
#define FIREBEETLE2_ESP32E

#if !defined(AT_REFLOW_MK1) && !defined(FIREBEETLE2_ESP32E)
#define FIREBEETLE2_ESP32E
#endif

#if defined(AT_REFLOW_MK1) && defined(FIREBEETLE2_ESP32E)
#error "Select only one hardware target in config.h"
#endif

#if defined(AT_REFLOW_MK1)
#define DEVICE_NAME "ReflowDesk"
#define DEVICE_MODEL "AT-MK1"
#define DEVICE_MANUFACTURER "A-Tech Officials"
#define DEVICE_HW_VERSION "1.0"
#elif defined(FIREBEETLE2_ESP32E)
#define DEVICE_NAME "SMD Reflow Plate"
#define DEVICE_MODEL "FireBeetle2"
#define DEVICE_MANUFACTURER "A-Tech Officials"
#define DEVICE_HW_VERSION "1.0"
#endif

namespace DeviceInfo {
constexpr const char *NAME = DEVICE_NAME;
constexpr const char *MANUFACTURER = DEVICE_MANUFACTURER;
constexpr const char *MODEL = DEVICE_MODEL;
constexpr const char *HW_VERSION = DEVICE_HW_VERSION;
constexpr const char *FW = FW_VERSION;
}

namespace Pins {
#if defined(AT_REFLOW_MK1)
constexpr uint32_t I2C_SCL = 22;
constexpr uint32_t I2C_SDA = 21;

constexpr uint32_t MAX6675_CS = 2;
constexpr uint32_t MAX6675_SCK = 18;
constexpr uint32_t MAX6675_SO = 19;

constexpr uint32_t ENCODER_A = 13;
constexpr uint32_t ENCODER_B = 14;
constexpr uint32_t ENCODER_BTN = 15;

constexpr uint32_t SSR = 25;

constexpr uint32_t FAN_PWM = 17;
constexpr uint32_t FAN_TACH = 16;
constexpr uint32_t FAN_POWER = 4;

constexpr uint32_t STATUS_LED = 5;
constexpr uint32_t BUZZER = 26;
#elif defined(FIREBEETLE2_ESP32E)
constexpr uint32_t I2C_SCL = 22;
constexpr uint32_t I2C_SDA = 21;

constexpr uint32_t MAX6675_CS = 2;
constexpr uint32_t MAX6675_SCK = 18;
constexpr uint32_t MAX6675_SO = 19;

constexpr uint32_t ENCODER_A = 13;
constexpr uint32_t ENCODER_B = 14;
constexpr uint32_t ENCODER_BTN = 15;

constexpr uint32_t SSR = 25;

constexpr uint32_t FAN_PWM = 17;
constexpr uint32_t FAN_TACH = 16;
constexpr uint32_t FAN_POWER = 4;

constexpr uint32_t STATUS_LED = 5;
constexpr uint32_t BUZZER = 26;
#endif
}

namespace PinSafety {
constexpr bool isFlashBusPin(uint32_t pin) {
  return pin >= 6 && pin <= 11;
}

constexpr bool isFlashVoltageStrap(uint32_t pin) {
  return pin == 12;
}

constexpr bool isInputOnlyPin(uint32_t pin) {
  return pin >= 34 && pin <= 39;
}

static_assert(!isFlashBusPin(Pins::I2C_SCL) && !isFlashBusPin(Pins::I2C_SDA) && !isFlashBusPin(Pins::MAX6675_CS) &&
                  !isFlashBusPin(Pins::MAX6675_SCK) && !isFlashBusPin(Pins::MAX6675_SO) && !isFlashBusPin(Pins::ENCODER_A) &&
                  !isFlashBusPin(Pins::ENCODER_B) && !isFlashBusPin(Pins::ENCODER_BTN) && !isFlashBusPin(Pins::SSR) &&
                  !isFlashBusPin(Pins::FAN_PWM) && !isFlashBusPin(Pins::FAN_TACH) && !isFlashBusPin(Pins::FAN_POWER) &&
                  !isFlashBusPin(Pins::STATUS_LED) && !isFlashBusPin(Pins::BUZZER),
              "GPIO6-GPIO11 are connected to the ESP32 flash chip and cannot be used.");

static_assert(!isFlashVoltageStrap(Pins::I2C_SCL) && !isFlashVoltageStrap(Pins::I2C_SDA) && !isFlashVoltageStrap(Pins::MAX6675_CS) &&
                  !isFlashVoltageStrap(Pins::MAX6675_SCK) && !isFlashVoltageStrap(Pins::MAX6675_SO) &&
                  !isFlashVoltageStrap(Pins::ENCODER_A) && !isFlashVoltageStrap(Pins::ENCODER_B) &&
                  !isFlashVoltageStrap(Pins::ENCODER_BTN) && !isFlashVoltageStrap(Pins::SSR) &&
                  !isFlashVoltageStrap(Pins::FAN_PWM) && !isFlashVoltageStrap(Pins::FAN_TACH) &&
                  !isFlashVoltageStrap(Pins::FAN_POWER) && !isFlashVoltageStrap(Pins::STATUS_LED) &&
                  !isFlashVoltageStrap(Pins::BUZZER),
              "GPIO12 is the ESP32 flash-voltage strapping pin; external pullups can break flash upload/boot.");

static_assert(!isInputOnlyPin(Pins::SSR) && !isInputOnlyPin(Pins::FAN_PWM) && !isInputOnlyPin(Pins::FAN_POWER) &&
                  !isInputOnlyPin(Pins::STATUS_LED) && !isInputOnlyPin(Pins::BUZZER),
              "GPIO34-GPIO39 are input-only and cannot drive outputs.");
}

namespace Timing {
constexpr uint16_t TEMP_READ_MS = 500;
constexpr uint16_t AMBIENT_READ_MS = 1000;
constexpr uint16_t DISPLAY_MS = 180;
constexpr uint16_t CONTROL_MS = 500;
constexpr uint16_t SSR_WINDOW_MS = 2000;
constexpr uint16_t BUTTON_DEBOUNCE_MS = 40;
constexpr uint32_t HEATER_RESPONSE_CHECK_MS = 45000;
constexpr uint32_t FORCED_COOLDOWN_MS = 120000;
}

namespace Limits {
constexpr int16_t PREHEAT_MIN_C = 80;
constexpr int16_t PREHEAT_MAX_C = 180;
constexpr int16_t SOAK_MIN_C = 120;
constexpr int16_t SOAK_MAX_C = 210;
constexpr int16_t REFLOW_MIN_C = 160;
constexpr int16_t REFLOW_MAX_C = 245;
constexpr int16_t SAFE_TOUCH_MIN_C = 35;
constexpr int16_t SAFE_TOUCH_MAX_C = 50;
constexpr int16_t SAFETY_MIN_C = 220;
constexpr int16_t SAFETY_MAX_C = 250;
constexpr uint16_t STAGE_TIME_MIN_S = 5;
constexpr uint16_t STAGE_TIME_MAX_S = 600;
}

constexpr uint8_t OLED_ADDRESS = 0x3C;
constexpr uint8_t ADS1115_I2C_ADDRESS = 0x48;
constexpr uint8_t ADS1115_NTC_CHANNEL = 0;
constexpr uint8_t OLED_WIDTH = 128;
constexpr uint8_t OLED_HEIGHT = 64;
constexpr int8_t OLED_RESET = -1;

constexpr float NTC_SERIES_OHMS = 100000.0f;
constexpr float NTC_NOMINAL_OHMS = 100000.0f;
constexpr float NTC_NOMINAL_TEMP_C = 25.0f;
constexpr float NTC_BETA = 3950.0f;
constexpr float NTC_BIAS_VOLTAGE = 3.3f;
constexpr uint16_t MAX6675_SW_SPI_DELAY_US = 2;
constexpr float MIN_HEATER_RESPONSE_RISE_C = 2.0f;

// Set to 1 for serial trace of temperatures, PID output and state changes.
#ifndef REFLOW_DEBUG
#define REFLOW_DEBUG 0
#endif
