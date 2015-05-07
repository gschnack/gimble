#include <stdint.h>



#include <libpic30.h>


#include <p33FJ12MC202.h>

#include "fix6_10.h"
#include "clarkepark.h"





//#define SQ3 fix6_10_from_float(1.73205080757)
#define SQ3by2 886
#define oneby12 84

// 0.866 = srt(3) /2 that is decimal 886


struct sclarkepark SCP;



void invClarkePark_30()
{

//#ifdef NIX

	SCP.Ualpha=fix6_10_mul( SCP.Vd , SCP.cosangle) - fix6_10_mul(SCP.Vq , SCP.sinangle );

	SCP.Ubeta= fix6_10_mul( SCP.Vd ,SCP.sinangle)+ fix6_10_mul(SCP.Vq , SCP.cosangle );
//#endif

   //     SCP.Ualpha=-SCP.Ualpha;
   //     SCP.Ubeta=-SCP.Ubeta;

// remove V1,V2,V3

//#ifdef inv_clark
	SCP.V1= SCP.Ualpha;
	SCP.V2=  -(SCP.Ualpha/2) + fix6_10_mul( (int16_t) SQ3by2, SCP.Ubeta);
  	SCP.V3= -(SCP.Ualpha/2) - fix6_10_mul( (int16_t) SQ3by2, SCP.Ubeta);
//#endif

	SCP.V1_30= SCP.Ubeta;
	SCP.V2_30=  -(SCP.Ubeta/2) + fix6_10_mul( (int16_t) SQ3by2, SCP.Ualpha);
  	SCP.V3_30= -(SCP.Ubeta/2) - fix6_10_mul( (int16_t) SQ3by2, SCP.Ualpha);


}

// Attention Ta,Tb need to be changed from the Microchip design

void CalcRefVec30()
{
  int16_t dPWM1,dPWM2,dPWM3;


    if( SCP.V1_30 >= 0 )
        {
        // (xx1)
        if( SCP.V2_30 >= 0 )
            {
            // (x11)
            // Must be Sector 3 since Sector 7 not allowed
            // Sector 3: (0,1,1)  0-60 degrees
            SCP.T1 = SCP.V2_30;   // decrease
            SCP.T2 = SCP.V1_30;  // increase

           CalcTimes();
            dPWM1 = SCP.Ts; // changed
            dPWM2 = SCP.Tm;
            dPWM3 = SCP.Tl;

	    SCP.sector =0;
            }
        else
            {
            // (x01)
            if( SCP.V3_30 >= 0 )
                {
                // Sector 5: (1,0,1)  120-180 degrees
                SCP.T1 = SCP.V1_30; // decrease
                SCP.T2 = SCP.V3_30;  // increase
                CalcTimes();
                dPWM1 = SCP.Tl;
                dPWM2 = SCP.Ts;
                dPWM3 = SCP.Tm;
SCP.sector =120;
                }
            else
                {
                // Sector 1: (0,0,1)  60-120 degrees
                SCP.T1 = -SCP.V2_30;
                SCP.T2 = -SCP.V3_30;
                CalcTimes();
                dPWM1 = SCP.Tm;
                dPWM2 = SCP.Ts;
                dPWM3 = SCP.Tl;
SCP.sector =60;
                }
            }
        }
    else
        {
        // (xx0)
        if( SCP.V2_30 >= 0 )
            {
            // (x10)
            if( SCP.V3_30 >= 0 )
                {
                // Sector 6: (1,1,0)  240-300 degrees
                SCP.T1 = SCP.V3_30;
                SCP.T2 = SCP.V2_30;
                CalcTimes();
                dPWM1 = SCP.Tm;
                dPWM2 = SCP.Tl;
               dPWM3 = SCP.Ts;
SCP.sector =240;
                }
            else
                {
                // Sector 2: (0,1,0)  300-0 degrees
                SCP.T1 = -SCP.V3_30;
               SCP.T2 = -SCP.V1_30;
                CalcTimes();
                dPWM1 = SCP.Ts;
                dPWM2 = SCP.Tl;
                dPWM3 = SCP.Tm;
SCP.sector =300;
                }
            }
        else
            {
            // (x00)
            // Must be Sector 4 since Sector 0 not allowed
            // Sector 4: (1,0,0)  180-240 degrees
            SCP.T1 = -SCP.V1_30;
            SCP.T2 = -SCP.V2_30;
            CalcTimes();
            dPWM1 = SCP.Tl;
	    dPWM2 = SCP.Tm;
            dPWM3 = SCP.Ts;
SCP.sector =180;
            }
        }

// achtung
  SCP.P1=dPWM1;
  SCP.P2=dPWM2;
  SCP.P3=dPWM3;

    P1DC1 = dPWM1; //1023; // 10 bits resolution
    P1DC2 = dPWM2; // 10 bits resolution
    P1DC3 = dPWM3;

	//printf("  PWM: %d %d %d S:%d",dPWM1,dPWM2,dPWM3, SCP.sector );
}


