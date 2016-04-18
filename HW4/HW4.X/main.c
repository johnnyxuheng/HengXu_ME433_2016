#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<math.h>
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
#pragma config WDTPS = PS1 // slowest wdt
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
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // USB BUSON controlled by USB module

#define CS LATBbits.LATB8

unsigned int spi_io(unsigned char o) {
  
  SPI1BUF = o;
  while(!SPI1STATbits.SPIRBF) { // wait to receive the byte
    ;
  }
  return SPI1BUF;
}

void DAC_init() {
  // set up the chip select pin as an output
  // the chip select pin is used by the sram to indicate
  // when a command is beginning (clear CS to low) and when it
  // is ending (set CS high)
  TRISBbits.TRISB14 = 0;  //use B14 as clock output  
  TRISBbits.TRISB8 = 0;  //use B8 as SS1
  TRISAbits.TRISA0 = 0;  //udr A0 as SDO1
  CS = 1;
  RPA0Rbits.RPA0R = 0b0011;
  // Master - SPI4, pins are: SDI4(F4), SDO4(F5), SCK4(F13).  
  // we manually control SS4 as a digital output (F12)
  // since the pic is just starting, we know that spi is off. We rely on defaults here
 
  // setup spi4
  SPI1CON = 0;              // turn off the spi module and reset it
  SPI1BUF;                  // clear the rx buffer by reading from it
  SPI1BRG = 0x3;            // baud rate to 6 MHz [SPI4BRG = (48000000/(2*desired))-1]
  SPI1STATbits.SPIROV = 0;  // clear the overflow bit
  SPI1CONbits.CKE = 1;      // data changes when clock goes from hi to lo (since CKP is 0)
  SPI1CONbits.MSTEN = 1;    // master operation
  SPI1CONbits.ON = 1;       // turn on spi 1
                            // send a ram set status command.
  //CS = 0;                   // enable the ram
  //spi_io(0x01);             // ram write status
  //spi_io(0x41);             // sequential mode (mode = 0b01), hold disabled (hold = 0)
  //CS = 1;                   // finish the command
}

// write len bytes to the ram, starting at the address addr
void DAC_write(const int data[], int len) {
  int i = 0;
  CS = 0;                        // enable the ram by lowering the chip select line
  spi_io(0x2);                   // sequential write operation
  //spi_io((addr & 0xFF00) >> 8 ); // most significant byte of address
  //spi_io(addr & 0x00FF);         // the least significant address byte
  for(i = 0; i < len; ++i) {
    spi_io(data[i]);
  }
  CS = 1;                        // raise the chip select line, ending communication
}

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
    
    __builtin_enable_interrupts();
    //TRISAbits.TRISA4 = 0;     // set B4-B7 as digital outputs, 0-3 as digital inputs
    //TRISBbits.TRISB4 = 1;
    
    
    double A_sin=0;
    double A_t=0;
    int A_bit=0;
    double B_tri=0;
    double B_t=0;
    int B_bit=0;
    
    
    
    A_sin=3.3*sin(20*M_PI*A_t);
    B_tri=3.3/0.2*B_t;
    
    DAC_write((int)A_sin,strlen((int)A_sin)+1);
    while(1) {
        _CP0_SET_COUNT(0);
        //LATAbits.LATA4=0;
        while(_CP0_GET_COUNT()<12000){
        }
        _CP0_SET_COUNT(0);
        //LATAbits.LATA4=1;
        while(_CP0_GET_COUNT()<12000){
            ;
        }
        while (PORTBbits.RB4==0){
            ;
        }     
	    // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
		// remember the core timer runs at half the CPU speed
    }
    
    
}