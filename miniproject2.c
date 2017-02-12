#include <p24FJ128GB206.h>
#include <stdint.h>
#include "config.h"
#include "common.h"
#include "usb.h"
#include "pin.h"
#include "ui.h"
#include "spi.h"
#include "timer.h"
#include "oc.h"

#define FALSE 0
#define TRUE 1

#define TOGGLE_LED1       1   // Vendor request that toggles LED 1 from on/off
#define SET_DUTY          2
#define GET_DUTY          3
#define GET_ANGLE         4
#define GET_MAGNITUDE     5
#define GET_ENC           6
#define GET_CURRENT       7
#define SET_CONTROLLER	  8
#define WALL              9
#define SET_WALL_THRESHOLD 10

#define REG_ANG_ADDR      0x3FFF
#define REG_MAG_ADDR      0x3FFE

#define SENSOR_MASK       0X3F
#define ANALOG_MASK       0XC0
#define KONSTANT          0x4

int control_state = 1; 
int wall_thresh = 19000;

//uint16_t val1, val2;

//void ClassRequests(void) {
//    switch (USB_setup.bRequest) {
//        default:
//            USB_error_flags |= 0x01;                    // set Request Error Flag
//    }
//}

_PIN *ANG_SCK, *ANG_MISO, *ANG_MOSI;
_PIN *ANG_NCS;
_PIN *CURR_P;

WORD enc_read_reg(WORD address) {
    WORD cmd, angle;
    cmd.w = 0x4000|address.w; //set 2nd MSB to 1 for a read
    cmd.w |= parity(cmd.w)<<15; //calculate even parity for

    pin_clear(ANG_NCS);
    spi_transfer(&spi1, cmd.b[1]);
    spi_transfer(&spi1, cmd.b[0]);
    pin_set(ANG_NCS);

    pin_clear(ANG_NCS);
    angle.b[1] = spi_transfer(&spi1, 0);
    angle.b[0] = spi_transfer(&spi1, 0);
    angle.b[0]&SENSOR_MASK;
    pin_set(ANG_NCS);
    return angle;
}

//////////////////////////////Get & Set Functions ////////////////////////////
void set_wall_threshold(int anglepos){
    wall_thresh = anglepos; 
    return; 
}

int get_wall_threshold(){
    return wall_thresh; 
}

int get_speed(){
    return; 
}

int get_encoder_val_angle(void){
  /*Inputs:  None
  Outputs:  Encoder value
  see GET_ENC?
  Gets encoder value from the angle address
  */
  WORD angle = enc_read_reg((WORD)REG_ANG_ADDR);
  return (int)angle.b[0]+(int)angle.b[1]*256; 
}

int get_encoder_val_mag(void){
  /*Inputs:  None
  Outputs:  Encoder value
  see GET_ENC?
  Gets encoder value from the magnitute address
  */
  WORD angle = enc_read_reg((WORD)REG_MAG_ADDR);
  return (int)angle.b[0]+(int)(angle.b[1])*256; 
}

int encoder_to_angle(int encodervalue) {

    // Takes number from encoder, outputs an angle
    int angle = (encodervalue - 16167)/(-47);  // Actual value is (x-16167)/46.59
    return angle;
}
//////////////////////////////Calculating Functions ////////////////////////////
int set_zero_point(){
  /* To be run during initialization - set the zero to be wherever the joystick is at startup
  */
  int zero_pt_enc, zero_pt; 
  zero_pt_enc = get_encoder_val_angle();
  zero_pt = encoder_to_angle(zero_pt_enc);
  return zero_pt;
}


void set_pwm_duty(int duty){
  // Need to edit to take signed int for duty and no boolean.
  if (duty>0){
    pin_write(&D[7], duty);
    pin_write(&D[8], 0x0);
  }
  else if (duty<0){
    pin_write(&D[8], -1*duty);
    pin_write(&D[7], 0x0);
  }
  else {
    pin_write(&D[8], 0x0);
    pin_write(&D[7], 0x0);
  }
}

