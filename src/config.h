/* ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
 * Github: https://github.com/atechofficials/ReflowDesk
 * Author: Mrinal @atechofficials
 * License: General Public License v3.0
 * config.h: Hardware configuration and device settings
 */

#pragma once

#include <Arduino.h>

#define FW_VERSION "0.9.3"

// Hardware selection
// PlatformIO environments select the target with build flags. AT-MK1 is the default
// ReflowDesk PCB target if this file is compiled without build flags.
// #define REFLOW_AT_MK1
// #define FIREBEETLE2_ESP32E
// #define ESP32_S3_PICO

// PTC heater output mode selection.
// AT-MK1 hardware uses jumpers to route HEATER_CTRL:
// - JP2 shorted, JP3 open: 220VAC PTC heater control through the zero-cross SSR circuit.
//   Leave REFLOW_HEATER_DC_PWM undefined or set to 0.
// - JP3 shorted, JP2 open: low-voltage DC PTC heater control through the MOSFET circuit.
//   Set REFLOW_HEATER_DC_PWM to 1.
// Do not short JP2 and JP3 together. The firmware mode must match the physical jumper.
// #define REFLOW_HEATER_DC_PWM 1

// DC PTC Heating Element Voltage Selection:
// #define REFLOW_DC_HEATER_VOLTAGE_12V
// #define REFLOW_DC_HEATER_VOLTAGE_24V

#if !defined(REFLOW_AT_MK1) && !defined(FIREBEETLE2_ESP32E) && !defined(ESP32_S3_PICO)
#define REFLOW_AT_MK1
#endif

#if (defined(REFLOW_AT_MK1) + defined(FIREBEETLE2_ESP32E) + defined(ESP32_S3_PICO)) != 1
#error "Select exactly one hardware target in config.h or PlatformIO build flags"
#endif

#if defined(REFLOW_AT_MK1)
#define REFLOW_HAS_BOARD_NTC 1
#define REFLOW_HAS_BOARD_FAN 1
#else
#define REFLOW_HAS_BOARD_NTC 0
#define REFLOW_HAS_BOARD_FAN 0
#endif

#ifndef REFLOW_WEB_ENABLED
#if defined(REFLOW_AT_MK1) || defined(ESP32_S3_PICO)
#define REFLOW_WEB_ENABLED 1
#else
#define REFLOW_WEB_ENABLED 0
#endif
#endif

#if defined(REFLOW_AT_MK1) && defined(REFLOW_WEB_ENABLED)
#define REFLOW_WEB_SETUP_AP_BASE_SSID "ReflowDesk-AT-MK1"
#elif defined(ESP32_S3_PICO) && defined(REFLOW_WEB_ENABLED)
#define REFLOW_WEB_SETUP_AP_BASE_SSID "ReflowDesk-Dev"
#else
#define REFLOW_WEB_SETUP_AP_BASE_SSID "ReflowDesk-Dev"
#endif

#ifndef REFLOW_WEB_SETUP_AP_PASSWORD
#define REFLOW_WEB_SETUP_AP_PASSWORD "ReflowDesk@2037"
#endif

#ifndef REFLOW_OLED_SLEEP_DEFAULT_SECONDS
#define REFLOW_OLED_SLEEP_DEFAULT_SECONDS 120
#endif

#ifndef REFLOW_OLED_BRIGHTNESS_MAX_PERCENT
#define REFLOW_OLED_BRIGHTNESS_MAX_PERCENT 100
#endif

// SSR output polarity configuration
// 0 = Active-high SSR: MCU HIGH = SSR ON, MCU LOW = SSR OFF
// 1 = Active-low SSR:  MCU LOW  = SSR ON, MCU HIGH = SSR OFF
#ifndef REFLOW_SSR_ACTIVE_LOW
#if defined(REFLOW_AT_MK1) || defined(ESP32_S3_PICO)
#define REFLOW_SSR_ACTIVE_LOW 1
#else
#define REFLOW_SSR_ACTIVE_LOW 0
#endif
#endif

#ifndef REFLOW_HEATER_DC_PWM
#define REFLOW_HEATER_DC_PWM 0
#endif

#if REFLOW_HEATER_DC_PWM != 0 && REFLOW_HEATER_DC_PWM != 1
#error "REFLOW_HEATER_DC_PWM must be set to 0 or 1 to match the hardware jumper configuration for the HEATER_CTRL pin."
#endif

