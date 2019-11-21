/*
 * File:   main.c
 * Author: RichardvdK
 *
 * Created on 5th of May 2016, 17:07
 */


#include <xc.h>
#include <stdio.h>
#define	_XTAL_FREQ 8000000
#include "delays.h"
#include "xlcd.h"



// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown Out Reset Selection bits (BOR enabled)
#pragma config IESO = ON        // Internal External Switchover bit (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)


void tx(char *array, unsigned int size);

void main(void) {

    OSCCONbits.IRCF = 0b111;
    ANSEL = 0;
    ANSELH = 0;

    TRISC = 0b10000000;
    PORTC = 0b00000000;
    TRISB = 0b00000000;
    TRISA = 0b00000000;

    BAUDCTLbits.SCKP = 1;

    SYNC = 0;
    BRGH = 0;
    BRG16 = 0;
    SPBRG = 11;

    TXSTAbits.TXEN = 1;
    TXSTAbits.SYNC = 0;
    TXSTAbits.BRGH = 0;

    PIE1bits.RCIE = 1;
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;

    WDTCONbits.WDTPS = 0b0101;
    OPTION_REGbits.PSA = 1;


    __delay_ms(250);

    
    OpenXLCD(FOUR_BIT & LINES_5X7);
    while (BusyXLCD());
    WriteCmdXLCD(SHIFT_DISP_LEFT);
    while (BusyXLCD());
    putrsXLCD("start init");
    while (BusyXLCD());

    __delay_ms(2000);


    PORTCbits.RC6 = 1;
    __delay_ms(200);
    PORTCbits.RC6 = 0;
    __delay_ms(400);
    PORTCbits.RC6 = 1;
    __delay_ms(400);
    PORTCbits.RC6 = 0;
    __delay_ms(400);
    PORTCbits.RC6 = 1;
    __delay_ms(400);
    PORTCbits.RC6 = 0;



    RCSTAbits.SPEN = 1; //enable eusart
    RCSTAbits.CREN = 1; //receive enable


    WDTCONbits.SWDTEN = 1;

    int iso;
    while (RCIF == 0);
    char syncrr = RCREG;
    if (syncrr == 0x55) {
        while (RCIF == 0);
        char key1 = RCREG;
        while (RCIF == 0);
        char key2 = RCREG;
        __delay_ms(25);
        TXREG = ~key2;
        while (RCIF == 0);
        char clear = RCREG;
        while (RCIF == 0);
        char ready = RCREG;
        if (ready == 0xcc) {
            WriteCmdXLCD(0x01);
            while(BusyXLCD());
            __delay_ms(25);
            if (key1 == 0x08 && key2 == 0x08) {
                iso = 9141;
                putrsXLCD("ISO 9141-2");
                while (BusyXLCD());
                WriteCmdXLCD(0xC0);
                while(BusyXLCD());
                putrsXLCD("5-baud init OK");
                while (BusyXLCD());
            }
            if (key2 == 0x8f) {
                iso = 14230;
                putrsXLCD("ISO 14230-4 KWP");
                while (BusyXLCD());
                WriteCmdXLCD(0xC0);
                while(BusyXLCD());
                putrsXLCD("5-baud init OK");
                while (BusyXLCD());
            }
        }
    } else {
        WriteCmdXLCD(0x01);
        while(BusyXLCD());
        putrsXLCD("init failed");
        while(BusyXLCD());
    }

    WDTCONbits.SWDTEN = 0;




    __delay_ms(4000);

    
    WriteCmdXLCD(0x01);
    while(BusyXLCD());
    putrsXLCD("init failed");
    while(BusyXLCD());
    
    unsigned int counter = 0;


    PR2 = 0xff;

    CCP2CON = 0b00001100;

    CCPR2L = 0x00;
    CCPR2H = 0x00;

    TMR2IF = 0x00;

    PIR1 = 0x00;


    T2CONbits.T2CKPS = 0b11;
    T2CONbits.TMR2ON = 1;

//    CCP2CONbits.DC2B0 = counter;
//    CCP2CONbits.DC2B1 = (counter >> 1);
//    CCPR2L = (counter >> 2);
    CCPR2L = counter;
    
    WriteCmdXLCD(0x01);
    while(BusyXLCD());
    
    OPTION_REGbits.nRBPU=0;
    WPUBbits.WPUB0=1;
    TRISBbits.TRISB0 = 1;
    
    

    while(1) {

        WDTCONbits.SWDTEN = 1;

        char byteline1[16];
        char byteline2[16];
        char rx_buffer[8];
        __delay_ms(5);
        
        
        WriteCmdXLCD(0x80);
        while(BusyXLCD());
        putrsXLCD("CEL  T Speed RPM");
        while(BusyXLCD());
        
        WriteCmdXLCD(0xC0);
        while(BusyXLCD());

        

        if (iso == 9141) {
            const char CalculatedEngineLoad[] = {0x68, 0x6a, 0xf1, 0x01, 0x04, 0xc8};
            tx(CalculatedEngineLoad, sizeof (CalculatedEngineLoad));
        }
        if (iso == 14230) {
            const char CalculatedEngineLoad[] = {0xc2, 0x33, 0xf1, 0x01, 0x04, 0xeb};
            tx(CalculatedEngineLoad, sizeof (CalculatedEngineLoad));
        }

        for (int i = 0; i < 7; i++) {
            while (RCIF == 0);
            char byte_in = RCREG;
            rx_buffer[i] = byte_in;
        }

        char cel = rx_buffer[5] * 100 / 255;


        if (rx_buffer[5] >= 35) {
            while (rx_buffer[5] - 35 > counter) {
                CCPR2L = counter;
                __delay_ms(2);
                counter++;
            }
            while (rx_buffer[5] - 35 < counter) {
                CCPR2L = counter;
                __delay_ms(2);
                counter--;
            }
        }







        __delay_ms(50);


        if (iso == 9141) {
            const char EngineCoolantTemperature[] = {0x68, 0x6a, 0xf1, 0x01, 0x05, 0xc9};
            tx(EngineCoolantTemperature, sizeof (EngineCoolantTemperature));
        }
        if (iso == 14230) {
            const char EngineCoolantTemperature[] = {0xc2, 0x33, 0xf1, 0x01, 0x05, 0xec};
            tx(EngineCoolantTemperature, sizeof (EngineCoolantTemperature));
        }

        for (int i = 0; i < 7; i++) {
            while (RCIF == 0);
            char byte_in = RCREG;
            rx_buffer[i] = byte_in;
        }

        char ect = rx_buffer[5] - 0x28;
        
        if (ect >= 93) {
            PORTBbits.RB7=1;
        }
        else {
            PORTBbits.RB7=0;
        }




        __delay_ms(50);


        if (iso == 9141) {
            const char VehicleSpeed[] = {0x68, 0x6a, 0xf1, 0x01, 0x0d, 0xd1};
            tx(VehicleSpeed, sizeof (VehicleSpeed));
        }
        if (iso == 14230) {
            const char VehicleSpeed[] = {0xc2, 0x33, 0xf1, 0x01, 0x0d, 0xf4};
            tx(VehicleSpeed, sizeof (VehicleSpeed));
        }

        for (int i = 0; i < 7; i++) {
            while (RCIF == 0);
            char byte_in = RCREG;
            rx_buffer[i] = byte_in;
        }

        char vs = rx_buffer[5];


        
        
        
        
        
        __delay_ms(50);

        
        if (iso == 9141) {
            const char CalculatedEngineLoad[] = {0x68, 0x6a, 0xf1, 0x01, 0x04, 0xc8};
            tx(CalculatedEngineLoad, sizeof (CalculatedEngineLoad));
        }
        if (iso == 14230) {
            const char CalculatedEngineLoad[] = {0xc2, 0x33, 0xf1, 0x01, 0x04, 0xeb};
            tx(CalculatedEngineLoad, sizeof (CalculatedEngineLoad));
        }

        for (int i = 0; i < 7; i++) {
            while (RCIF == 0);
            char byte_in = RCREG;
            rx_buffer[i] = byte_in;
        }

        cel = rx_buffer[5] * 100 / 255;


        if (rx_buffer[5] >= 35) {
            while (rx_buffer[5] - 35 > counter) {
                CCPR2L = counter;
                __delay_ms(2);
                counter++;
            }
            while (rx_buffer[5] - 35 < counter) {
                CCPR2L = counter;
                __delay_ms(2);
                counter--;
            }
        }

        
        
        
        
        

        __delay_ms(50);

        
        if (iso == 9141) {
            const char EngineRPM[] = {0x68, 0x6a, 0xf1, 0x01, 0x0c, 0xd0};
            tx(EngineRPM, sizeof (EngineRPM));
        }
        if (iso == 14230) {
            const char EngineRPM[] = {0xc2, 0x33, 0xf1, 0x01, 0x0c, 0xf3};
            tx(EngineRPM, sizeof (EngineRPM));
        }

        for (int i = 0; i < 8; i++) {
            while (RCIF == 0);
            char byte_in = RCREG;
            rx_buffer[i] = byte_in;
        }

        unsigned int rpm = ((rx_buffer[5] * 256) + rx_buffer[6]) / 4;

        sprintf(byteline2, "%3d %3d %3d %4d", cel, ect, vs, rpm);
        putrsXLCD(byteline2);
        while(BusyXLCD());



        __delay_ms(50);

        WDTCONbits.SWDTEN = 0;
        
        
        
        
        
        
        
        if (PORTBbits.RB0 == 0) {
            __delay_ms(1500);
            if (PORTBbits.RB0 == 0) {
                WriteCmdXLCD(0x01);
                while(BusyXLCD());
                putrsXLCD("Check for stored");
                while(BusyXLCD());
                WriteCmdXLCD(0xC0);
                while(BusyXLCD());
                putrsXLCD("trouble code 1");
                while (BusyXLCD());
                
                __delay_ms(2000);
                
                WDTCONbits.SWDTEN = 1;
                
                if (iso == 9141) {
                    const char trouble[] = {0x68, 0x6a, 0xf1, 0x03, 0xc6};
                    tx(trouble, sizeof (trouble));
                }
                if (iso == 14230) {
                    const char trouble[] = {0xc2, 0x33, 0xf1, 0x03, 0xe9};
                    tx(trouble, sizeof (trouble));
                }
                
                WDTCONbits.SWDTEN = 0;
                
                for (int i = 0; i < 8; i++) {
                    while (RCIF == 0);
                    char byte_in = RCREG;
                    rx_buffer[i] = byte_in;
                }
                
                char dtc1 = (rx_buffer[5] && 0x0c) >> 6;
                char dtc2 = (rx_buffer[5] && 0x30) >> 4;
                char dtc3 = rx_buffer[5] && 0x0f;
                char dtc4 = (rx_buffer[6] && 0xf0) >> 4;
                char dtc5 = rx_buffer[6] && 0x0f;
                
                char code = "";
                switch (dtc1) {
                    case 0b00 : code = "P";
                    break;
                    case 0b01 : code = "C";
                    break;
                    case 0b10 : code = "B";
                    break;
                    case 0b11 : code = "U";
                    break;
                }
                
                sprintf(byteline2, "%c%1d%1x%1x%1x", code, dtc2, dtc3, dtc4, dtc5);
                
                WriteCmdXLCD(0x01);
                while(BusyXLCD());
                putrsXLCD("Trouble code 1 =");
                while(BusyXLCD());
                WriteCmdXLCD(0xC0);
                while(BusyXLCD());
                putrsXLCD(byteline2);
                while (BusyXLCD());
                
                __delay_ms(10000);
                
                WriteCmdXLCD(0x01);
                while(BusyXLCD());
                putrsXLCD("Press button to");
                while(BusyXLCD());
                WriteCmdXLCD(0xC0);
                while(BusyXLCD());
                putrsXLCD("clear ECU");
                while (BusyXLCD());
                
                if (PORTBbits.RB0 == 0) {
                    __delay_ms(1000);
                    if (PORTBbits.RB0 == 0) {
                        WriteCmdXLCD(0x01);
                        while(BusyXLCD());
                        putrsXLCD("Clearing ECU");
                        while(BusyXLCD());
                
                        __delay_ms(4000);
                        
                        char byte_in = RCREG;
                        
                        WDTCONbits.SWDTEN = 1;
                
                        if (iso == 9141) {
                            const char trouble[] = {0x68, 0x6a, 0xf1, 0x04, 0xc7};
                            tx(trouble, sizeof (trouble));
                        }
                        if (iso == 14230) {
                            const char trouble[] = {0xc2, 0x33, 0xf1, 0x04, 0xea};
                            tx(trouble, sizeof (trouble));
                        }
                        WDTCONbits.SWDTEN = 0;
                    }
                }
            }
        }
    }
}



void tx(char * array, unsigned int size) {

    PIE1bits.TXIE = 1;
    for (int i = 0; i < size; i++) {
        while (TXIF == 0);
        TXREG = array[i];
        while (RCIF == 0);
        char clear = RCREG;
    }
    PIE1bits.TXIE = 0;
}




/*------------------------------------------------------------------------*/
void DelayFor18TCY(void) {
    __delay_us(18);

}

/*------------------------------------------------------------------------*/
void DelayPORXLCD(void) {
    __delay_ms(15);

}

/*------------------------------------------------------------------------*/
void DelayXLCD(void) {
    __delay_ms(5);

}
/*------------------------------------------------------------------------*/
