#include <stdint.h>



#include <libpic30.h>


#include <p33FJ12MC202.h>

#include "fix6_10.h"
#include "adc.h"
#include "main.h"
#include "clarkepark.h"
#include "control.h"
#include "encoder.h"

volatile int adc1, adc2 , adc3, adc0 ;

volatile uint16_t index1 = 0;
volatile int16_t IqRef= 0;
volatile int16_t vd;

void __attribute__((__interrupt__, auto_psv)) _ADC1Interrupt(void) {

    grnLEDON1;

    SPIC.Pos=POSCNT;
    //if( (index1 % 100) ==0)
    //    calc_vel();
    //calc_deltax();

  //  if(SPIC.velocity > ((int16_t) 20 ) )
  //      IqRef = 0;

  //  if(IqRef != 0)
  //      index1++;


    //#ifdef FUNKTIONIERT
    index1++;
        


if( index1 == 100){
    SPIC.Pos=POSCNT;
    calc_vel();
    calc_deltax();
    IqRef= fix6_10_mul(SPIC.deltax ,(int16_t) 3000)   - fix6_10_mul( SPIC.velocity,  (int16_t )2000  )  ;
            //+ SPIC.velocity *1;
    //IqRef=0;

    vd=0;
    //if( (SPIC.deltax <50) && (SPIC.deltax >-50))
    //    vd=5000;
    index1=0;
}
//#endif
        

#ifdef NIX
     if( index1 == 400) // 20ms
     {
         if ((POSCNT > 5 ) & (POSCNT <1000))
             IqRef=-70;
         else
             if(POSCNT >1000 )
                 IqRef=70;
             else
                 IqRef=0;

index1=0;
     }
#endif
        
#ifdef NIX
    if( index1 == 1000 )
    {
        IqRef=-IqRef;
        index1=0;
    }
#endif
#ifdef NIX
    if ((index1 & 1) == 0) {
        grnLEDON1;
    } else {
        grnLEDOFF;
    }
#endif

    adc0 = ADC1BUF0;
    adc1 = ADC1BUF1;
    adc2 = ADC1BUF2;
    adc3 = ADC1BUF3;

    SCP.Ia=( fix6_10_t ) dac2current( adc1 );
    SCP.Ib=( fix6_10_t ) dac2current( adc2 );

    //SCP.angle = 0xff00;
//#ifdef NIX
    SCP.angle = cnt2omega_el((POSCNT+20) %2000);
//#endif
     // SCP.angle = winkel *182;
    // angle Ia, Ib fo ClarkePark
    //SCP.angle = 0;

    ClarkePark();
//#ifdef NIX

    //IqRef= (int16_t ) 200;

    SPIC.deltaIq= IqRef -SCP.Iq;
    tf_spi();
//#ifdef NIX
    SCP.Vq= (int16_t  ) SPIC.Uq; //IqRef;   //SPIC.Uq;
    //SCP.Vd =( int16_t ) 0*1024;
    SCP.Vd = (int16_t  ) 0; //vd; //4000; // 0;  // 0 // SPIC.Uq ; // SPIC.Uq; // ( int16_t ) 0;
//#endif
//#endif

  //  SCP.Vq= ( int16_t ) 0*1024;
    //SCP.Vd =( int16_t ) 2*1024;

    //SCP.IUalpha = SCP.cosangle;
    //SCP.IUbeta = SCP.sinangle;


  //SCP.angle = cnt2omega_el(POSCNT);
    invClarkePark_30();
   //if( IqRef !=0)
      CalcRefVec30();
    
    IFS0bits.AD1IF = 0;

grnLEDOFF;
}



