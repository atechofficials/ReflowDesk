"""
ReflowDesk: Desktop SMD Reflow Soldering Hot Plate
File: merge_factory_bin.py
Purpose: Merge bootloader.bin, partitions.bin, boot_app0.bin, 
and firmware.bin into a single factory firmware binary for easy flashing.
Github: https://github.com/atechofficials/ReflowDesk
Author: Mrinal @atechofficials
License: General Public License v3.0
"""

import re
import subprocess
from pathlib import Path

Import("env")

PROJECT_DIR = Path(env.subst("$PROJECT_DIR"))
BUILD_DIR = Path(env.subst("$BUILD_DIR"))
PIOENV = env.subst("$PIOENV")

OUTPUT_DIR = PROJECT_DIR / "release_bins"
OUTPUT_DIR.mkdir(exist_ok=True)

PROJECT_NAME = "ReflowDesk"

# Optional PlatformIO custom option:
# custom_release_suffix = beta.1
RELEASE_SUFFIX = env.GetProjectOption("custom_release_suffix", "").strip()
# custom_release_board_name = AT-MK1
RELEASE_BOARD_NAME = env.GetProjectOption("custom_release_board_name", "").strip()

BOARD_CONFIG = env.BoardConfig()


def normalize_chip_name(chip: str) -> str:
    """
    Convert PlatformIO MCU names into esptool chip names.
    Examples:
      esp32      -> esp32
      esp32s3    -> esp32s3
      esp32-s3   -> esp32s3
    """
    chip = chip.lower().strip()
    chip = chip.replace("-", "")

    supported = {
        "esp32": "esp32",
        "esp32s2": "esp32s2",
        "esp32s3": "esp32s3",
        "esp32c3": "esp32c3",
        "esp32c6": "esp32c6",
        "esp32h2": "esp32h2",
    }

    return supported.get(chip, chip)


def detect_chip() -> str:
    """
    Prefer board_build.mcu from platformio.ini.
    Fall back to board manifest build.mcu.
    """
    chip = env.GetProjectOption("board_build.mcu", "")

    if not chip:
        chip = BOARD_CONFIG.get("build.mcu", "")

    if not chip:
        raise RuntimeError("Could not detect ESP32 chip type from PlatformIO config.")

    return normalize_chip_name(chip)


def detect_flash_size() -> str:
    """
    Detect flash size from the PlatformIO board manifest.

    For example:
      esp32-s3-devkitc1-n8r8  -> 8MB
      esp32-s3-devkitc1-n16r2 -> 16MB
    """
    flash_size = BOARD_CONFIG.get("upload.flash_size", "")

    if not flash_size:
        # Optional manual fallback if ever needed:
        # board_upload.flash_size = 8MB
        flash_size = env.GetProjectOption("board_upload.flash_size", "")

    if not flash_size:
        raise RuntimeError(
            "Could not detect flash size from PlatformIO board config. "
            "Add board_upload.flash_size = 8MB or use a board definition with upload.flash_size."
        )

    return str(flash_size).upper()


def detect_flash_mode() -> str:
    """
    Detect flash mode from board config or platformio.ini.
    Common values: dio, qio, dout, qout.
    """
    flash_mode = env.GetProjectOption("board_build.flash_mode", "")

    if not flash_mode:
        flash_mode = BOARD_CONFIG.get("build.flash_mode", "")

    if not flash_mode:
        flash_mode = "dio"

    return str(flash_mode).lower()


def detect_flash_freq() -> str:
    """
    Convert PlatformIO flash frequency format into esptool format.

    PlatformIO may provide:
      40000000L
      80000000L

    esptool expects:
      40m
      80m
    """
    freq = env.GetProjectOption("board_build.f_flash", "")

    if not freq:
        freq = BOARD_CONFIG.get("build.f_flash", "")

    if not freq:
        return "40m"

    freq = str(freq).lower().replace("l", "").strip()

    if freq in ("40000000", "40mhz", "40m"):
        return "40m"

    if freq in ("80000000", "80mhz", "80m"):
        return "80m"

    return freq


def find_config_file() -> Path:
    """
    Search common config.h locations.
    Adjust this list if your FW_VERSION is somewhere else.
    """
    candidates = [
        PROJECT_DIR / "include" / "config.h",
        PROJECT_DIR / "src" / "config.h",
        PROJECT_DIR / "config.h",
    ]

    for path in candidates:
        if path.exists():
            return path

    raise FileNotFoundError(
        "Could not find config.h. Expected one of: include/config.h, src/config.h, or config.h"
    )


