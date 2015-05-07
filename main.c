/* 
 * File:   main.c
 * Author: gs
 *
 * Created on 11. November 2014, 20:10
 */

/*  Pin Assignments
    2 RA0/AN0/CN2					- Current Motor1
    3 RA1/AN1/CN3					- Current Motor2
    9 RA2/CN30						- OSC
   10 RA3/CN29						- OSC
   12 RA4/CN0
   nc RA5
   nc RA6
   nc RA7

    4 RB0/AN2/CN4/RP0				- DC Ref
    5 RB1/AN3/CN5/RP1				- Button 1
    6 AN4/RP2(1)/CN6/RB2/RP2 		- Green LED
    7 RB3/AN5/CN7/RP3				- current set via pot
   11 RB4/CN1/RP4
   14 ASDA1/RP5(1)/CN27/RB5/RP5    	- Red LED
   15 RB6/CN24/RP6					- Direction / RX
   16 RB7/INT0/CN23/RP7				- Step /TX
   17 RB8/CN22/RP8					- FAULT2
   18 RB9/CN21/RP9					- FAULT1
   21 RB10/CN16/RP10				- IN1
   22 RB11/CN15/RP11				- IN2
   23 RB12/AN9/CN14/RP12			- IN3
   24 RB13/AN8/CN13/RP13			- IN4
   25 RB14/AN7/CN12/RP14			- EN_A
   26 RB15/AN6/CN11/RP15			- EN_B
 */



#define FCY 40000000UL

#include <libpic30.h>


#include <p33FJ12MC202.h>

#include <stdint.h>

#include "main.h"

#include "encoder.h"
#include "uart.h"
#include "fix6_10.h"
#include "adc.h"
#include "clarkepark.h"
#include "uart.h"
#include "cordic.h"
#include "control.h"



int16_t winkel=0;

//int pwm1 = 0, pwm2 = 85, pwm3 = 170;
uint16_t angle = 0;
unsigned int oldangle = 0;

_FBS(BSS_NO_FLASH & BWRP_WRPROTECT_OFF);
/* no Boot sector and
   write protection disabled */

//_FOSCSEL (IESO_OFF & FNOSC_FRC);
/* The chip is started using FRC then switch */
//_FOSCSEL (IESO_OFF & FNOSC_FRC);

_FOSCSEL(IESO_OFF & FNOSC_PRIPLL); // my choice cristal W PLL



_FWDT(FWDTEN_OFF);
/* Turn off Watchdog Timer */

_FGS(GSS_OFF & GCP_OFF & GWRP_OFF);
/* Set Code Protection Off for the General Segment */

//_FOSC (FCKSM_CSECMD & IOL1WAY_ON & OSCIOFNC_OFF & POSCMD_XT);
_FOSC(FCKSM_CSECMD & OSCIOFNC_OFF & POSCMD_XT & IOL1WAY_OFF);

/* clock switch & monitoring disabled
   remapable I/O enabled
  OSC2 pin is clock O/P
  oscilator is XT
 */


_FPOR(PWMPIN_ON & HPOL_ON & LPOL_ON & FPWRT_PWR128);
//_FPOR(PWMPIN_ON  & HPOL_OFF & LPOL_OFF & FPWRT_PWR128); // invert PWM output

/* PWM mode is Port registers
   PWM high & low active high
   FPOR power on reset 128ms
 */
_FICD(JTAGEN_OFF & ICS_PGD3);




#define grnLEDON		PORTBbits.RB2 = 1; __builtin_nop()
#define grnLEDON1		PORTB =(1 <<2) | LATB ; __builtin_nop()

#define grnLEDOFF		PORTBbits.RB2 = 0; __builtin_nop()

#define redLEDON		PORTBbits.RB5 = 1; __builtin_nop()
#define redLEDON1		PORTB =(1 <<5)| LATB; __builtin_nop()

#define redLEDOFF		PORTBbits.RB5 = 0; __builtin_nop()