int relative_angle(int calc_angle, int initial_angle){
  int actual_diff;
  int raw_diff = calc_angle-initial_angle;
  if (abs(raw_diff)>100){
    actual_diff = raw_diff - 360;
    return actual_diff;
  }
  return raw_diff;
}

int deriv_calcs(s1,s2,t) {
  // Takes two values and the time between them, and outputs the derivative
  // Can calculate speed (from two positions) and acceleration (from two speeds)
  int deriv = (s2 - s1)/t;
  return deriv;
}

float calc_torque(){
    WORD vout;
    float realvout;
    float current;
    float torque;

    vout.w = pin_read(CURR_P); //reads analog pin
    vout.b[1]&ANALOG_MASK; //masks last 6 digits
    vout.b[0] = vout.b[0]*256; 
    realvout = vout.b[0]+vout.b[1]; //combines bytes into integer
    realvout = (realvout*3.3)/65535; //normalization
    current = (realvout - 1.6) * 0.075;
    torque = KONSTANT * current;
    return torque;
}

int pwm_control(int ideal, float real, int duty_cycle){
  int diff_torque;
  int new_duty;
  int constant_p = (1/3);
  diff_torque = ideal - real;
  new_duty = duty_cycle + (constant_p * diff_torque);
  return new_duty;

}

/*
Still need:
replacing vendor requests
Vout -> current math
Current -> torque math
compare torque real to torque Desiredi
increase or decrease or sustain pwm duty cycle
*/

///////////////////////Control Type Calculators/////////////////////////////////
/*
Inputs:  Speed, position
Outputs:  Desired torque

Need:
* Wall
* Spring
* damper
* Texture
*/


int wall_control(int position){
    // input current angle, ouput desired torque (PWM) 
    float torque = calc_torque(); 
    int ideal, pwm, threshold; 
    WORD duty;
    threshold = get_wall_threshold();
    duty.w = pin_read(&D[7]); 
    if (duty.w == 0x0){
      duty.w = pin_read(&D[8]);
    }
    pwm = duty.b[0]+duty.b[1]*256; //combines bytes into integer
    if (position >= threshold){
	   ideal = 30; //set to "safe" max torque, 30/ 42.4 
    } 
    pwm = pwm_control(ideal, torque, pwm);
    //pwm = 0x8000; 
    return pwm;
}

int spring_control(int position, int k, int setpt){
  /*Inputs:  angular position, spring constant. zero set point maybe?
  torque proportinal to -position
  Output:  Desired torque
  */
  return 0;
}
int damper_control(int speed, int k){
  /* inputs:  angular speed, damping coefficient
  Torque proportional to -speed
  Output:  Desired Torque
  */
  return 0; 
}

int texture_control(int position, int speed, int bump_width){
  /*Need to figure out what we want for torque.  Right now, switching every 5 degrees
  May need to edit the direction vector
  */
  int effective_pos = position%(bump_width*2);
  float torque;
  int direction = speed/abs(speed);
  if (effective_pos %  (bump_width*2) == 0){ // if going over the edge of a bump
    torque = 0.5 * direction;
  }
  else if (effective_pos %  (bump_width*2) == 5){
    torque = -0.5 * direction;
  }
  else if (effective_pos < bump_width){
    torque = 0;
  }
  else if (effective_pos > bump_width){
    torque = 0.25 * direction;
  }
  return torque;
}



