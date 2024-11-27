from microbit import *
from joystick_utils import *
from cutebot_utils import *
from microbot_utils import *
from utils import *
import radio
import machine

joystick_id = b')\xdc2L\x95M\xdc\x8d'
cutebot_id = b'\x02\xd3\xc4\xe7xr\xe9\xce'
microbot_id = b'\x84\x02;\x03\xb7,\x90\x7f'

cur_id = machine.unique_id()

# Determine role based on unique device ID
if cur_id == joystick_id:
    role = "joystick"
elif cur_id == cutebot_id:
    role = "cutebot"
elif cur_id == microbot_id:
    role = "microbot"
else:
    role = None#invalid device
    print("Invalid device")

# Radio configuration
radio.on()
j2c_channel = 1#joystick to cutebot channel
c2m_channel = 2#cutebot to mbot channel

if role == "joystick":
    joystick = JOYSTICKBIT()
    joystick.init_joystick_bit()
    radio.config(group=j2c_channel)
    while True:
        x = joystick.get_rocker_value(RockerType.X)
        y = joystick.get_rocker_value(RockerType.Y)
        x_vel,y_vel = map_value(x), map_value(y)
        print("Raw x:{} Mapped x:{} Raw y:{} Mapped y:{}".format(x,x_vel,y,y_vel))
        msg = "{},{}".format(x_vel, y_vel)
        radio.send(msg)

elif role == "cutebot":
    radio.config(group=j2c_channel)
    cutebot = Cutebot()
    while True:
        msg = radio.receive()
        if msg:
            x,y = msg.split(",")
            x,y = int(x), int(y)
            print("Received x:{} y:{}".format(x,y))
            left_wheel_speed, right_wheel_speed =  y - x, y + x
            print("Left Wheel Speed:{} Right Wheel Speed:{}".format(left_wheel_speed,right_wheel_speed))
            cutebot.set_motors_speed(left_wheel_speed, right_wheel_speed)

elif role == "microbot":
    radio.config(group=j2c_channel)
    microbot = Microbot()
    microbot.enable()
    while True:
        msg = radio.receive()
        if msg:
            x,y = msg.split(",")
            x,y = int(x), int(y)
            print("Received x:{} y:{}".format(x,y))
            left_wheel_speed, right_wheel_speed =  y - x, y + x
            print("Left Wheel Speed:{} Right Wheel Speed:{}".format(left_wheel_speed,right_wheel_speed))
            microbot.left_motor(invert=True).forward(left_wheel_speed)
            microbot.right_motor(invert=True).forward(right_wheel_speed)
else:
    print("Invalid device")
    display.scroll("Invalid device")
    sleep(1000)
    display.clear()

    
