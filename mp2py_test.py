import time
import miniproject2 as mp

at = mp.jtest()

def set_period():
    #call vendor request?
    timer = 0.001 #in seconds
    return timer;

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
    a0 = at.get_angle_speed()
    #print "a0: ",a0
    #print "{0:b}".format(a0[0])
    #print "{0:b}".format(a0[1])
    #print "{0:b}".format(a0[2])
    #print "{0:b}".format(a0[3])
    int_a1 = ((int(a0[0]) + int(a0[1])*256) - 16167)/46.59
    #print "angle 1 integer: ",int_a1
    int_a2 = ((int(a0[2]) + int(a0[3])*256) - 16167)/46.59
    #print "angle 2 integer: ",int_a2
    dif_a = int_a2 - int_a1
    #print "difference btwn angles: ",dif_a
    t = set_period() #hard code period in earlier function
    speed = dif_a / t #could just put function call here, just wanted to confirm it would work first
    print "speed: ",speed
    angle = (int_a1 + int_a2)/2
    print "average angle: ",angle
    return [speed, angle];

def torque_get():
    #at.toggle_led1()
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
    print "real torque: ",real
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
    #print "diff:",diffT
    fraction = diffT/30
    #print "fraction: ",fraction
    dutyf = at.get_duty_f()
    #print "dutyf: ",dutyf
    dutyr = at.get_duty_r()
    #print "dutyr: ",dutyr
    duty = dutyf - dutyr
    #print "total duty: ",duty
    if duty > 0:
        new_duty = duty + (2**16-1)* fraction
    else:
        new_duty = duty - (2**16-1) *fraction
    if new_duty > 2**16-1:
        new_duty = 2**16-1
    elif new_duty<-(2**16-1):
        new_duty = -(2**16-1)
    #NEGATIVE VALUE FOR PWM JUST GET BIGGER AND BIGGER -changed?
    print "new_duty: ",new_duty
    #print "{0:b}".format(int(new_duty))
    if new_duty > 0:
        at.set_duty_f(new_duty)
        #print "set duty forward"
    elif new_duty <0:
        at.set_duty_r(new_duty *-1)
        #print "set duty reverse"
    else: 
        at.set_duty_f(0)
        #print "set duty 0"
    return;

def wall_control(position):
    # input current angle, ouput desired torque (PWM) 
    torque = torque_get() 
    threshold_r = -15 
    threshold_f = 15
    if (position <= threshold_r):
        ideal = 30 #set to "safe" max torque, 30/ 42.4 
        control = True
        print "past threshold r"
        at.set_duty_f(0xF000)
    elif (position >= threshold_f):
        ideal = -30 #set to "safe" max torque, 30/ 42.4 
        control = True
        print "past threshold f"
        at.set_duty_r(0xF000)
    else: 
        ideal = 0
        control = True
        print "not past threshold"
        at.set_duty_f(0)
    #print "control:"
    #print control

    # pwm_control(torque, ideal, control);

    return;

#spring_control
def spring_control(position):
    torque = torque_get()
    ideal = (position) * -0.5 #FIGURE OUT ANGLE SYSTEM
    print "ideal: ",ideal
    pwm_control(torque, ideal, 1)
    #print "done"
    return;

#damper_control
def damper_control(speed, position):
    torque = torque_get()
    ideal = speed * -0.25 #FIGURE OUT ANGLE SYSTEM
    print "ideal: ",ideal
    pwm_control(torque, ideal, 1)
    #print "done"
    return;


#texture_control
def texture_control(position):
    # 5 deg increment "ratchet"
    rel = abs(position)%10
    print rel
    torque = torque_get()
    if rel < 5:
        ideal = 0
        at.set_duty_r(0x0000)
        print "setting zero"
    elif rel > 5: 
        ideal = 15
        at.set_duty_f(0xF000)
        print "bump"
    else: 
        ideal = 15
    # pwm_control(torque, ideal, control)
    return; 


# call functions

#while t < 1000000000000000000000000:
#[speed, position] = speed_get()
#position = angle_get()
#damper_control(speed, position)
while True:
    #position = angle_get()
    #position = position - starting_position
    #if position > 180:
    #    position -= 360
    #elif position < -180:
    #    position += 360
    #print('relative position:', position)
    [speed, position] = speed_get() 
    damper_control(speed, position)
    #wall_control(position)
    #spring_control(position)

    #texture_control(position)

    #t = t+1