void switchClockTo80MHzPll(void) {
    /*
            Perform a clock switch to 40MIPS (80Mhz)

            The source clock is an external crystal (8Mhz in our case) or the internal oscillator.

            N1 = This factor (2 through 33) scales the source clock into the range
                     of 0.8 to 8MHz.  It's value is PLLPRE+2

            M  = PLL Feedback divisor (2 through 513).  The factor by which the source clock is
             multipled.  The resulting value must be between 100-200MHz.  It's value is
             PLLFBD+2

            N2 = This factor (2,4 or 8) scales the 100-200MHz clock into the 12.5-80MHz range.
                     It's value is 2*(PLLPOST+1)

            Our target is 80MHz

            80MHz = ((8Mhz/N1)*M)/N2
                      = ((8Mhz/2)*40)/2

              N1 = 2 so PLLPRE = 0
                      M = 40 so PLLFBD = 38
              N2 = 2 so PLLPOST = 0
     */

    PLLFBD = 38;
    CLKDIVbits.PLLPOST = 0;
    CLKDIVbits.PLLPRE = 0;

    //unlock OSCCON register
    __builtin_write_OSCCONH(0x03);
    __builtin_write_OSCCONL(0x01);



    //wait for clock to stabilize
    while (OSCCONbits.COSC != 0b011);
    //wait for PLL to lock
    while (OSCCONbits.LOCK != 1);
    //clock switch finished
}



/*
 * 
 */

