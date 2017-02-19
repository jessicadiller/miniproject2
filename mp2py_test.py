import time
import miniproject2 as mp

at = mp.jtest()

def angle_get():
    sum_angle = 0; 
    for i in range(0,20):
        measured_angle = at.get_angle()
        # print "measured angle: ",measured_angle
        # print "{0:b}".format(measured_angle[0])
        # print "{0:b}".format(measured_angle[1])
        int_angle = int(measured_angle[0]) + int(measured_angle[1])*256
        # print "measured angle integer: ",int_angle
        sum_angle += int_angle
    avg_angle = sum_angle/20
    # print "avg: ", avg_angle
    position = (avg_angle - 16167) / 46.59
    print "position: ",position
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
    #print "get_current output: ",analog
    
    #TORQUE CALC
    measuredvout = int(analog[0])+int(analog[1])*256 #formats to integer
    #print "measured integer: ",measuredvout
    fraction = (float(measuredvout) / 65535)
    #print "vout fraction: ",fraction
    normalized = fraction * 3.3
    #print "vout actual: ",normalized
    current = (normalized - 1.6) / 0.75
    #print "current calculation: ",print current
    real = 4 * current
    print "real torque:, ",real
    return real;

def duty_frac_converter(num):
    # Takes in a positive float, outputs the fractional part as an int between 0
    # and 2^16, where 2^16 = 1)
    if num>1:
        frac_num = 0.99999
    else:
        frac_num = num
    exponent=0
    shifted_num=frac_num
    while shifted_num != int(shifted_num) and exponent<16:
        shifted_num*=2
        exponent+=1
    # print(shifted_num)
    # print(int(shifted_num))
    new_shifted_num = shifted_num*(2**(16-exponent))
    return new_shifted_num

def pwm_control(realT, idealT, change):
    #print "pwm control function"
    # if change == False:
    #     print "no change in pwm"
    #     return;
    ### Commented out if change == False because having that means that PWM will
    #### always be on?  Let's talk about this.
    at.toggle_led2()

    diffT = idealT - realT

    print "diff:",diffT
    fraction = diffT/30
    print "fraction: ",fraction

    dutyf = at.get_duty_f()
    print "dutyf: ",dutyf
    dutyr = at.get_duty_r()
    print "dutyr: ",dutyr
    duty = dutyf - dutyr
    print "total duty: ",duty

    new_duty = duty + abs(duty) * fraction
    if new_duty > 2**16-1:
        new_duty = 2**16-1
    elif new_duty<-(2**16-1):
        new_dut = -(2**16-1)
    #NEGATIVE VALUE FOR PWM JUST GET BIGGER AND BIGGER
    print "new_duty: ",new_duty
    print "{0:b}".format(int(new_duty))
    if new_duty > 0:
        at.set_duty_f(new_duty)
        print "set duty forward"
    elif new_duty <0:
        at.set_duty_r(new_duty *-1)

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
	ideal = 1 #set to "safe" max torque, 30/ 42.4 
        control = True
        print "past threshold r"
        # at.set_duty_f(0xF000)
    elif (position >= threshold_f):
	ideal = -1 #set to "safe" max torque, 30/ 42.4 
        control = True
        print "past threshold f"
        # at.set_duty_r(0xF000)
    else: 
        ideal = 0
        control = False
        print "not past threshold"
        # at.set_duty_f(0)
    #print "control:"
    #print control


    pwm_control(torque, ideal, control);

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
def spring_control(position):
    torque = torque_get()
    print "torque: ",torque
    ideal = (position +145) * -1.5 #FIGURE OUT ANGLE SYSTEM
    print "ideal: ",ideal
    pwm_control(torque, ideal, 1)
    print "done"
    return;


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
    #spring_control(position)

    #t = t+1



