
#include <stdint.h>




#include <libpic30.h>


#include <p33FJ12MC202.h>

#include "fix6_10.h"
#include "encoder.h"
#include "clarkepark.h"
#include "control.h"

uint16_t cnt[8]={0,285,571,856,1142,1427,1713,1999};


uint16_t cnt2omega_el(int16_t count)
{
int a;

for( a=0; a <6; a++ )
{
    //printf("%d %d \n", a, (int ) (a*onenininiby7 ) );
    if((cnt[a] <=count ) && (count< cnt[a+1] ))
	break;

}
//printf("%d %d %d \n", a,cnt[a], cnt[a+1]);

count-=cnt[a];

return ( 229 * count); // 0xffff/285.57
}


uint16_t omega_el2cnt( uint16_t omega_el, uint16_t real_count )
{
int a;
uint16_t count;

for( a=0; a <6; a++ )
{
    //printf("%d %d \n", a, (int ) (a*onenininiby7 ) );
    if((cnt[a] <=real_count ) && (real_count< cnt[a+1] ))
	break;

}
//printf("%d %d %d \n", a,cnt[a], cnt[a+1]);

count=cnt[a]+ omega_el/229;

return( count );

}




void initEncoder() {
    // RP13   Blue
    // RP11   Yellow
    // yellow channel A, Blue B, GREEN InDX
    // RPINR 14 QEAR ,QEBR
    // RPINR 15 INDX

    //ADPCFG |= 0x0038; // Configure QEI pins as digital inputs
    QEICONbits.QEIM = 0; // Disable QEI Module
    QEICONbits.CNTERR = 0; // Clear any count errors
    QEICONbits.QEISIDL = 0; // Continue operation during sleep
    QEICONbits.SWPAB = 0; // QEA and QEB not swapped
    QEICONbits.PCDOUT = 0; // Normal I/O pin operation
    QEICONbits.TQGATE = 0; // Normal I/O pin operation
    QEICONbits.TQCKPS = 2; // 1:64 prescale
    QEICONbits.POSRES = 1; // Index pulse resets position counter
    QEICONbits.TQCS = 0; // 1:64 prescale clock
    QEICONbits.UPDN_SRC = 0;

    DFLTCONbits.CEID = 1; // Count error interrupts disabled
    DFLTCONbits.QEOUT = 0; // Digital filters output enabled for QEn pins
    DFLTCONbits.QECK = 5; // 1:64 clock divide for digital filter for QEn
    POSCNT = 0; // Reset position counter
    QEICONbits.QEIM = 6; // X4 mode with position counter reset by Index
    MAXCNT = 1999;

    return;


}


#define CNTPROREV 1999
#define HALFCNTPROREV 999


//int16_t Pos=0;
//int16_t prevPos=0;

void calc_vel()
{
//static int16_t prevPos=0;
int16_t delta;


delta = SPIC.Pos - SPIC.prevPos;

if( delta >= 0 )
{
	// Delta > 0 either because
 	// 1) vel is > 0 or
 	// 2) Vel < 0 and
 	//encoder wrapped around
  if( delta >= HALFCNTPROREV )
	delta -= CNTPROREV;

}
else{
 // Delta < 0 either because
 // 1) vel is < 0 or
 // 2) Vel > 0 and wrapped around
  if( delta <= -HALFCNTPROREV )
	delta += CNTPROREV;

}

// scale 50 Hz , 20000/ 50 = 400
// after 400 ticks this routine is called.
// 50hz = 0.02 s
// omega = 2*pi /2000 * delta/ 0.02 s
// =  PI* 100 /2  *1/1000 *delta = pi* 0.05 *delta

// 0.1571 * delta;

SPIC.velocity = delta;
SPIC.prevPos= SPIC.Pos;

//printf("delta %d omega %f", delta, fix6_10_to_float ( delta* fix6_10_from_float( 0.1571 )));


}


void calc_deltax()
{
int16_t delta;


delta = SPIC.target - SPIC.Pos;

if( delta >= 0 )
{
	// Delta > 0 either because
 	// 1) vel is > 0 or
 	// 2) Vel < 0 and
 	//encoder wrapped around
  if( delta >= HALFCNTPROREV )
	delta -= CNTPROREV;

}
else{
 // Delta < 0 either because
 // 1) vel is < 0 or
 // 2) Vel > 0 and wrapped around
  if( delta <= -HALFCNTPROREV )
	delta += CNTPROREV;
}

SPIC.deltax= delta;

}
