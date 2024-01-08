from microbit import *
import array
from keyes_mecanum_Car_V2 import *
mecanumCar = Mecanum_Car_Driver_V2()
display.off()
## this is required PIN10 is shared with LED Display
## The sensors are very sensitive to the black line
## it cannot have any reflectiveness

## the cabling provided only allowed me to connect pins 3,4,10
## val_R = pin10.read_digital()
## val_C = pin4.read_digital()
## val_L = pin3.read_digital()

## No sensitivity for analog input

val_array = array.array('i',[0] * 17)
print(val_array)
val_L = 0
val_C = 0
val_R = 0

while True:
    '''
    val_array[1] = pin1.read_analog()
    val_array[2] = pin2.read_analog()
    val_array[3] = pin3.read_analog()
    val_array[4] = pin4.read_analog()
    '''
    #val_array[5] = pin5.read_analog()
    #val_array[6] = pin6.read_analog()
    #val_array[7] = pin7.read_analog()
    #val_array[8] = pin8.read_analog()
    #val_array[9] = pin9.read_analog()
    '''
    val_array[10] = pin10.read_analog()
    '''
    #val_array[11] = pin11.read_analog()
    #val_array[12] = pin12.read_analog()
    #val_array[13] = pin13.read_analog()
    '''
    val_array[14] = pin14.read_digital()
    val_array[15] = pin15.read_digital()
    '''
    #val_array[16] = pin16.read_digital()
    
    val_R = pin10.read_digital()
    val_C = pin4.read_digital()
    val_L = pin3.read_digital()
    print('L:',val_L)
    print('R:',val_R)
    print('C:',val_C)
    
    ##print(val_array)
    sleep(1000)
    
