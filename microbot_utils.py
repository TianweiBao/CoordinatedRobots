from microbit import i2c, display, Image


class Microbot():
    '''Initialize moto:bit hardware.
    '''
    I2C_ADDR = 0x59         # 89 in decimal
    CMD_ENABLE = 0x70       # 112 in decimal
    CMD_SPEED_LEFT = 0x21   # 33 in decimal
    CMD_SPEED_RIGHT = 0x20  # 32 in decimal

    def __init__(self):
        # cool code that you'll write
        # lol I can't give you all the answers
        pass

    def enable(self):
        '''Enable motor driver.
        '''
        i2c.write(self.I2C_ADDR, bytes([self.CMD_ENABLE, 0x01]))

    def disable(self):
        '''Disable motor driver.
        '''
        i2c.write(self.I2C_ADDR, bytes([self.CMD_ENABLE, 0x00]))

    def drive(self, speed_left, speed_right):
        '''Drive motors continuously based on 100 point scale.
        Args:
            speed_left (int|float): Motor power value. [-100, 100]
            speed_right (int|float): Motor power value. [-100, 100]
        '''
        speeds = [speed_left, speed_right]
        # cool code that you'll write
        # lol think again, no free answers here
        i2c.write(self.I2C_ADDR, bytes([self.CMD_SPEED_LEFT, speeds[0]]))
        i2c.write(self.I2C_ADDR, bytes([self.CMD_SPEED_RIGHT, speeds[1]]))


