import time
import angletest

ang = angletest.angletest()

ang.toggle_led1()

f = open('workfile','r+')
print f

while(True):
    angleBytes = ang.get_angle() #gets angle measurement in bytes
    mask = 0x3FFF
    angle = int(angleBytes[0])+int(angleBytes[1])*256 #formats to integer
    value = str(angle)
    f.write(value + ',')
    #print 'Bin: {0:016b} Hex: {1:04x} Dec: {2:0f}'.format(angle, angle, float(angle&mask)/mask*360) 
    #print in angle code in binary and hex
    #the decimal section is a fractional number between 0 and 1 
    time.sleep(.01)
    #pause for .01 amount of seconds
