/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * config.h: Hardware configuration and device settings
 */

#pragma once

#include <Arduino.h>

#define FW_VERSION "0.3.0"

// Hardware selection
// PlatformIO environments select the target with build flags. AT-MK1 is the default
// ReflowDesk PCB target if this file is compiled without build flags.
// #define REFLOW_AT_MK1
// #define FIREBEETLE2_ESP32E

#if !defined(REFLOW_AT_MK1) && !defined(FIREBEETLE2_ESP32E)
#define REFLOW_AT_MK1
#endif

#if defined(REFLOW_AT_MK1) && defined(FIREBEETLE2_ESP32E)
#error "Select only one hardware target in config.h or PlatformIO build flags"
#endif

#if defined(REFLOW_AT_MK1)
#define REFLOW_HAS_BOARD_NTC 1
#define REFLOW_HAS_BOARD_FAN 1
#else
#define REFLOW_HAS_BOARD_NTC 0
#define REFLOW_HAS_BOARD_FAN 0
#endif

#if defined(REFLOW_AT_MK1)
#define DEVICE_NAME "ReflowDesk"
#define DEVICE_MODEL "AT-MK1"
#define DEVICE_MANUFACTURER "A-Tech Officials"
#define DEVICE_HW_VERSION "1.0"
#elif defined(FIREBEETLE2_ESP32E)
#define DEVICE_NAME "ReflowDesk"
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
#if defined(REFLOW_AT_MK1)
constexpr uint32_t I2C_SCL = 9;
constexpr uint32_t I2C_SDA = 8;
constexpr uint32_t ADS1115_ALERT = 21;

constexpr uint32_t SPI_MOSI = 11;
constexpr uint32_t SPI_SCK = 12;
constexpr uint32_t SPI_MISO = 13;

constexpr uint32_t MAX6675_CS = 16;

constexpr uint32_t OLED_CS = 10;
constexpr uint32_t OLED_DC = 14;
constexpr uint32_t OLED_RESET = 15;

constexpr uint32_t ENCODER_A = 4;
constexpr uint32_t ENCODER_B = 5;
constexpr uint32_t ENCODER_BTN = 6;

constexpr uint32_t HEATER_CTRL = 2;

constexpr uint32_t FAN_PWM = 1;
constexpr uint32_t FAN_PWM2 = 39;
constexpr uint32_t FAN_TACH = 38;
constexpr uint32_t FAN_TACH2 = 40;
constexpr uint32_t FAN_POWER = 17;

constexpr uint32_t STATUS_LED = 18;
constexpr uint32_t BUZZER = 7;
#elif defined(FIREBEETLE2_ESP32E)
constexpr uint32_t I2C_SCL = 22;
constexpr uint32_t I2C_SDA = 21;

constexpr uint32_t SPI_MOSI = 23;
constexpr uint32_t SPI_SCK = 18;
constexpr uint32_t SPI_MISO = 19;

constexpr uint32_t MAX6675_CS = 2;

constexpr uint32_t ENCODER_A = 13;
constexpr uint32_t ENCODER_B = 14;
constexpr uint32_t ENCODER_BTN = 15;

constexpr uint32_t HEATER_CTRL = 25;

constexpr uint32_t FAN_PWM = 17;
constexpr uint32_t FAN_TACH = 16;
constexpr uint32_t FAN_POWER = 4;

constexpr uint32_t STATUS_LED = 5;
constexpr uint32_t BUZZER = 26;
#endif
}

