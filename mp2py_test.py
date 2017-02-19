import time
import miniproject2 as mp

at = mp.jtest()

def angle_get():
    measured_angle = at.get_angle()
    #print "measured angle:"
    #print measured_angle
    #print "{0:b}".format(measured_angle[0])
    #print "{0:b}".format(measured_angle[1])
    int_angle = int(measured_angle[0]) + int(measured_angle[1])*256
    #print "measured angle integer:"
    #print int_angle
    position = (int_angle - 16167) / 46.59
    print "position:"
    print position
    return position;

def speed_get(): 
    a1 = angle_get()
    t1 = time.clock()
    a2 = angle_get()
    t2 = time.clock()
    print a2 - a1
    speed = (a2 - a1)/(t2 - t1)
    angle = a2
    return [speed, angle]


def torque_get():
    at.toggle_led1()
    #GET CURRENT
    analog = at.get_current()
    #print "get_current output:"
    #print analog
    
    #TORQUE CALC
    measuredvout = int(analog[0])+int(analog[1])*256 #formats to integer
    #print "measured integer:"
    #print measuredvout
    fraction = (float(measuredvout) / 65535)
    #print "vout fraction:"
    #print fraction
    normalized = fraction * 3.3
    #print "vout actual:"
    #print normalized
    current = (normalized - 1.6) / 0.75
    #print "current calculation:"
    #print current
    real = 4 * current
    print "real torque:"
    print real
    return real;

def pwm_control(realT, idealT, change):
    #print "pwm control function"
    if change == False:
        print "no change in pwm"
        return;
    at.toggle_led2()
    diffT = idealT - realT
    print "diff:"
    print diffT
    new_duty = diffT/(3 * idealT)
    print "new_duty:"
    print new_duty
    print "{0:b}".format(int(new_duty))
    #NEED TO FIGURE OUT HOW TO GO FROM THIS FRACTIONAL DECIMAL FLOAT TO BINARY FIXED POINT
    if new_duty > 0:
        at.set_duty_f(new_duty)
        print "set duty forward"
    elif new_duty <0:
        at.set_duty_r(new_duty * -1)
        print "set duty reverse"
    else: 
        at.set_duty_f(0)
        print "set duty 0"
    return;

def wall_control(position):
    # input current angle, ouput desired torque (PWM) 
    torque = torque_get() 
    threshold_r = -160 
    threshold_f = -130
    if (position <= threshold_r):
	    ideal = 1; #set to "safe" max torque, 30/ 42.4 
        control = True
        print "past threshold r"
        at.set_duty_f(0xF000)
    elif (position >= threshold_f):
	    ideal = -1; #set to "safe" max torque, 30/ 42.4 
        control = True
        print "past threshold f"
        at.set_duty_r(0xF000)
    else: 
        ideal = 0
        control = False
        print "not past threshold"
        at.set_duty_f(0)
    #print "control:"
    #print control

    #pwm_control(torque, ideal, control);

    #int ideal, pwm, threshold; 
    # threshold = get_wall_threshold(); // in degrees
    #duty.w = pin_read(&D[7]); 
    #if (duty.w == 0x0){
    #  duty.w = pin_read(&D[8]);
    #}
    #pwm = duty.b[0]+duty.b[1]*256; //combines bytes into integer
     #pin_write(&D[8], 0x0);
     #pin_write(&D[7], 0x0);
    #} 
    return;

#spring_control
#damper_control
#texture_control

#call functions
#t = 1
#while t < 1000000000000000000000000:
while True:
    position = angle_get()
# [speed, position] = speed_get() 
# print "speed "
# print speed
# print "; position "
# print position
    wall_control(position)
    #t = t+1



