#include <xc.h>           // processor SFR definitions
#include <sys/attribs.h>  // __ISR macro
#include <math.h>
#include "i2c.h"

// DEVCFG0
#pragma config DEBUG = OFF // no debugging
#pragma config JTAGEN = OFF // no jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect
#pragma config BWP = OFF // no boot write protect
#pragma config CP = OFF // no code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // turn off secondary oscillator
#pragma config IESO = OFF // no switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // free up secondary osc pins
#pragma config FPBDIV = DIV_1 // divide CPU freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // do not enable clock switch
#pragma config WDTPS = PS1048576 // slowest wdt
#pragma config WINDIS = OFF // no wdt window
#pragma config FWDTEN = OFF // wdt off by default
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the CPU clock to 48MHz
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz
#pragma config UPLLIDIV = DIV_2 // divider for the 8MHz input clock, then multiply by 12 to get 48MHz for USB
#pragma config UPLLEN = ON // USB clock on

// DEVCFG3
#pragma config USERID = 0xFFFF // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // USB BUSON controlled by USB module


unsigned char data_IMU[14];
unsigned short temperature, gyroX, gyroY, gyroZ, accelX, accelY, accelZ;
int i;

int main() {
    
    
    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    // do your TRIS and LAT commands here
    TRISAbits.TRISA4 = 0;     // ouput
    TRISBbits.TRISB4 = 1;     // input

    initI2C2();
    init_ctrl1();
    init_ctrl2();
    init_ctrl3();
    init_OC();
    
    __builtin_enable_interrupts();
    
    while(1) {
	    // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
		// remember the core timer runs at half the CPU speed
        _CP0_SET_COUNT(0);                   // set core timer to 0
        
        I2C_read_multipleLine(0x6B, 0x20, data_IMU, 14);
        
        temperature = data_IMU[0] << 8 | data_IMU[1];
        gyroX = data_IMU[2] << 8 | data_IMU[3];
        gyroY = data_IMU[4] << 8 | data_IMU[5];
        gyroZ = data_IMU[6] << 8 | data_IMU[7];
        accelX = data_IMU[8] << 8 | data_IMU[9];
        accelY = data_IMU[10] << 8 | data_IMU[11];
        accelZ = data_IMU[12] << 8 | data_IMU[13];
        
        if (accelX < 49152){
//            OC1RS=0;
//            OC2RS=0;
            OC1RS=0;
            OC2RS=0;
            LATAbits.LATA4 = 1;
        }
        if (accelX >= 49152){
            OC1RS=12000;
            OC2RS=12000;
            
            LATAbits.LATA4 = 0;
        }
        
        
        for (i=0;i<20;i++){
            while(_CP0_GET_COUNT() < 48000){     // wait 1ms / 0.001s
                ;
            }
            _CP0_SET_COUNT(0);
        }
        //OC1RS < 0 || OC2RS < 0
        //OC1RS = accelX/32768*12000;
        //OC2RS = accelY/32768*12000;
        

        

    }  
    
}