void initADC(void) {

    //AD1PCFGL = 0x0ff8;				//Port pin multiplexed with AN0-AN3 in Analog mode
    AD1PCFGL = 0b0000111111011000; //Port pin multiplexed with AN0-AN2, AN5 in Analog mode
    AD1PCFGLbits.PCFG0 = 0;
    AD1PCFGLbits.PCFG1 = 0;
    AD1PCFGLbits.PCFG2 = 0;
    AD1PCFGLbits.PCFG3 = 1;
    AD1PCFGLbits.PCFG4 = 1;
    AD1PCFGLbits.PCFG5 = 0;


    TRISAbits.TRISA0 = 1; // pin 2 an0     		//set the used ADC pins as inputs
    TRISAbits.TRISA1 = 1; // pin 3 an1
    TRISBbits.TRISB0 = 1; // pin 4 an2
    TRISBbits.TRISB3 = 1; // pin 7 an5	poti		//AN5




    AD1CON1bits.ADON = 0; //turn off the A/D
    AD1CON1bits.ADSIDL = 1; //Stop module operation in Idle mode
    AD1CON1bits.AD12B = 0; //10-bit, 4-channel ADC operation
    //	AD1CON1bits.FORM = 0b01;		//Data Output Format bits Signed Int (ssss sssd dddd dddd)
    //	AD1CON1bits.FORM = 0b10;		//Data Output Format bits Signed Fraction (sddd dddd dd00 0000)
    AD1CON1bits.FORM = 0b00; //Data Output Format bits Integer (0000 00dd dddd dddd)
    //AD1CON1bits.SSRC = 0b000;		//Clearing sample bit ends sampling and starts conversion
    AD1CON1bits.SSRC = 0b011; // get input from special event PWM

    AD1CON1bits.SIMSAM = 1; //Samples CH0, CH1, CH2, CH3 simultaneously
    AD1CON1bits.ASAM = 1; //Sampling begins immediately after last conversion
    // change ASAM to non cont mode
    //AD1CON1bits.ASAM = 0;

    // A0-> Ch1 A1->CH2 A2->CH3 //

    AD1CHS123bits.CH123NB = 0; //MUX A CH1, CH2, CH3 negative input is VREF-
    AD1CHS123bits.CH123SB = 0; //MUX A CH1 positive input is AN0, CH2 positive input is AN1, CH3 positive input is AN2
    AD1CHS123bits.CH123NA = 0; //MUX B CH1, CH2, CH3 negative input is VREF-
    AD1CHS123bits.CH123SA = 0; //MUX B CH1 positive input is AN0, CH2 positive input is AN1, CH3 positive input is AN2

    AD1CHS0bits.CH0NB = 0; //MUX B Channel 0 negative input is VREF-
    AD1CHS0bits.CH0SB = 5; //MUX B Channel 0 positive input is AN5
    AD1CHS0bits.CH0NA = 0; //MUX A Channel 0 negative input is VREF-
    AD1CHS0bits.CH0SA = 5; //MUX A Channel 0 positive input is AN5

    AD1CSSL = 0x0000; //Skip all ANx channels for input scan

    AD1CON3bits.ADRC = 0; //ADC Clock derived from system clock
    AD1CON3bits.SAMC = 0; //Autosample time time bits = 2 TAD
    AD1CON3bits.ADCS = 2; //TAD = 2*TCY, TAD = 50 nSec (at 40MHz TCY)

    AD1CON2bits.VCFG = 0b000; //ADREF+ = AVDD ADREF- = AVSS
    AD1CON2bits.CSCNA = 0; //Do not scan inputs
    AD1CON2bits.CHPS = 0b11; //11 = Converts CH0, CH1, CH2, and CH3
    AD1CON2bits.BUFS = 0; //ADC is currently filling the first half of the buffer
    AD1CON2bits.SMPI = 0b0000; //Interrupts at the completion of conversion for every sample/convert sequence
    AD1CON2bits.BUFM = 0; //Always starts filling the buffer from the start address
    //AD1CON2bits.BUFM = 1;			//fill lower 8 then upper 8
    AD1CON2bits.ALTS = 0;

    IPC3bits.AD1IP = 2; //(highest priority interrupt = 7)
    IFS0bits.AD1IF = 0; //clear ADC interrupt flag
    IEC0bits.AD1IE = 1; //enable ADC interrupt

    AD1CON1bits.ADON = 1; //turn ADC on
    AD1CON1bits.SAMP = 1; //start sampling
}


