import time
import angletest

ang = angletest.angletest()

ang.toggle_led1()

#while(True):
angleBytes = ang.get_angle() #gets angle measurement in bytes
angle = int(angleBytes[0])+int(angleBytes[1])*256 #formats to integer
print 'Bin: {0:016b} Dec: {0:0d}'.format(angle) 
#print in angle code in binary and hex
#the decimal section is a fractional number between 0 and 1 
#    time.sleep(.01)
    #pause for .01 amount of seconds