#if REFLOW_HEATER_DC_PWM
#if defined(REFLOW_DC_HEATER_VOLTAGE_12V) && defined(REFLOW_DC_HEATER_VOLTAGE_24V)
#error "Select only one DC heater voltage option in config.h or PlatformIO build flags."
#elif !defined(REFLOW_DC_HEATER_VOLTAGE_12V) && !defined(REFLOW_DC_HEATER_VOLTAGE_24V)
#define REFLOW_DC_HEATER_VOLTAGE_24V
#endif
#endif

// Default PID tuning parameters for fresh settings/factory reset.
// User-saved PID values in NVS override these defaults at runtime.
#if REFLOW_HEATER_DC_PWM == 0
// 220VAC PTC heater with zero-cross SSR time-window control.
#define REFLOW_DEFAULT_PID_KP 6.0f
#define REFLOW_DEFAULT_PID_KI 0.04f
#define REFLOW_DEFAULT_PID_KD 35.0f
#elif defined(REFLOW_DC_HEATER_VOLTAGE_12V)
// 12V DC PTC heater with MOSFET PWM control.
#define REFLOW_DEFAULT_PID_KP 6.0f
#define REFLOW_DEFAULT_PID_KI 0.04f
#define REFLOW_DEFAULT_PID_KD 35.0f
#elif defined(REFLOW_DC_HEATER_VOLTAGE_24V)
// 24V DC PTC heater with MOSFET PWM control.
#define REFLOW_DEFAULT_PID_KP 5.0f
#define REFLOW_DEFAULT_PID_KI 0.02f
#define REFLOW_DEFAULT_PID_KD 40.0f
#else
#error "DC heater PWM mode requires REFLOW_DC_HEATER_VOLTAGE_12V or REFLOW_DC_HEATER_VOLTAGE_24V."
#endif

static_assert(sizeof(REFLOW_WEB_SETUP_AP_PASSWORD) >= 9 && sizeof(REFLOW_WEB_SETUP_AP_PASSWORD) <= 64,
              "REFLOW_WEB_SETUP_AP_PASSWORD must be 8 to 63 characters for WPA2 setup AP security.");
static_assert(REFLOW_OLED_BRIGHTNESS_MAX_PERCENT >= 10 && REFLOW_OLED_BRIGHTNESS_MAX_PERCENT <= 100 &&
                  REFLOW_OLED_BRIGHTNESS_MAX_PERCENT % 10 == 0,
              "REFLOW_OLED_BRIGHTNESS_MAX_PERCENT must be a 10..100 percent value in 10 percent steps.");

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
#elif defined(ESP32_S3_PICO)
#define DEVICE_NAME "ReflowDesk"
#define DEVICE_MODEL "ESP32-S3-Pico"
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

#define RGB_LED_COLOR_ORDER NEO_GRB

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

#define RGB_LED_COLOR_ORDER NEO_GRB

#elif defined(ESP32_S3_PICO)
constexpr uint32_t I2C_SCL = 11;
constexpr uint32_t I2C_SDA = 12;

constexpr uint32_t SPI_MOSI = 13;
constexpr uint32_t SPI_SCK = 14;
constexpr uint32_t SPI_MISO = 15;

constexpr uint32_t MAX6675_CS = 16;

constexpr uint32_t ENCODER_A = 17;
constexpr uint32_t ENCODER_B = 18;
constexpr uint32_t ENCODER_BTN = 33;

constexpr uint32_t HEATER_CTRL = 34;

constexpr uint32_t FAN_PWM = 38;
constexpr uint32_t FAN_TACH = 39;
constexpr uint32_t FAN_POWER = 40;

constexpr uint32_t STATUS_LED = 21;
constexpr uint32_t BUZZER = 41;

#define RGB_LED_COLOR_ORDER NEO_RGB
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
#elif defined(FIREBEETLE2_ESP32E)
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
#elif defined(ESP32_S3_PICO)
constexpr bool isReservedS3Pin(uint32_t pin) {
  return pin == 0 || pin == 3 || pin == 19 || pin == 20 || (pin >= 26 && pin <= 32) || pin == 45 || pin == 46;
}