int main(void) {
//struct sclarkepark CP;
//    int commandValue;
    short writeb;
    char ReceivedChar = 0;
//    int RecNo = 0;
    char state = 0;
    char adcstate = 0;
    switchClockTo80MHzPll();
    uint16_t IqRef1=200;
    char helpbuffer[10];
    char copybuffer[10];
    int helpbufferc;
    int value = 0;
    uint16_t velocity = 2000;
    uint16_t setangle;
    uint16_t uicos,uisin;
    int16_t z;
    int16_t pos;
    //fix6_10_t IqRef=0;
    

    AD1PCFGL = 0b0000001101111111; // All I/O pins are digital to start
    AD1PCFGL = 0b0000001101111111; // All I/O pins are digital to start



    //PORTB = writeb;

    TRISA = 0; //default all A to output
    TRISB = 0; //default all B to output
    PORTA = 0;
    PORTB = 0;
    //TRISBbits.TRISB6 = 1; // input STEP
    TRISBbits.TRISB1 = 1; // input STEP
    TRISBbits.TRISB7 = 1; // input STEP
    TRISBbits.TRISB11 = 1;
    TRISBbits.TRISB13 = 1;



    //step trips INT0 and advances the target current
    CNPU2bits.CN23PUE = 1;

    ADC1BUF0 = ADC1BUF1 = ADC1BUF2 = ADC1BUF3 = 0;

    __builtin_write_OSCCONL(OSCCON & ~(1 << 6));

    RPOR3bits.RP6R = 3; // TX
    RPINR18bits.U1RXR = 1; //RX
    //RPINR18bits.U1 = 6;  //RX

    // RP13   Blue  Chan B  phys pin 24
    // RP11   Yellow      CHan A  phys pin 22
    // RP7 GReen INDX            phys pin 16
    // yellow channel A, Blue B, GREEN InDX
    // RPINR 14 QEAR ,QEBR
    // RPINR 15 INDX


    RPINR14bits.QEA1R = 11;

    RPINR14bits.QEB1R = 13;
    RPINR15bits.INDX1R = 7;

    //RPOR3bits.RP6R = 3; // TX
    //RPINR18bits.U1RXR = 7;  //RX
    __builtin_write_OSCCONL(OSCCON | (1 << 6));

    //TRISBbits.TRISB7 = 0; // TX


    InitUART();


    // Interrupt 0 is currently used for Index pulse

    // INTCON2bits.INT0EP = 0;			//trigger on rising edge
    //IPC0bits.INT0IP = 1;			//(7 is highest)
    //IFS0bits.INT0IF = 0;			//clear the INT0 flag
    //IEC0bits.INT0IE = 1;
    IEC0bits.U1TXIE = 1;
    IEC0bits.U1RXIE = 1;

    state = 0;
    //initControl();
    initEncoder();
    PWMinit();
    initADC();
    init_spic();

    helpbuffer[0] = '0';
    helpbuffer[1] = '0';
    helpbuffer[2] = '0';
    helpbuffer[3] = '0';
    helpbuffer[4] = 13;
    helpbuffer[5] = 10;
    copybuffer[0] = '0';
    copybuffer[1] = '0';
    copybuffer[2] = '0';
    copybuffer[3] = '0';
    copybuffer[4] = 13;
    copybuffer[5] = 10;


    P1DC1= 0; //1000;
    P1DC2= 0; //1000;

    P1DC3= 0; //1000;


    while (1) {
        copybuffer[0] = ' ';
        copybuffer[1] = ' ';
        copybuffer[2] = ' ';
        copybuffer[3] = ' ';
        copybuffer[4] = ' ';
        copybuffer[5] = 13;
        copybuffer[6] = 10;

        //__builtin_write_OSCCONH(0b);
        //__builtin_write_OSCCONL(0x01);

        //if(U1STAbits.OERR == 1)
        //  U1STAbits.OERR = 0;

        //if(U1STAbits.FERR == 1)
        //  U1STAbits.FERR == 0;
        //if(U1STAbits.PERR == 1)
        //  U1STAbits.PERR = 0;


        //ReceivedChar = U1STAbits.URXDA;
        //    if(U1STAbits.URXDA==1  )

        //ReceivedChar = U1RXREG;
        //#ifdef NIX
        if (RS232received() == 1) {
            ReceivedChar = RS232getch();
            RS232print(&ReceivedChar, 1);
            writeb = PORTB;
            writeb |= (1 << 2);
            PORTB = writeb;
            //grnLEDON1;

            if (ReceivedChar == 'a' || ReceivedChar == 'b' || ReceivedChar == 'c' || ReceivedChar == 'e'
                    || ReceivedChar == 'r' || ReceivedChar == 'd' || ReceivedChar == 'v'||
                    ReceivedChar == '+'||ReceivedChar == '-') {
                state = ReceivedChar;
                helpbuffer[0] = 0;
                helpbufferc = 0;
                //RecNo=0;

                //grnLEDON;
            }
            if ((ReceivedChar <= '9') && (ReceivedChar >= '0') &&
                    (state == 'a' || state == 'b' || state == 'c' || state == 'r' || state == 'v' || state == 'e')) {
                //RecNo += (ReceivedChar -'0') * state;
                if (helpbufferc < 9)
                    helpbuffer[helpbufferc++] = ReceivedChar;
            }
            if (ReceivedChar == 13) {
                if (helpbufferc < 9)
                    helpbuffer[helpbufferc] = '\0';


                if (state == 'a') {
                    value = myAtoi(helpbuffer);
                    itoa_1(value, &helpbuffer[0]);
                    RS232print(&helpbuffer[0], helpbufferc);
                    P1DC1 = value;
                    helpbufferc = 0;
                    state = 0;

                }
                if (state == 'b') {
                    value = myAtoi(helpbuffer);
                    itoa_1(value, &helpbuffer[0]);
                    RS232print(&helpbuffer[0], helpbufferc);
                    P1DC2 = value;
                    helpbufferc = 0;
                    state = 0;

                }
                if (state == 'c') {
                    value = myAtoi(helpbuffer);
                    itoa_1(value, &helpbuffer[0]);
                    RS232print(&helpbuffer[0], helpbufferc);
                    P1DC3 = value;
                    helpbufferc = 0;
                    state = 0;
                }
                if (state == 'v') {
                    value = myAtoi(helpbuffer);
                    itoa_1(value, &helpbuffer[0]);
                    RS232print(&helpbuffer[0], helpbufferc);
                    velocity = 0;
                    SPIC.target=value;
                    helpbufferc = 0;
                    state = 0;
                }
                 if (state == 'e') {
                    value = myAtoi(helpbuffer);
                    itoa_1(value, &helpbuffer[0]);
                    RS232print(&helpbuffer[0], helpbufferc);
                    IqRef1 = value;
                    helpbufferc = 0;
                    state = 0;
                }



                if (state == 'r' && (helpbuffer[0] >= '0') && ( helpbuffer[0] <= '8') ) // adc
                {
                    adcstate = helpbuffer[0];
                    if (adcstate == '0')
                        value = adc0;
                    if (adcstate == '1')
                        value = adc1;
                    if (adcstate == '2')
                        value = adc2;
                    if (adcstate == '3')
                        value = adc3;
                    if (adcstate == '4')
                        value = POSCNT;


                    helpbuffer[0] = ' ';
                    helpbuffer[1] = ' ';
                    helpbuffer[2] = ' ';
                    helpbuffer[3] = ' ';
                    helpbuffer[4] = 13;
                    helpbuffer[5] = 10;




                    itoa_1(value, &helpbuffer[0]);
                    RS232print(&helpbuffer[0], 6);
                    helpbufferc = 1;
                    //state = 0;
                    //state = 'r';

                    __delay32((uint32_t) (800000L));

                }

                //grnLEDOFF;
                RS232print("OK", 2);
                RS232print((char*) 13, 1);
                RS232print((char*) 10, 1);

                RS232print(">", 2);


                //writeb &= ~(1 << 2);
                //PORTB = writeb;
            }
        }

        if (adcstate == '0')
            value = adc0;
        if (adcstate == '1')
            value = adc1;
        if (adcstate == '2')
            value = adc2;
        if (adcstate == '3')
            value = adc3;
        if (adcstate == '4')
               value = POSCNT;

        if (adcstate == '5')
               value = SCP.Iq;
        if (adcstate == '6')
               value = SCP.Id;

        //if( adcstate == '7')
        //    value = cordic( SCP.Ialpha,SCP.Ibeta)>>16;

        //   itoa_1(value                ,&copybuffer[0]);
        //   RS232print(&copybuffer[0],7 );
        //if(state== 'v')
        //{

        //}







        if (state == 'd' || state == '+'|| state =='-') {   // d

            if( state=='d'){
                IqRef=0;
                index1=0;
                //winkel =0;
                if (winkel <0)
                    winkel =0; // should be 0
            }
            if( state=='+'){
                IqRef=IqRef1;
                state='+';
                //winkel+=10;
                //winkel =120;
                if (winkel >=360)
                    winkel =0;
            }
            if( state=='-'){

                pos= POSCNT;
                    IqRef=-IqRef1;
//#ifdef NIX
                if( (POSCNT <= 500 ) && (POSCNT >50))
                    IqRef=-IqRef1;
                else
                    IqRef=0;

                if((POSCNT > 1500 ) && (POSCNT <1999))
                   IqRef=IqRef1;
                 else
                    IqRef=0;

//#endif





                //IqRef=-IqRef1;
                //winkel-=10;
                //winkel =240;

                if (winkel <0)
                    winkel =0; // should be 0



            }
            //if( state=='d'){
            //    winkel=0;
            //}

//angle =16;  // REMOVE ######
//#ifdef NIX
            //angle = cnt2omega_el(POSCNT);
            //SCP.angle = winkel *182;
            //angle = cnt2omega_el(POSCNT);
            //print_str("angle:",6);
           // print_int16( angle/182 );

            //IqRef =winkel;

#ifdef NIX
            SCP.angle = 0; // angle;;
            //angle =0xffff;
            //angle -=200;
            //angle=setangle;

            //uicos= mycos( angle);
            //uisin =mysin(angle );
            //CP.IUalpha= (fix6_10_t) mycos( angle);
            //CP.IUbeta=(fix6_10_t) mysin(angle );

#ifdef NIX
            SCP.Ia=( fix6_10_t ) dac2current( adc1 );
            SCP.Ib=( fix6_10_t ) dac2current( adc2 );
#endif
                print_str("Ia:",5);
                print_fix6_10( SCP.Ia );
                //print_str("Ia16:",5);
                //print_int16( SCP.IUalpha );
                //print_str("ADC1",4 );
                //print_int16( adc1 );
                print_str("Ib:",5);

                //print_fix16( SCP.Ib );
                print_fix6_10( SCP.Ib );


            //angle;
            //CP.IUalpha=fix16_mul (CP.IUalpha, 0x7f00 );
            //CP.IUbeta=fix16_mul (CP.IUbeta, 0x7f00 );

            // electrical angle of encoder
            // SCP.angle
            // SCP.Ia,SCP.ib
            ClarkePark();
            // output SCP.Id, SCP.Iq

            IqRef= (int16_t ) 200; //0x700; //fix16_from_float( 0.1);




             print_str("Ialpha:",6);
             print_fix6_10( SCP.Ialpha );
             print_str("Ibeta:",6);
             print_fix6_10( SCP.Ibeta );
             print_CR();


               print_str("Cordic ",7);
                 //print_int16( cordic6_10 ( SCP.IUalpha,SCP.IUbeta) );
                 //print_str("  ",2);
                 print_int16( cordic6_10 ( SCP.Id,SCP.Iq) );


//#ifdef TEST
            SPIC.deltaIq= IqRef -SCP.Iq;
            //PICTRL.deltaIdcurrent= -CP.Id;   // Idref =0;
            tf_spi(); //gives Uq



            //SCP.Vd = PICTRL.sumIdlast;  // the sum is a voltage
            //SCP.Uq = PICTRL.sumIqlast;
//#endif
            // 360 * 182 = 2^16,  C =12 Volt, b6=182

            // SCP.IUalpha=  fix6_10_mul(mycos(  winkel*182 ) /32, fix6_10_from_int(12 ));
            // SCP.IUbeta =  fix6_10_mul(mysin(  winkel*182 ) /32, fix6_10_from_int(12 ));
            //CP.IUbeta = fix6_10_mul( fix16_mul( mysin( fix16_mul( winkel,182 )), );
#ifdef NIX
            SCP.IUalpha=( int16_t) mycos(   winkel * 182 ) /32 ;
            SCP.IUbeta = ( int16_t) mysin(   winkel * 182 ) /32 ;

            //SCP.IUalpha = fix6_10_mul(SCP.IUalpha, (int16_t ) 5*1024 );

            
            //SCP.IUbeta = fix6_10_mul(SCP.IUbeta, (int16_t ) 5 *1024 );
            SCP.IUalpha = fix6_10_mul(SCP.IUalpha, (int16_t ) 9*1024 );

            SCP.IUbeta = fix6_10_mul(SCP.IUbeta, (int16_t )  9*1024 );
#endif

        SCP.Vq= SPIC.Uq;


        SCP.Vd =( int16_t ) 0*1024;
        print_str("Cord U ",7);
                 //print_int16( cordic6_10 ( SCP.IUalpha,SCP.IUbeta) );
                 //print_str("  ",2);
                 print_int16( cordic6_10 ( SCP.Vd,SCP.Vq) );

              print_str("Uq:",3);
             print_fix6_10( SCP.Vq );
             print_CR();
                 
        invClarkePark_30();

        CalcRefVec30();
#endif
        //SCP.Ia = SCP.V1;
        //SCP.Ib = SCP.V2;

// electrical angle of encoder
            // SCP.angle
            // SCP.Ia,SCP.ib
            //ClarkePark();
            // output SCP.Id, SCP.Iq

//#endif

            if (1) { //CP.sector !=oldangle

#ifdef NIX
                print_str("ADC:",4);
                print_int16( adcstate);
                print_str("  ",2);

                print_str("V  :",4);
                print_int16( value );
                print_str("  ",2);
#endif

#ifdef NIX
                if( adcstate == 5){
                    print_fix16( value );
                    print_str("  ",2);
                     }
#endif 
#ifdef NIX
                print_fix16( CP.V1_30 );
                print_str("  ",2);

                print_fix16( CP.V2_30 );
                print_str("  ",2);

                print_fix16( CP.V3_30 );
                print_CR();
#endif
                // Strom

               //print_str("win:",4);
               //print_int16( winkel );


                //print_str("Ialbet:",6);
                //print_int16( cordic6_10 ( SCP.Ialpha, SCP.Ibeta) );


                print_str("Iqref:",6);
                print_fix6_10( IqRef );

                //print_str("Iq :",4);
                //print_fix6_10( SCP.Iq );
                print_str("Poscnt",6);
                print_int16( POSCNT );
                //print_str("revenc",6);
                //print_int16( omega_el2cnt( SCP.angle, POSCNT ) );

                print_str("vel",6);
                print_int16( SPIC.velocity );


                print_str("index1",6);
                print_int16( index1 );
                //print_str("Idq   ",6);
                //print_int16(  cordic6_10( SCP.Id, SCP.Iq)  );

                //print_str("veloc",6);
                //print_int16( SPIC.velocity );
                //print_CR();

                //print_str("c d,q:",6);
                //print_int16( cordic6_10 ( SCP.Id, SCP.Iq) );

#ifdef NIX
                print_str("W_Ualbet",8);
                print_int16( cordic6_10 ( SCP.Ualpha, SCP.Ubeta) );
                print_str("Ualph:",6);
                print_fix6_10( SCP.Ualpha );
                print_str("Ubet:",6);
                print_fix6_10( SCP.Ubeta );

                print_str("cos:  ",6);
                print_fix6_10( SCP.cosangle);

                print_str("sin:  ",6);
                print_fix6_10( SCP.sinangle);
#endif
#ifdef NIX
                angle=SCP.angle;
                print_str("angle  ",6);
                print_int16( angle/182);
#endif

                //print_str("Poscnt",6);
                //print_int16( POSCNT );

#ifdef NIX
                print_str("cos:  ",6 );
                print_int16( mycos( angle));

                print_str("sin:  ",6 );
                print_int16( mysin( angle));
#endif
               //print_str("U   :",6);
               // print_fix6_10(  SCP.Vq );


                //print_str("Ts   :",6);
                //print_int16( SCP.Ts );
                //print_str("Tm   :",6);
                //print_int16( SCP.Tm );
                //print_str("Tl   :",6);
                //print_int16( SCP.Tl );


#ifdef NIX
                 print_str("T1   :",6);
                print_int16( SCP.P1 );

                 print_str("T2   :",6);
                print_int16( SCP.P2 );

                 print_str("T3   :",6);
                print_int16( SCP.P3 );
#endif
                // print_str("sect:",6);
                //print_int16( SCP.sector );

                //print_str("c_Iab :",6);
                //print_int16( cordic6_10 ( SCP.Ia,SCP.Ib) );

              //  print_str("Ia   :",6);
              //  print_fix6_10(  SCP.Ia );

              //  print_str("Ib   :",6);
              //  print_fix6_10(  SCP.Ib );
#ifdef NIX
                print_str("IUalp:",6);
                print_fix6_10( SCP.IUalpha );
                //print_str("Ia16:",5);
                //print_int16( SCP.IUalpha );
                //print_str("ADC1",4 );
                //print_int16( adc1 );
                print_str("IUbet:",6);

                //print_fix16( SCP.Ib );
                print_fix6_10( SCP.IUbeta );
                //print_str("Ib16:",5);
                //print_int16( SCP.IUbeta );
#endif
                //print_str("Sect:",5);
                //print_int16( SCP.sector);
                //print_fix6_10( fix6_10_mul( SCP.IUbeta,SCP.IUalpha ));
#ifdef NIX
                print_str("TA TB TC",8);
                print_int16(SCP.Ta);
                print_str("  ",2);
                print_int16(SCP.Tb);
                print_str("  ",2);
                print_int16(SCP.Tc);
#endif
                //print_str("ADC2",4 );
                //print_int16( adc2 );
                //print_CR();
#ifdef nix
                print_str("CordIab",7);
                 print_int16( cordic6_10 ( SCP.Ia,SCP.Ib) );

                print_str("CordV12",7);
                 print_int16( cordic6_10 ( SCP.V1,SCP.V2) );


                print_str("ang:",4);
                print_int16( angle/182 );
                print_str("POS:",4);
                print_int16( POSCNT );
                print_CR();

#endif
#ifdef NIX
                print_str("  ",2);

                print_int16( cordic( SCP.Ialpha,SCP.Ibeta) >>6);
                print_str("  ",2);
                // angle 0..0xffff 1/182 = 0x168
                print_int16( fix6_10_mul(0x168,angle<<16 )  );
                print_str("  ",2);
                // angle 0..0xffff 1/182 = 0x168
                print_int16( POSCNT  );
#endif

         //       print_CR();
#ifdef NIX


                    helpbuffer[0] = 'a';
                    helpbuffer[1] = 'd';
                    helpbuffer[2] = 'c';
                    helpbuffer[3] = adcstate;
                    helpbuffer[4] = ' ';
                    helpbuffer[5] = ' ';
                    helpbuffer[6] = ' '; //13;
                    helpbuffer[7] = ' '; //10;


            //itoa_1(value, &helpbuffer[1 ]);
                    RS232print(&helpbuffer[0], 6);
                    helpbufferc = 0;
                    //state = 0;
            __delay32((uint32_t) (80000L));


                    helpbuffer[0] = 'a';
                    helpbuffer[1] = ' ';
                    helpbuffer[2] = ' ';
                    helpbuffer[3] = ' ';
                    helpbuffer[4] = ' ';
                    helpbuffer[5] = ' ';
                    helpbuffer[6] = ' '; //13;
                    helpbuffer[7] = ' '; //10;

                    helpbuffer[7] = 13;
                    helpbuffer[8] = 10;


                    itoa_1(value, &helpbuffer[1]);
                    RS232print(&helpbuffer[0], 9);
                    helpbufferc = 0;
                   __delay32((uint32_t) (80000L));
#endif

#ifdef NIX

                    helpbuffer[0] = 'I';
                    helpbuffer[1] = ' ';
                    helpbuffer[2] = ' ';
                    helpbuffer[3] = ' ';
                    helpbuffer[4] = ' ';
                    helpbuffer[5] = ' ';
                    helpbuffer[6] = 13; //13;
                    helpbuffer[7] = 10; //10;

                    helpbuffer[7] = 13;
                    helpbuffer[8] = 10;
                    //z=(fix6_10_t) dac2current( value ) ;

                    itoa_1(lowfix2dec( z ), &helpbuffer[1]);
                    RS232print(&helpbuffer[0], 9);
                    helpbufferc = 0;

#endif
                   __delay32((uint32_t) (40000L));

            }
            //oldangle =SCP.sector;

#ifdef NIX


            pwm1 = mysin((angle) << 8);
            pwm2 = mysin(((angle + 85) << 8));
            pwm3 = mysin(((angle + 170) << 8));

            //11 bit PWM; 0.. 2047
            //10 bit

            pwm1 <<= 5;
            pwm1 += 1024;

            pwm2 <<= 5;
            pwm2 += 1024;

            pwm3 <<= 5;
            pwm3 += 1024;


            P1DC1 = (unsigned int) pwm1;
            P1DC2 = (unsigned int) pwm2;
            P1DC3 = (unsigned int) pwm3;
            RS232print("PWM ", 4);
            itoa_1(pwm1, &copybuffer[0]);
            RS232print(&copybuffer[0], 7);
            itoa_1(pwm2, &copybuffer[0]);
            RS232print(&copybuffer[0], 7);
            itoa_1(pwm3, &copybuffer[0]);
            RS232print(&copybuffer[0], 7);
#endif

            //angle+=8;
            //if (angle >20000)
            //    angle =0;
        }


        //#endif
        //redLEDON1;
        //writeb = PORTB;

        //writeb |= (1 << 5);
        //PORTB = writeb;
        //grnLEDON;
        //U1TXREG = 'H';

        //RS232print("hello\n\f",7);
     //   __delay32((uint32_t) (8000 * velocity));
        //grnLEDON1;

        //writeb &= ~(1 << 5);
        //PORTB = writeb;

        //redLEDOFF;
        //grnLEDOFF;
       // __delay32((uint32_t) (8000 * velocity));
        //redLEDOFF;
        //__delay32(20000000);

        /*redLEDON;
        grnLEDON;
        __delay32(40000000);
        redLEDOFF; */
        //grnLEDOFF;


    }

    return(0);
}