def read_fw_version() -> str:
    """
    Parse:
      #define FW_VERSION "0.8.1"

    Also supports:
      #define FW_VERSION "v0.8.1"
      #define FW_VERSION "0.8.1-beta.1"
    """
    config_file = find_config_file()
    content = config_file.read_text(encoding="utf-8", errors="ignore")

    match = re.search(
        r'^\s*#\s*define\s+FW_VERSION\s+"([^"]+)"',
        content,
        flags=re.MULTILINE,
    )

    if not match:
        raise RuntimeError(f'Could not find #define FW_VERSION "..." inside {config_file}')

    version = match.group(1).strip()

    if not version:
        raise RuntimeError("FW_VERSION is empty.")

    if not version.startswith("v"):
        version = f"v{version}"

    if RELEASE_SUFFIX:
        version = f"{version}-{RELEASE_SUFFIX}"

    return version


def check_file(path: Path) -> None:
    if not path.exists():
        raise FileNotFoundError(f"Required file not found: {path}")


def get_boot_app0_path() -> Path:
    """
    Arduino-ESP32 boot_app0.bin path inside PlatformIO package.
    """
    path = (
        Path.home()
        / ".platformio"
        / "packages"
        / "framework-arduinoespressif32"
        / "tools"
        / "partitions"
        / "boot_app0.bin"
    )

    if path.exists():
        return path

    raise FileNotFoundError(f"boot_app0.bin not found at expected path: {path}")

def sanitize_filename_part(value: str) -> str:
    """
    Keep release filename parts safe and readable.

    Examples:
      "ESP32-S3 DevKit" -> "ESP32-S3_DevKit"
      "AT MK1 / 8MB"    -> "AT_MK1_8MB"
    """
    value = value.strip()
    value = re.sub(r"\s+", "_", value)
    value = re.sub(r'[^A-Za-z0-9._-]', "_", value)
    value = re.sub(r"_+", "_", value)
    return value.strip("_")

def merge_factory_bin(source, target, env):
    chip = detect_chip()
    flash_size = detect_flash_size()
    flash_mode = detect_flash_mode()
    flash_freq = detect_flash_freq()
    fw_version = read_fw_version()

    bootloader_bin = BUILD_DIR / "bootloader.bin"
    partitions_bin = BUILD_DIR / "partitions.bin"
    firmware_bin = BUILD_DIR / "firmware.bin"
    boot_app0_bin = get_boot_app0_path()

    release_board_name = RELEASE_BOARD_NAME if RELEASE_BOARD_NAME else PIOENV
    release_board_name = sanitize_filename_part(release_board_name)

    output_file = OUTPUT_DIR / f"{PROJECT_NAME}_{release_board_name}_{flash_size}_{fw_version}_factory.bin"

    required_files = [
        bootloader_bin,
        partitions_bin,
        boot_app0_bin,
        firmware_bin,
    ]

    for file_path in required_files:
        check_file(file_path)

    print()
    print("============================================================")
    print("Creating merged ESP32 factory firmware")
    print("============================================================")
    print(f"Environment : {PIOENV}")
    print(f"Release name : {release_board_name}")
    print(f"Chip        : {chip}")
    print(f"Flash mode  : {flash_mode}")
    print(f"Flash freq  : {flash_freq}")
    print(f"Flash size  : {flash_size}")
    print(f"FW version  : {fw_version}")
    print(f"Output      : {output_file}")
    print("============================================================")
    print()

    command = [
        "pio",
        "pkg",
        "exec",
        "-p",
        "tool-esptoolpy",
        "--",
        "esptool",
        "--chip",
        chip,
        "merge-bin",
        "-o",
        str(output_file),
        "--flash-mode",
        flash_mode,
        "--flash-freq",
        flash_freq,
        "--flash-size",
        flash_size,
        "0x0",
        str(bootloader_bin),
        "0x8000",
        str(partitions_bin),
        "0xE000",
        str(boot_app0_bin),
        "0x10000",
        str(firmware_bin),
    ]

    subprocess.run(command, check=True)

    print()
    print("Factory firmware created successfully:")
    print(output_file)
    print()


env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", merge_factory_bin)