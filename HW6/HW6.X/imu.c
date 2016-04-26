#include "imu.h"
#include<xc.h> 
#include <math.h>

void init_OC(void){
    TRISAbits.TRISA0 = 0;     // set RA0 output for OC1
    TRISAbits.TRISA1 = 0;     // set RA1 output for OC2
    RPA0Rbits.RPA0R = 0b0101; // assign RA0 for OC1
    RPA1Rbits.RPA1R = 0b0101; // assign RA1 for OC2
    
    T2CONbits.TCKPS = 0b010;     // prescale = 4 / N = 4
    PR2 = 11999;                  // period = (PR2 + 1) * N * ? ns = ?  / 1kHz
    // duty cycle = OC1RS/(PR2+1) = 50%
    OC1RS = 6000;
    OC2RS = 6000;
    TMR2 = 0;
    OC1CONbits.OCM = 0b110;   // PWM
    OC2CONbits.OCM = 0b110;   // PWM
    OC1CONbits.OCTSEL = 0;  // use timer2 for OC1
    OC2CONbits.OCTSEL = 0;  // use timer2 for OC2
    T2CONbits.ON = 1;       // turn on time2
    OC1CONbits.ON = 1;      // turn on OC1
    OC2CONbits.ON = 1;      // turn on OC2  
}

void init_ctrl1(void){
    i2c_master_start();
    i2c_master_send(0x6B << 1 | 0);   // chip address & indicate write
    i2c_master_send(0x10);   // addr of ctrl_1 register
    i2c_master_send(0x80);   // send the value to the register, enable accelerometer
    i2c_master_stop();
}

void init_ctrl2(void){
    i2c_master_start();
    i2c_master_send(0x6B << 1 | 0);   // chip address & indicate write
    i2c_master_send(0x11);   // addr of ctrl_2 register
    i2c_master_send(0x80);   // send the value to the register, enable agyroscope
    i2c_master_stop();
}

void init_ctrl3(void){
    i2c_master_start();
    i2c_master_send(0x6B << 1 | 0);   // chip address & indicate write
    i2c_master_send(0x12);   // addr of ctrl_3 register
    i2c_master_send(0x04);   // send the value to the register, enable if_inc
    i2c_master_stop();
}

void I2C_read_multipleLine(char address, char register_addr, unsigned char * data, char length){
    //char data2;
    int j;
    i2c_master_start();
    i2c_master_send(address << 1 | 0);   // chip address & indicate write
    i2c_master_send(register_addr);   // addr of OUT_TEMP_L register
    //i2c_master_send(0x0F);
    i2c_master_restart();   // make the restart bit, so we can begin reading
    i2c_master_send(address << 1 | 1);   // chip address & indicate reading
    //data2=i2c_master_recv();
    for(j = 0; j < length - 1; j++){
        *(data + j) = i2c_master_recv();
        i2c_master_ack(0); // make the ack so the slave knows we got it
    }
    *(data + length) = i2c_master_recv();
    i2c_master_ack(1);
    i2c_master_stop(); // make the stop bit
//    if (data2==0b01101001){
//        LATAbits.LATA4 = 0;
//    }
    
}