namespace PinSafety {
#if defined(REFLOW_AT_MK1)
constexpr bool isReservedPin(uint32_t pin) {
  return pin == 0 || pin == 3 || pin == 19 || pin == 20 || pin == 35 || pin == 36 || pin == 37 || pin == 43 ||
         pin == 44 || pin == 45 || pin == 46;
}

static_assert(!isReservedPin(Pins::I2C_SCL) && !isReservedPin(Pins::I2C_SDA) &&
                  !isReservedPin(Pins::ADS1115_ALERT) && !isReservedPin(Pins::SPI_MOSI) &&
                  !isReservedPin(Pins::SPI_SCK) && !isReservedPin(Pins::SPI_MISO) &&
                  !isReservedPin(Pins::MAX6675_CS) && !isReservedPin(Pins::OLED_CS) &&
                  !isReservedPin(Pins::OLED_DC) && !isReservedPin(Pins::OLED_RESET) &&
                  !isReservedPin(Pins::ENCODER_A) && !isReservedPin(Pins::ENCODER_B) &&
                  !isReservedPin(Pins::ENCODER_BTN) && !isReservedPin(Pins::HEATER_CTRL) &&
                  !isReservedPin(Pins::FAN_PWM) && !isReservedPin(Pins::FAN_PWM2) &&
                  !isReservedPin(Pins::FAN_TACH) && !isReservedPin(Pins::FAN_TACH2) &&
                  !isReservedPin(Pins::FAN_POWER) && !isReservedPin(Pins::STATUS_LED) &&
                  !isReservedPin(Pins::BUZZER),
              "Selected REFLOW_AT_MK1 pin uses a reserved ESP32-S3 function.");
#else
constexpr bool isFlashBusPin(uint32_t pin) {
  return pin >= 6 && pin <= 11;
}

constexpr bool isFlashVoltageStrap(uint32_t pin) {
  return pin == 12;
}

constexpr bool isInputOnlyPin(uint32_t pin) {
  return pin >= 34 && pin <= 39;
}

static_assert(!isFlashBusPin(Pins::I2C_SCL) && !isFlashBusPin(Pins::I2C_SDA) && !isFlashBusPin(Pins::SPI_MOSI) &&
                  !isFlashBusPin(Pins::SPI_SCK) && !isFlashBusPin(Pins::SPI_MISO) &&
                  !isFlashBusPin(Pins::MAX6675_CS) && !isFlashBusPin(Pins::ENCODER_A) &&
                  !isFlashBusPin(Pins::ENCODER_B) && !isFlashBusPin(Pins::ENCODER_BTN) &&
                  !isFlashBusPin(Pins::HEATER_CTRL) && !isFlashBusPin(Pins::FAN_PWM) &&
                  !isFlashBusPin(Pins::FAN_TACH) && !isFlashBusPin(Pins::FAN_POWER) &&
                  !isFlashBusPin(Pins::STATUS_LED) && !isFlashBusPin(Pins::BUZZER),
              "GPIO6-GPIO11 are connected to the ESP32 flash chip and cannot be used.");

static_assert(!isFlashVoltageStrap(Pins::I2C_SCL) && !isFlashVoltageStrap(Pins::I2C_SDA) &&
                  !isFlashVoltageStrap(Pins::SPI_MOSI) && !isFlashVoltageStrap(Pins::SPI_SCK) &&
                  !isFlashVoltageStrap(Pins::SPI_MISO) && !isFlashVoltageStrap(Pins::MAX6675_CS) &&
                  !isFlashVoltageStrap(Pins::ENCODER_A) && !isFlashVoltageStrap(Pins::ENCODER_B) &&
                  !isFlashVoltageStrap(Pins::ENCODER_BTN) && !isFlashVoltageStrap(Pins::HEATER_CTRL) &&
                  !isFlashVoltageStrap(Pins::FAN_PWM) && !isFlashVoltageStrap(Pins::FAN_TACH) &&
                  !isFlashVoltageStrap(Pins::FAN_POWER) && !isFlashVoltageStrap(Pins::STATUS_LED) &&
                  !isFlashVoltageStrap(Pins::BUZZER),
              "GPIO12 is the ESP32 flash-voltage strapping pin; external pullups can break flash upload/boot.");

static_assert(!isInputOnlyPin(Pins::HEATER_CTRL) && !isInputOnlyPin(Pins::FAN_PWM) &&
                  !isInputOnlyPin(Pins::FAN_POWER) && !isInputOnlyPin(Pins::STATUS_LED) &&
                  !isInputOnlyPin(Pins::BUZZER),
              "GPIO34-GPIO39 are input-only and cannot drive outputs.");
#endif
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

namespace BoardCooling {
constexpr float FAN_OFF_C = 40.0f;
constexpr float FAN_ON_C = 45.0f;
constexpr float FAN_FULL_C = 65.0f;
constexpr uint8_t FAN_MIN_PERCENT = 25;
constexpr uint8_t FAN_MAX_PERCENT = 100;
constexpr uint8_t FAN_NTC_FAIL_PERCENT = 100;
}

constexpr uint8_t OLED_ADDRESS = 0x3C;
constexpr uint8_t ADS1115_I2C_ADDRESS = 0x48;
constexpr uint8_t ADS1115_AMBIENT_NTC_CHANNEL = 0;
constexpr uint8_t ADS1115_BOARD_NTC_CHANNEL = 1;
constexpr uint8_t ADS1115_NTC_CHANNEL = ADS1115_AMBIENT_NTC_CHANNEL;
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