// check again

#ifdef NIX
fix6_10_t dac2current( int32_t i )
{
fix6_10_t r;
if( i > 510 ){
 i-= 510;
 r=fix6_10_mul( 0xe9, i<<16 ); //0xe9 = 0.26 /73

}
 else{
  i=510-i;
  r=fix6_10_mul( -0xe9, i<<16 );
}

return(r );
}

#endif



fix6_10_t dac2current( int32_t i )
{
int16_t z16;
fix6_10_t r;



if( i > 510 ){
 i-= 510;
 z16 = 0x0e * i;    //, 0x0e )) ; // 0.26 /73   0x0e = 1/73   0x10a = 0.26
 r= fix6_10_mul (z16,  0x10a);


}
 else{
  i=510-i;
 z16 = 0x0e * i;    //, 0x0e )) ; // 0.26 /73   0x0e = 1/73   0x10a = 0.26
 r= fix6_10_mul (-z16,  0x10a);

}

return(r );
}






void PWMinit(void) {
    P1TCONbits.PTEN = 0; // disable
    P1TCONbits.PTSIDL = 0; // run in idle mode
    P1TCONbits.PTOPS = 0b1111; // postscale for interrupts not important
    P1TCONbits.PTCKPS = 0b00; // 1:1 prescale
    P1TCONbits.PTMOD = 0b10; // up down w/o interrupts

    P1TMRbits.PTMR = 0; // timer count value

    P1TPERbits.PTPER = 0x3ff; //39 Khz @40 Mhz TCY

    P1SECMPbits.SEVTDIR = 0; // interrupt on upwards counting
    P1SECMPbits.SEVTCMP =950;  // Very important it sts the time when AD
            //conversion starts, 950 means that AD is always when all the lower
            //MOSFETS are connected to ground             ;


    PWM1CON1bits.PMOD1 = 0; // complementary mode A1=H -> A2=L
    PWM1CON1bits.PMOD2 = 0;
    PWM1CON1bits.PMOD3 = 0;

    PWM1CON1bits.PEN1H = 1; // not i/O but PWM
    PWM1CON1bits.PEN2H = 1;
    PWM1CON1bits.PEN3H = 1;

    PWM1CON1bits.PEN1L = 0; // not i/O but PWM ###
    PWM1CON1bits.PEN2L = 0;
    PWM1CON1bits.PEN3L = 0;

    PWM1CON2bits.SEVOPS = 0; // postscale on every up/down int
    PWM1CON2bits.IUE = 0; // noz updated immediately
    PWM1CON2bits.UDIS = 0; // enable updates
    PWM1CON2bits.OSYNC = 0;

    // dead time
    P1DTCON1bits.DTAPS = 0; // no prescale TCy
    P1DTCON1bits.DTBPS = 0; // no prescale TCy

    P1DTCON1bits.DTA = 0b00100; // 8* TCy
    P1DTCON1bits.DTB = 0b00100;

    P2DTCON2bits.DTS1A = 0; // all from unit A

    P1FLTACONbits.FAEN1 = 0; // faults are disabled
    P1FLTACONbits.FAEN2 = 0;
    P1FLTACONbits.FAEN3 = 0;


    P1OVDCONbits.POVD1L = 1; // no overwrite output determined by PWM
    P1OVDCONbits.POVD2L = 1;
    P1OVDCONbits.POVD3L = 1;

    P1OVDCONbits.POVD1H = 1;
    P1OVDCONbits.POVD2H = 1;
    P1OVDCONbits.POVD3H = 1;

    P1DC1 = 0; //1023; // 10 bits resolution
    P1DC2 = 0; //512; // 10 bits resolution
    P1DC3 = 0; //20; // 10 bits resolution
    P1TCONbits.PTEN = 1; // enable

}