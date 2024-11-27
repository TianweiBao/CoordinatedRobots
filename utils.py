from microbit import *

def scan_i2c():
    """
    Scans for i2c devices and prints their addresses.
    """
    devices = i2c.scan()
    if devices:
        for device in devices:
            print(hex(device))
    else:
        print("No i2c device found")

def map_value(value, from_low=0, from_high=1023, to_low=-100, to_high=100):
    """
    Re-maps a number from one range to another.
    """
    return int(to_low + (value - from_low) * (to_high - to_low) / (from_high - from_low))

def constrain(value, min_value, max_value):
    """
    Constrains a number to be within a range.
    """
    return max(min_value, min(value, max_value))