from microbit import *
from utils import *
from time import sleep_us
from machine import time_pulse_us

CUTEBOT_ADDR = 0x10
LEFT_LIGHT_ADDR = 0x04
RIGHT_LIGHT_ADDR = 0x08
LEFT_WHEEL_ADDR = 0x01
RIGHT_WHEEL_ADDR = 0x02
FORWARD_FLAG = 0x02
BACKWARD_FLAG = 0x01

class Cutebot():

    def __init__(self):
        i2c.init()
        self.__pin_e = pin12
        self.__pin_t = pin8
        self.__pinL = pin13
        self.__pinR = pin14
        self.__pinL.set_pull(self.__pinL.PULL_UP)
        self.__pinR.set_pull(self.__pinR.PULL_UP)

    def set_motors_speed(self, left_wheel_speed, right_wheel_speed):
        """
        configure the speed of the left and right wheels
        :param left_wheel_speed: left wheel speed -100~100
        :param right_wheel_speed: right wheel speed -100~100
        :return: none
        """
        # Make sure the speed is within the range of -100~100
        left_wheel_speed = constrain(left_wheel_speed, -100, 100)
        right_wheel_speed = constrain(right_wheel_speed, -100, 100)
        # Set the direction of the left and right wheels
        left_dir = FORWARD_FLAG if left_wheel_speed > 0 else BACKWARD_FLAG
        right_dir = FORWARD_FLAG if right_wheel_speed > 0 else BACKWARD_FLAG
        left_wheel_speed, right_wheel_speed = abs(left_wheel_speed), abs(right_wheel_speed)
        # Send the speed and direction to the cutebot
        i2c.write(CUTEBOT_ADDR, bytearray([LEFT_WHEEL_ADDR, left_dir, left_wheel_speed, 0]))
        i2c.write(CUTEBOT_ADDR, bytearray([RIGHT_WHEEL_ADDR, right_dir, right_wheel_speed, 0]))

    def set_head_light(self, head_light, r, g, b):
        """
        configure the color of the car light
        :param light: left head light 0x04, right head light 0x08
        :param R:Red channel 0-255
        :param G:Green channel 0-255
        :param B:Blue channel 0-255
        :return:none
        """
        if r not in range(256) or g not in range(256) or b not in range(256):
            raise ValueError('Invalid RGB value')
        head_light = LEFT_LIGHT_ADDR if head_light == "left" else RIGHT_LIGHT_ADDR
        i2c.write(CUTEBOT_ADDR, bytearray([head_light, r, g, b]))

    def get_distance(self, unit = "cm"):
        """
        read the distance of the ultrasonic sensor
        :param unit: distance unit cm or inch
        :return: distance
        """
        self.__pin_e.read_digital()
        self.__pin_t.write_digital(1)
        sleep_us(10)
        self.__pin_t.write_digital(0)
        ts = time_pulse_us(self.__pin_e, 1, 25000)

        distance = round(ts * 34 / 2 / 1000)
        if unit == "cm":
            return distance
        elif unit == "inch":
            return round(distance/30.48, 2)
        else:
            raise ValueError('Invalid Unit')
        
    def get_tracking(self):
        """
        return the current tracking status
        :return:00 -> all in white
                10 -> left in black right in white
                01 -> left in white right in black
                11 -> all in black
        """
        left = self.__pinL.read_digital()
        right = self.__pinR.read_digital()
        if left == 1 and right == 1:
            return 00
        elif left == 0 and right == 1:
            return 10
        elif left == 1 and right == 0:
            return 1
        elif left == 0 and right == 0:
            return 11
        else:
            print("Unknown ERROR")

    def set_servo(self, servo, angle):
        """
        select the servo and set the angle/speed
        Args:
            servo (number): select which servo to control 1,2
            angle (number): set the angle 0~180 degrees
        """
        if servo > 2 or servo < 1:
            raise ValueError('select servo error,1,2')
        if angle > 180 or angle < 0:
            raise ValueError('angle error,0~180')
        i2c.write(CUTEBOT_ADDR, bytearray([servo + 4, angle, 0, 0]))

