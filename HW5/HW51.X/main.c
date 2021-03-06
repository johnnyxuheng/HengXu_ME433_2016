#include <xc.h>           // processor SFR definitions
#include <sys/attribs.h>  // __ISR macro
#include <math.h>
#include "i2c.h"
#include "imu.h"
#include "ILI9163C.h"

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

void LCD_sprintf(unsigned short x, unsigned short y, unsigned short color, char c);

char message[100];
short data_IMU[14];
short temperature, gyroX, gyroY, gyroZ, accelX, accelY, accelZ;
int i, l, m;

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
    
    SPI1_init();
    LCD_init();
    initI2C2();
    init_ctrl1();
    init_ctrl2();
    init_ctrl3();
    //init_OC();
    
    __builtin_enable_interrupts();
    
    
    
    
    
    
    while(1) {
	    // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
		// remember the core timer runs at half the CPU speed
        _CP0_SET_COUNT(0);                   // set core timer to 0
        
        I2C_read_multipleLine(0x6B, 0x20, data_IMU, 14);
        
        temperature = data_IMU[1] << 8 | data_IMU[0];
        gyroX = data_IMU[3] << 8 | data_IMU[2];
        gyroY = data_IMU[5] << 8 | data_IMU[4];
        gyroZ = data_IMU[7] << 8 | data_IMU[6];
        accelX = data_IMU[9] << 8 | data_IMU[8];
        accelY = data_IMU[11] << 8 | data_IMU[10];
        accelZ = data_IMU[13] << 8 | data_IMU[12];
        
        LCD_clearScreen(WHITE);
        sprintf(message, "Hello World 1337!");
        int n = 0;
        while(message[n]){
          LCD_sprintf(28 + n * 5,32,BLACK,message[n]); 
          n++;
        }
        sprintf(message,"temp: %2.4f deg. c  ",25+(temperature/16.0));
        n=0;
        while(message[n]){
            LCD_sprintf(18 + n * 5,42,BLACK,message[n]); 
            n++;
        }
        sprintf(message,"gyroX: %2.4f dps ",245*gyroX/32768.0);
        n=0;
        while(message[n]){
            LCD_sprintf(18 + n * 5,52,BLACK,message[n]); 
            n++;
        }
        sprintf(message,"gyroY: %2.4f dps ",245*gyroY/32768.0);
        n=0;
        while(message[n]){
            LCD_sprintf(18 + n * 5,62,BLACK,message[n]); 
            n++;
        }
        sprintf(message,"gyroY: %2.4f dps ",245*gyroZ/32768.0);
        n=0;
        while(message[n]){
            LCD_sprintf(18 + n * 5,72,BLACK,message[n]); 
            n++;
        }
        sprintf(message,"accelX: %2.4f g  ",2*accelX/32768.0);
        n=0;
        while(message[n]){
            LCD_sprintf(18 + n * 5,82,BLACK,message[n]); 
            n++;
        }
        sprintf(message,"accelY: %2.4f g  ",2*accelY/32768.0);
        n=0;
        while(message[n]){
            LCD_sprintf(18 + n * 5,92,BLACK,message[n]); 
            n++;
        }
        sprintf(message,"accelZ: %2.4f g  ",2*accelZ/32768.0);
        n=0;
        while(message[n]){
            LCD_sprintf(18 + n * 5,102,BLACK,message[n]); 
            n++;
        }


        //OC1RS=(int)(6000.0+accelX/32767.0*12000.0);
        //OC2RS=(int)(6000.0+accelY/32767.0*12000.0);

        for (i=0;i<200;i++){
            while(_CP0_GET_COUNT() < 48000){     // wait 1ms / 0.001s
                ;
            }
            _CP0_SET_COUNT(0);
        }      
    }    
}
void LCD_sprintf(unsigned short x, unsigned short y, unsigned short color, char c){
        char pixel;
        char LCD_ascii;
        LCD_ascii = c - 0x20;
        for(l = 0; l < 5; l++){
            for(m = 0;m < 8; m++){
                pixel = ASCII[LCD_ascii][l];  // temp = 0x7f = 01111111, 00001000, 00001000, 00001000, 01111111
                if((pixel >> (7 - m)) & 1){
                    LCD_drawPixel(x + l, y + (7-m), color);
                }

            }
        }
    }
