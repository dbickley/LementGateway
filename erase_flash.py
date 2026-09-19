#!/usr/bin/env python3
"""
Script to erase ESP32-S3 flash memory completely.
Requires: pip install esptool
"""

import subprocess
import sys
import os

def erase_flash():
    """Erase the ESP32-S3 flash."""
    print("=" * 60)
    print("ESP32-S3 Flash Erase Utility")
    print("=" * 60)
    print()
    print("IMPORTANT: Follow these steps:")
    print("1. Disconnect the USB cable from the ESP32-S3")
    print("2. Wait 5 seconds")
    print("3. Hold down the BOOT button")
    print("4. Plug in the USB cable while holding BOOT")
    print("5. Wait 2 seconds then release BOOT")
    print("6. Press Enter to continue...")
    print()
    
    input("Press Enter when the board is in bootloader mode: ")

    print("\nConnecting to ESP32-S3 and erasing flash...")
    try:
        # Using esptool to wipe the 16MB chip entirely
        subprocess.run([
            sys.executable, "-m", "esptool",
            "--before", "default_reset",
            "--after", "hard_reset",
            "--chip", "esp32s3",
            "erase_flash"
        ], check=True)
        print("\nSUCCESS: Flash memory is now clean. You can now upload via PlatformIO.")
    except subprocess.CalledProcessError as e:
        print(f"\nError: Failed to erase flash. {e}")
    except Exception as e:
        print(f"\nAn unexpected error occurred: {e}")

if __name__ == "__main__":
    erase_flash()