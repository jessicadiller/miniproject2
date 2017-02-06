import time
import angletest

def encodertoangle(encodervalue):
    ### Takes in value from the encoder, and outputs the angle that the magnet
    ### is with respect to the encoder, in degrees
    angle = (encodervalue - 16167)/(-46.59167)
    return angle

def derivcalc(s1,s2, t):
    """ Inputs:  Two angles and a time
        Outputs:  Velocity, in degrees/s
    """
    return (s2-s1)/t

def get_encoder_value():
    """
    Inputs: None
    Outputs: Encoder Value
    """
    ang = angletest.angletest()

    # ang.toggle_led1()
    # ang.set_duty(0xFFFF)
    # time.sleep(1)
    # ang.set_duty(0x0)

    # f = open('workfile','r+')
    # print f
    angleBytes = ang.get_angle() #gets angle measurement in bytes
    mask = 0x3FFF
    angle = int(angleBytes[0])+int(angleBytes[1])*256 #formats to integer
    # value = str(angle)
    # f.write(value + '; ')
    #print 'Bin: {0:016b} Hex: {1:04x} Dec: {2:0f}'.format(angle, angle, float(angle&mask)/mask*360) 
    #print in angle code in binary and hex
    #the decimal section is a fractional number between 0 and 1
    # time.sleep(.001)
    #pause for .001 amount of seconds
    return angle

def encodercalculation(angle1,t):
    """Pass in the last angle and time between points.  Output new angle and velocity."""
    angle2 = encodertoangle(get_encoder_value())
    velocity = derivcalc(angle1,angle2,t)
    return [angle2,velocity]