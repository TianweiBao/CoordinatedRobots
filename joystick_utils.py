from microbit import *
import utime 
  
class Joystick:
      p_buzzer = 0
      p_x = 1
      p_y = 2       
      p_btnC = 12  
      p_btnD = 13  
      p_btnE = 14  
      p_btnF = 15  
      p_vibration_motor = 16

class RockerType:  
      X = 1  
      Y = 2  

#Assume High and Low of ButtonType correspond to high and low levels on the pin
ButtonType = {  
      'down': 0,  # low level indicates pressed  
      'up': 1     # high level indicates released  
}  

class JOYSTICKBIT():
      def __init__(self):
            pass

      def init_joystick_bit(self):  
            """initialization joystick bit
            P0 is set to low level (assuming this is used for other purposes, such as initializing LED or other functions)
            """ 
            pin0.write_digital(0)
            
            #P12, P13, P14, P15 are set to input and pull-up mode  
            pin12.set_pull(pin12.PULL_UP) 
            pin13.set_pull(pin13.PULL_UP) 
            pin14.set_pull(pin14.PULL_UP)   
            pin15.set_pull(pin15.PULL_UP)  
            
            #Vibration motor control pin output high level, default off state  
            pin16.write_digital(1) 
      
      def get_button(self, button):  
            """ 
            get button status
            button: pass in the button number (P12, P13, P14, P15)
            check if the corresponding button number is pressed
            if the button is pressed, return True, otherwise return False
            """
            pin = 1
            if button == Joystick.p_btnC:
                pin12.set_pull(pin12.PULL_UP)
                pin = pin12.read_digital()
            elif button == Joystick.p_btnD:
                pin13.set_pull(pin13.PULL_UP)
                pin = pin13.read_digital()
            elif button == Joystick.p_btnE:
                pin14.set_pull(pin14.PULL_UP)
                pin = pin14.read_digital()
            elif button == Joystick.p_btnF:
                pin15.set_pull(pin15.PULL_UP)
                pin = pin15.read_digital()
            # return True if the button is pressed (low level), otherwise return False
            return not pin 
                
      def on_button_event(self, button, event_type, handler):  
            """ 
            button: pass in the button number (P12, P13, P14, P15)
            event_type: pass in the event type (down, up)
            handler: pass in the handler function
            check the status of the corresponding button number has met the event type,
            if the event type is met, execute the handler function
            """  
            if JOYSTICKBIT.get_button(self,button) == (event_type == ButtonType['down']):  
                  handler()  
         
      def get_rocker_value(self, rocker):  
            """ 
            # Get the value of the rocker (use ADC to read the analog value)
            # Assume ADC channel 0 is connected to the X (P1) and Y (P2) of the joystick
            # adc = display.scroll(pin0.read_analog())
            rocker: pass in the rocker type (X, Y)
            """   
            if rocker == RockerType.X:    
                  return pin1.read_analog()   
            elif rocker == RockerType.Y:    
                  return pin2.read_analog()   
            else:  
                  return 0  
       
      def vibration_motor(self, time_ms): 
            """ 
            control the vibration motor to vibrate for the specified time
            time_ms: pass in the time in milliseconds
            """   
            pin16.write_digital(0) # Start the vibration motor  
            utime.sleep_ms(time_ms)  # Vibrate for time_ms milliseconds 
            pin16.write_digital(1)  # Stop the vibration motor  
            
            # example:  
            # init_joystick_bit()  
            # print(get_button(JoystickBitPin.P12))  
            # on_button_event(JoystickBitPin.P13, ButtonType['down'], lambda: display.show(Image.YES)))  
            # print(get_rocker_value(RockerType.X))  
            # vibration_motor(500)  # Vibrate for 500 milliseconds



# joystickbit = JOYSTICKBIT()


# if __name__ == '__main__':
     
#       joystickbit.init_joystick_bit()
      # JOYSTICKBIT.get_button(JoystickBitPin.P12)
      # JOYSTICKBIT.on_button_event(JoystickBitPin.P12,ButtonType['down'],lambda: display.show(Image.YES))
      # JOYSTICKBIT.get_rocker_value(RockerType.X)
      # JOYSTICKBIT.vibration_motor(100)