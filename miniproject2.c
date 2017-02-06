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

#define REG_ANG_ADDR      0x3FFF
#define REG_MAG_ADDR      0x3FFE

//uint16_t val1, val2;

//void ClassRequests(void) {
//    switch (USB_setup.bRequest) {
//        default:
//            USB_error_flags |= 0x01;                    // set Request Error Flag
//    }
//}

_PIN *ANG_SCK, *ANG_MISO, *ANG_MOSI;
_PIN *ANG_NCS;

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
    pin_set(ANG_NCS);
    return angle;
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
            BD[EP0IN].address[0] = angle.b[0];
            BD[EP0IN].address[1] = angle.b[1];
            BD[EP0IN].bytecount = 2;    // set EP0 IN byte count to 2
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case GET_MAGNITUDE:
            angle = enc_read_reg((WORD)REG_MAG_ADDR);
            BD[EP0IN].address[0] = angle.b[0];
            BD[EP0IN].address[1] = angle.b[1];
            BD[EP0IN].bytecount = 2;    // set EP0 IN byte count to 2
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case GET_ENC:
            angle = enc_read_reg(USB_setup.wValue);
            BD[EP0IN].address[0] = angle.b[0];
            BD[EP0IN].address[1] = angle.b[1];
            BD[EP0IN].bytecount = 2;    // set EP0 IN byte count to 2
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
      /*case GET_CONTROL:
            Define what control it's going to be.
      */
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

//////////////////////////////Calculating Functions ////////////////////////////
int set_zero_point(){
  /* To be run during initialization - set the zero to be wherever the joystick is at startup
  */
}

void set_pwm_duty(bool forward, uint16_t duty){
  if (forward){
    pin_write(&D[7], duty);
    pin_write(&D[8], 0x0);
  }
  else{
    pin_write(&D[8], duty);
    pin_write(&D[7], 0x0);
  }
}

int get_encoder_val(void){
  /*Inputs:  None
  Outputs:  Encoder value
  see GET_ENC?
  */
}

int encoder_to_angle(int encodervalue) {
    // Takes number from encoder, outputs an angle
    int angle = (encodervalue - 16167)/(-47);  // Actual value is (x-16167)/46.59
    return angle;
}

int derivcalcs(s1,s2,t) {
  // Takes two values and the time between them, and outputs the derivative
  // Can calculate speed (from two positions) and acceleration (from two speeds)
  int deriv = (s2 - s1)/t;
  return deriv
}

int calc_torque(){
  // Reads and calculates torque
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
  // Implement this.  Torque happens at specific location
  int torque
  return torque
}

int spring_control(int position, int k, /*int setpt*/){
  /*Inputs:  angular position, spring constant. zero set point maybe?
  torque proportinal to -position
  Output:  Desired torque
  */
}
int damper_control(int speed, int k){
  /* inputs:  angular speed, damping coefficient
  Torque proportional to -speed
  Output:  Desired Torque
  */
}

int texture_control(int position){
  /*Need to figure out what we want for torque.  Masking binary?
  */
}

int pwm_control(int ideal, int real, int PWM_current){
  /* inputs:  torque Ideal, torque real, torque PWM_current
  Outputs:  new PWM duty cycle
  */

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

    pin_digitalOut(ANG_NCS);
    pin_set(ANG_NCS);

    spi_open(&spi1, ANG_MISO, ANG_MOSI, ANG_SCK, 2e6, 1); //WHY ONE

    oc_pwm(&oc1, &D[7], NULL, 10e3, 0x8000);  // Pin, internal vs external timer, frequency, initial duty cycle
    oc_pwm(&oc2, &D[8], NULL, 10e3, 0x0);

    InitUSB();                              // initialize the USB registers and serial interface engine
    while (USB_USWSTAT!=CONFIG_STATE) {     // while the peripheral is not configured...
        ServiceUSB();                       // ...service USB requests
    }
    while (1) {
        ServiceUSB();                       // service any pending USB requests
        // variable:  control state
        //get angle
        // get torque
        // get speed
        // using if statement or similar: check control state, run relevant control calculator
        // Calculate the proper PWM from torque stuff
        // Set variables we need next loop:
          //Current position, current time, current PWM
    }
}