static_assert(!isReservedS3Pin(Pins::I2C_SCL) && !isReservedS3Pin(Pins::I2C_SDA) &&
                  !isReservedS3Pin(Pins::SPI_MOSI) && !isReservedS3Pin(Pins::SPI_SCK) &&
                  !isReservedS3Pin(Pins::SPI_MISO) && !isReservedS3Pin(Pins::MAX6675_CS) &&
                  !isReservedS3Pin(Pins::ENCODER_A) && !isReservedS3Pin(Pins::ENCODER_B) &&
                  !isReservedS3Pin(Pins::ENCODER_BTN) && !isReservedS3Pin(Pins::HEATER_CTRL) &&
                  !isReservedS3Pin(Pins::FAN_PWM) && !isReservedS3Pin(Pins::FAN_TACH) &&
                  !isReservedS3Pin(Pins::FAN_POWER) && !isReservedS3Pin(Pins::STATUS_LED) &&
                  !isReservedS3Pin(Pins::BUZZER),
              "Selected ESP32_S3_PICO pin uses a reserved ESP32-S3 function.");
#endif
}

namespace Timing {
constexpr uint16_t TEMP_READ_MS = 500;
constexpr uint16_t AMBIENT_READ_MS = 1000;
constexpr uint16_t DISPLAY_MS = 60;

constexpr uint16_t CONTROL_MS = 500;
constexpr uint16_t SSR_WINDOW_MS = 2000; // Good for a zero-cross SSR and 220VAC PTC heater

constexpr uint16_t BUTTON_DEBOUNCE_MS = 40;
constexpr uint32_t HEATER_RESPONSE_CHECK_MS = 45000;
constexpr uint32_t FORCED_COOLDOWN_MS = 120000;
constexpr uint16_t OLED_SLEEP_DEFAULT_SECONDS = REFLOW_OLED_SLEEP_DEFAULT_SECONDS;
}

namespace Limits {
// Reflow Profiles Limits/setting ranges
constexpr int16_t PREHEAT_MIN_C = 50;
constexpr int16_t PREHEAT_MAX_C = 180;

constexpr int16_t SOAK_MIN_C = 120;
constexpr int16_t SOAK_MAX_C = 210;

constexpr int16_t REFLOW_MIN_C = 160;
constexpr int16_t REFLOW_MAX_C = 225; // maximum user-selectable target

constexpr int16_t PROFILE_STAGE_GAP_C = 10;
constexpr int16_t PROFILE_PREHEAT_SAFE_TOUCH_OFFSET_C = 15;

// Hot Plate Safe to touch temp range
constexpr int16_t SAFE_TOUCH_MIN_C = 35;
constexpr int16_t SAFE_TOUCH_MAX_C = 50;

// Hot Plate Safety temp cutoffs and limits
constexpr int16_t SAFETY_MIN_C = 220;
constexpr int16_t SAFETY_MAX_C = 235; // emergency firmware cutoff

constexpr uint16_t STAGE_TIME_MIN_S = 20;
constexpr uint16_t STAGE_TIME_MAX_S = 600;

// OLED brightness limits
constexpr uint8_t OLED_BRIGHTNESS_MIN_PERCENT = 10;
constexpr uint8_t OLED_BRIGHTNESS_MAX_PERCENT = REFLOW_OLED_BRIGHTNESS_MAX_PERCENT;
constexpr uint8_t OLED_BRIGHTNESS_STEP_PERCENT = 10;
}

namespace SensorLimits {
constexpr float PLATE_SENSOR_MIN_VALID_C = 0.0f;
constexpr float PLATE_SENSOR_MAX_VALID_C = 300.0f;
constexpr float PLATE_MAX_JUMP_PER_READ_C = 25.0f;
constexpr uint8_t PLATE_MAX_SPIKES_BEFORE_FAULT = 3;

constexpr float AMBIENT_MAX_RISE_PER_READ_C = 1.0f;
constexpr float AMBIENT_MAX_FALL_PER_READ_C = 2.0f;

constexpr float BOARD_MAX_RISE_PER_READ_C = 2.0f;
constexpr float BOARD_MAX_FALL_PER_READ_C = 3.0f;
}

