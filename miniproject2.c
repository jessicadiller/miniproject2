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

#define TOGGLE_LED1       1   // Vendor request that toggles LED 1 from on/off
#define TOGGLE_LED2       2
#define TOGGLE_LED3       3
#define SET_DUTY_F        4
#define SET_DUTY_R        5
#define GET_DUTY_F        6
#define GET_DUTY_R        7
#define GET_ANGLE         8
#define GET_ANGLE_SPEED   9
#define GET_CURRENT       10

#define REG_ANG_ADDR      0x3FFF
#define REG_MAG_ADDR      0x3FFE

#define SENSOR_MASK       0X3F
#define ANALOG_MASK       0XC0

WORD angle, old_angle;
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

WORD enc_read_reg(WORD registeraddress) {
    WORD cmd;
    cmd.w = 0x4000|registeraddress.w; //set 2nd MSB to 1 for a read
    cmd.w |= parity(cmd.w)<<15; //calculate even parity for

    pin_clear(ANG_NCS);
    spi_transfer(&spi1, cmd.b[1]);
    spi_transfer(&spi1, cmd.b[0]);
    pin_set(ANG_NCS);

    pin_clear(ANG_NCS);
    angle.b[1] = spi_transfer(&spi1, 0);
    angle.b[0] = spi_transfer(&spi1, 0);
    pin_set(ANG_NCS);
    angle.b[1] = angle.b[1]&0b00111111; //MASK
    return;
}

void VendorRequests(void) {
    WORD32 address;
    WORD temp;
    WORD vout;
    
    switch (USB_setup.bRequest) {
        case TOGGLE_LED1:
            led_toggle(&led1);
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case TOGGLE_LED2:
            led_toggle(&led2);
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case TOGGLE_LED3:
            led_toggle(&led3);
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case SET_DUTY_F:
            pin_write(&D[7], (uint16_t)USB_setup.wValue.w);
            pin_write(&D[8], 0x0);
	    // below needed to finish all vendor requests
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0 
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case SET_DUTY_R:
            pin_write(&D[7], 0x0);
            pin_write(&D[8], (uint16_t)USB_setup.wValue.w);
	    // below needed to finish all vendor requests
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0 
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case GET_DUTY_F:
            temp.w = pin_read(&D[7]);
            BD[EP0IN].address[0] = temp.b[0];
            BD[EP0IN].address[1] = temp.b[1];
            BD[EP0IN].bytecount = 2;    // set EP0 IN byte count to 2
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case GET_DUTY_R:
            temp.w = pin_read(&D[8]);
            BD[EP0IN].address[0] = temp.b[0];
            BD[EP0IN].address[1] = temp.b[1];
            BD[EP0IN].bytecount = 2;    // set EP0 IN byte count to 2
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case GET_ANGLE:
            enc_read_reg((WORD)REG_ANG_ADDR);
            //angle0.b[1] = angle0.b[1]&0b00111111;
            // angle1 = enc_read_reg((WORD)REG_ANG_ADDR);
            // angle1.b[1] = angle1.b[1]&0b00111111;
            // angle2 = enc_read_reg((WORD)REG_ANG_ADDR);
            // angle2.b[1] = angle2.b[1]&0b00111111;
            // angle3 = enc_read_reg((WORD)REG_ANG_ADDR);
            // angle3.b[1] = angle3.b[1]&0b00111111;
            // angle = (uint16_t)angle0.b[0] + (uint16_t)angle0.b[1]<<8;
            // // angle = ( ((uint16_t)angle0.b[0] + (uint16_t)angle0.b[1]<<8) + ((uint16_t)angle1.b[0] + (uint16_t)angle1.b[1]<<8) )>>1;
            // // angleAv2 = ((uint16_t)angle2.b + (uint16_t)angle3.b)>>1;
            // // angle = (angleAv1 + angleAv2)>>1;
            // BD[EP0IN].address[0] = (uint8_t) (angle >> 8); // from http://stackoverflow.com/questions/1289251/converting-a-uint16-value-into-a-uint8-array2 
            // BD[EP0IN].address[1] = (uint8_t) (angle);
            BD[EP0IN].address[0] = angle.b[0]; 
            BD[EP0IN].address[1] = angle.b[1];
            BD[EP0IN].bytecount = 2;    // set EP0 IN byte count to 2
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case GET_ANGLE_SPEED:
            BD[EP0IN].address[0] = old_angle.b[0]; 
            BD[EP0IN].address[1] = old_angle.b[1];
            BD[EP0IN].address[2] = angle.b[0]; 
            BD[EP0IN].address[3] = angle.b[1];
            BD[EP0IN].bytecount = 4;    // set EP0 IN byte count to 2
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
        case GET_CURRENT:
            vout.w = pin_read(CURR_P);
            BD[EP0IN].address[0] = vout.b[0];
            BD[EP0IN].address[1] = vout.b[1];
            BD[EP0IN].bytecount = 2;    // set EP0 IN byte count to 2
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

int16_t main(void) {
    init_clock();
    init_ui();
    init_pin();
    init_oc();
    init_spi();
    init_timer();
    //WORD first;
    //WORD second;
    //WORD diff;

    //angle.b[0] = 00000000;
    //angle.b[1] = 00000000;
  
    ANG_MOSI = &D[0];
    ANG_MISO = &D[1];
    ANG_SCK = &D[2];
    ANG_NCS = &D[3];
    CURR_P = &A[0];

    pin_analogIn(CURR_P);
    pin_digitalOut(ANG_NCS);
    pin_set(ANG_NCS);
    
    spi_open(&spi1, ANG_MISO, ANG_MOSI, ANG_SCK, 2e6, 1);

    oc_pwm(&oc1, &D[7], NULL, 20e3, 0x8000);
    oc_pwm(&oc2, &D[8], NULL, 20e3, 0x0);

    led_on(&led1);
    led_on(&led2);
    timer_setPeriod(&timer2, 1);
    timer_start(&timer2);

    old_angle.b[0] = 0b11111111;
    old_angle.b[1] = 0b11111111;
    angle.b[0] = 0b01010101;
    angle.b[1] = 0b10101010;

    InitUSB();                              // initialize the USB registers and serial interface engine
    while (USB_USWSTAT!=CONFIG_STATE) {     // while the peripheral is not configured...
        ServiceUSB();                       // ...service USB requests
    }
    while (1) {
        ServiceUSB();                       // service any pending USB requests
        if (timer_flag(&timer2)) { //periodically:
            timer_lower(&timer2);
            led_toggle(&led1); //switch LED1 from on to off and vice versa
            old_angle = angle;
            enc_read_reg((WORD)REG_ANG_ADDR);
            //enc_read_reg((WORD)REG_ANG_ADDR);
            //enc_read_reg((WORD)REG_ANG_ADDR);

        }
    }
}