void VendorRequests(void) {
    WORD32 address;
    WORD temp;
    WORD angle;

    switch (USB_setup.bRequest) {
        case TOGGLE_LED1:
            led_toggle(&led1);
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case SET_DUTY:
            pin_write(&D[7], (uint16_t)USB_setup.wValue.w);
            pin_write(&D[8], 0x0);
      // below needed to finish all vendor requests
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case GET_DUTY:
            temp.w = pin_read(&D[7]);
            BD[EP0IN].address[0] = temp.b[0];
            BD[EP0IN].address[1] = temp.b[1];
            BD[EP0IN].bytecount = 2;    // set EP0 IN byte count to 2
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case GET_ANGLE:
            angle = enc_read_reg((WORD)REG_ANG_ADDR);
            angle.b[0]&SENSOR_MASK;
            BD[EP0IN].address[0] = angle.b[0];
            BD[EP0IN].address[1] = angle.b[1];
            BD[EP0IN].bytecount = 2;    // set EP0 IN byte count to 2
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case GET_MAGNITUDE:
            angle = enc_read_reg((WORD)REG_MAG_ADDR);
            angle.b[0]&SENSOR_MASK;
            BD[EP0IN].address[0] = angle.b[0];
            BD[EP0IN].address[1] = angle.b[1];
            BD[EP0IN].bytecount = 2;    // set EP0 IN byte count to 2
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case GET_ENC:
            angle = enc_read_reg(USB_setup.wValue);
            angle.b[0]&SENSOR_MASK;
            BD[EP0IN].address[0] = angle.b[0];
            BD[EP0IN].address[1] = angle.b[1];
            BD[EP0IN].bytecount = 2;    // set EP0 IN byte count to 2
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;

      /*case GET_CONTROL:
            Define what control it's going to be.
      */

        case SET_CONTROLLER: 
            control_state = (int)USB_setup.wValue.w;  //changing global variable
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case WALL: 
            control_state = 1;
	    wall_control(19000); 
	    set_pwm_duty(0x8000);
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
	    break;
        case SET_WALL_THRESHOLD: 
            set_wall_threshold((int)USB_setup.wValue.w);
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
	    break;
        default:
            USB_error_flags |= 0x01;    // set Request Error Flag
    }
}

void VendorRequestsIn(void) {

    switch (USB_request.setup.bRequest) {
        default:
            USB_error_flags |= 0x01;                    // set Request Error Flag
    }
}

void VendorRequestsOut(void) {
    switch (USB_request.setup.bRequest) {
        default:
            USB_error_flags |= 0x01;                    // set Request Error Flag
    }
}
/////////////////////////////Main Function////////////////////////////////////

int16_t main(void) {
    init_clock();
    init_ui();
    init_pin();
    init_oc();
    init_spi();

    ANG_MOSI = &D[0];
    ANG_MISO = &D[1];
    ANG_SCK = &D[2];
    ANG_NCS = &D[3];
    CURR_P = &A[0];

    pin_analogIn(CURR_P);
    pin_digitalOut(ANG_NCS);
    pin_set(ANG_NCS);

    spi_open(&spi1, ANG_MISO, ANG_MOSI, ANG_SCK, 2e6, 1); //WHY ONE

    oc_pwm(&oc1, &D[7], NULL, 10e3, 0x8000);  // Pin, internal vs external timer, frequency, initial duty cycle
    oc_pwm(&oc2, &D[8], NULL, 10e3, 0x0);

    InitUSB();

    int angle,duty;                               // initialize the USB registers and serial interface engine
    while (USB_USWSTAT!=CONFIG_STATE) {     // while the peripheral is not configured...
        ServiceUSB();                       // ...service USB requests
    }
    while (1) {
        ServiceUSB();                       // service any pending USB requests
        // variable:  control state
        angle = get_encoder_val_angle(); 
	      angle = encoder_to_angle(angle); 
        set_wall_threshold(2500);
          switch (control_state){
            case 0: //No controller
              break;
            case 1: // Wall
              duty = wall_control(angle); 
              break;
            default: // No controller
	       duty = wall_control(angle); 
              break;
          }
	pin_write(&D[7], duty);
    	pin_write(&D[8], 0x0);
        // using if statement or similar: check control state, run relevant control calculator
        // Calculate the proper PWM from torque stuff
        // Set variables we need next loop:
          //Current position, current time, current PWM

    }
}