namespace HeaterTuning {
constexpr bool SSR_ACTIVE_LOW = REFLOW_SSR_ACTIVE_LOW;
constexpr bool DC_PWM_ENABLED = REFLOW_HEATER_DC_PWM != 0;
constexpr uint32_t DC_PWM_FREQ_HZ = 1000;
constexpr uint8_t DC_PWM_BITS = 10;
constexpr uint16_t DC_PWM_MAX_DUTY = (1U << DC_PWM_BITS) - 1U;

constexpr float INTEGRAL_MIN = -600.0f;
constexpr float INTEGRAL_MAX = 600.0f;

constexpr float AMBIENT_BOOST_GAIN = 0.08f;
constexpr float AMBIENT_BOOST_MAX = 8.0f;

constexpr float OVERSHOOT_CUTOFF_C = 3.0f;

constexpr float MIN_VALID_PLATE_C = -20.0f;
constexpr float MAX_VALID_PLATE_C = Limits::SAFETY_MAX_C;

constexpr float DUTY_MAX_STEP_PER_UPDATE = 25.0f;

constexpr float WARMUP_ERROR_FAR_C = 50.0f;
constexpr float WARMUP_ERROR_MID_C = 30.0f;
constexpr float WARMUP_ERROR_NEAR_C = 15.0f;

constexpr float WARMUP_DUTY_FAR_PERCENT = 95.0f;
constexpr float WARMUP_DUTY_MID_PERCENT = 80.0f;
constexpr float WARMUP_DUTY_NEAR_PERCENT = 50.0f;

// DC MOSFET PWM approach caps reduce thermal coast as the plate nears a stage target.
// AC SSR mode continues to use the normal time-window output path without these caps.
constexpr float DC_APPROACH_CAP_FAR_ERROR_C = 25.0f;
constexpr float DC_APPROACH_CAP_MID_ERROR_C = 12.0f;
constexpr float DC_APPROACH_CAP_NEAR_ERROR_C = 4.0f;

constexpr float DC_APPROACH_CAP_FAR_DUTY_PERCENT = 85.0f;
constexpr float DC_APPROACH_CAP_MID_DUTY_PERCENT = 65.0f;
constexpr float DC_APPROACH_CAP_NEAR_DUTY_PERCENT = 45.0f;

constexpr float HEATER_PID_KP = REFLOW_DEFAULT_PID_KP;
constexpr float HEATER_PID_KI = REFLOW_DEFAULT_PID_KI;
constexpr float HEATER_PID_KD = REFLOW_DEFAULT_PID_KD;
}

static_assert(HeaterTuning::DC_PWM_FREQ_HZ == 1000,
              "The default DC heater PWM frequency is intentionally 1 kHz for the EL817 optocoupler gate path.");
static_assert(HeaterTuning::DC_PWM_BITS >= 8 && HeaterTuning::DC_PWM_BITS <= 12,
              "DC heater PWM resolution should remain practical for ESP32 LEDC output.");
static_assert(HeaterTuning::DC_APPROACH_CAP_FAR_ERROR_C > HeaterTuning::DC_APPROACH_CAP_MID_ERROR_C &&
                  HeaterTuning::DC_APPROACH_CAP_MID_ERROR_C > HeaterTuning::DC_APPROACH_CAP_NEAR_ERROR_C,
              "DC heater approach cap error bands must be ordered far > mid > near.");
static_assert(HeaterTuning::DC_APPROACH_CAP_FAR_DUTY_PERCENT >= HeaterTuning::DC_APPROACH_CAP_MID_DUTY_PERCENT &&
                  HeaterTuning::DC_APPROACH_CAP_MID_DUTY_PERCENT >= HeaterTuning::DC_APPROACH_CAP_NEAR_DUTY_PERCENT,
              "DC heater approach cap duty limits must be ordered far >= mid >= near.");

namespace BoardCooling {
// Motherboard cooling fan control thresholds and settings
constexpr float FAN_OFF_C = 40.0f;
constexpr float FAN_ON_C = 45.0f;
constexpr float FAN_FULL_C = 65.0f;
constexpr uint8_t FAN_MIN_PERCENT = 25;
constexpr uint8_t FAN_MAX_PERCENT = 100;
// If the board NTC fails, run the fan at this percentage to provide some cooling as a fallback.
constexpr uint8_t FAN_NTC_FAIL_PERCENT = 80;
}

namespace FanTuning {
// Set these near the maximum realistic RPM for the installed fan model plus margin.
// Smaller 40 mm / 60 mm fans may need a higher limit than the current bench fan.
constexpr uint16_t HOT_PLATE_MAX_VALID_RPM = 4500;
constexpr uint16_t BOARD_MAX_VALID_RPM = 4500;
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
constexpr float NTC_BIAS_VOLTAGE = 3.3f; // 3.3V supply used for the NTC voltage divider
constexpr uint16_t MAX6675_SW_SPI_DELAY_US = 2;
constexpr float MIN_HEATER_RESPONSE_RISE_C = 2.0f;

// Set to 1 for serial trace of temperatures, PID output and state changes.
#ifndef REFLOW_DEBUG
#define REFLOW_DEBUG 0
#endif

#ifndef RGB_LED_COLOR_ORDER
#define RGB_LED_COLOR_ORDER NEO_GRB
#endif