void CalcTimes()
{
fix6_10_t fDCbus;
int32_t t1,t2;
int32_t t16_t1,t16_t2;

SCP.T1= fix6_10_mul(SCP.T1,94 );   // fix6_10_from_float(0.092 ) normalize to 0..1 0.092 = 1/ 10.83
SCP.T2= fix6_10_mul(SCP.T2,94 );

t1=   ( int32_t) SCP.T1  * PWM ;
t2=   ( int32_t) SCP.T2  * PWM  ;

t1/= 1024;
t2/= 1024;


t16_t1 =  t1 ;
t16_t2 =  t2 ;

//printf("t1 %d t2 %d    ",t1,t2 );

SCP.Ts = (PWM -t16_t1-t16_t2)/2 ;
//if ( SCP.Tc < PWM_MIN)
//	SCP.Tc = PWM_MIN;

SCP.Tm= SCP.Ts + t16_t1;
SCP.Tl= SCP.Tm+ t16_t2;

//SCP.Ta =999;


}

#ifdef NIX
void CalcTimes()
{
fix6_10_t fDCbus;
int32_t t1,t2;
int32_t t16_t1,t16_t2;

SCP.T1= fix6_10_mul(SCP.T1,( int16_t) oneby12  );   // normalize to 0..1 0.0833 = 1/ 12
SCP.T2= fix6_10_mul(SCP.T2,( int16_t) oneby12 );

t1=   ( int32_t) SCP.T1  * PWM ;
t2=   ( int32_t) SCP.T2  * PWM  ;

t1/= 1024;
t2/= 1024;


t16_t1 =  t1 ;
t16_t2 =  t2 ;

//printf("t1 %d t2 %d    ",t1,t2 );

SCP.Tc = (PWM -t16_t1-t16_t2 -PWM_MIN) ; // shortest time

if ( SCP.Tc < PWM_MIN)
	SCP.Tc = PWM_MIN;

SCP.Tb= SCP.Tc + t16_t1;  // medium
SCP.Ta= SCP.Tb+ t16_t2;  // longest time


}

#endif

// needs SCP->angle, which is the electrical angle of the incremental encoder
// and SCP->Ia,SCP->Ib

#define ONEDIVSQ3 590

void ClarkePark()
{

	SCP.cosangle=( int16_t )  mycos( SCP.angle)/32; // into lower 10 fractional bits
	SCP.sinangle= ( int16_t ) mysin( SCP.angle )/32;

	SCP.Ialpha=SCP.Ia;

	SCP.Ibeta = fix6_10_mul(SCP.Ia, (int16_t ) ONEDIVSQ3 )+fix6_10_mul(SCP.Ib , (int16_t) ONEDIVSQ3 * 2 ) ;

	SCP.Id= fix6_10_mul(SCP.Ialpha , SCP.cosangle ) +fix6_10_mul(SCP.Ibeta , SCP.sinangle );

	SCP.Iq= - fix6_10_mul(SCP.Ialpha , SCP.sinangle ) + fix6_10_mul(SCP.Ibeta , SCP.cosangle );


        //SCP.Id =-SCP.Id;  // ## Achtung
        //SCP.Iq =-SCP.Iq;


}





