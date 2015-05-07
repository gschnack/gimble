
#include <stdint.h>

#include <libpic30.h>

#include <p33FJ12MC202.h>
#include "fix6_10.h"


#ifdef NIX
int SinTable[128]={
    0,1608,3212,4808,6393,7962,9512,11039,
    12540,14010,15446,16846,18205,19520,20787,22005,
    23170,24279,25330,26319,27245,28106,28898,29621,
    30273,30852,31357,31785,32138,32413,32610,32728,
    32767,32728,32610,32413,32138,31785,31357,30852,
    30273,29621,28898,28106,27245,26319,25330,24279,
    23170,22005,20787,19520,18205,16846,15446,14010,
    12540,11039,9512,7962,6393,4808,3212,1608,
    0,-1608,-3212,-4808,-6393,-7962,-9512,-11039,
    -12540,-14010,-15446,-16846,-18205,-19520,-20787,-22005,
    -23170,-24279,-25330,-26319,-27245,-28106,-28898,-29621,
    -30273,-30852,-31357,-31785,-32138,-32413,-32610,-32728,
    -32767,-32728,-32610,-32413,-32138,-31785,-31357,-30852,
    -30273,-29621,-28898,-28106,-27245,-26319,-25330,-24279,
    -23170,-22005,-20787,-19520,-18205,-16846,-15446,-14010,
    -12540,-11039,-9512,-7962,-6393,-4808,-3212,-1608};

#endif

// removed highest values

int16_t SinTable[128]={
    0,1608,3212,4808,6393,7962,9512,11039,
    12540,14010,15446,16846,18205,19520,20787,22005,
    23170,24279,25330,26319,27245,28106,28898,29621,
    30273,30852,31357,31785,32138,32413,32610,32728,
    32765,32728,32610,32413,32138,31785,31357,30852,  // here 1 number
    30273,29621,28898,28106,27245,26319,25330,24279,
    23170,22005,20787,19520,18205,16846,15446,14010,
    12540,11039,9512,7962,6393,4808,3212,1608,
    0,-1608,-3212,-4808,-6393,-7962,-9512,-11039,
    -12540,-14010,-15446,-16846,-18205,-19520,-20787,-22005,
    -23170,-24279,-25330,-26319,-27245,-28106,-28898,-29621,
    -30273,-30852,-31357,-31785,-32138,-32413,-32610,-32728,
    -32765,-32728,-32610,-32413,-32138,-31785,-31357,-30852, // here 1 number 
    -30273,-29621,-28898,-28106,-27245,-26319,-25330,-24279,
    -23170,-22005,-20787,-19520,-18205,-16846,-15446,-14010,
    -12540,-11039,-9512,-7962,-6393,-4808,-3212,-1608};



 const fix6_10_t fix6_10_one = 1024;



/* Conversion functions between fix6_10_t and float/integer.
 * These are inlined to allow compiler to optimize away constant numbers
 */

 fix6_10_t fix6_10_from_int(int a)     { return a * fix6_10_one; }

//static inline fix6_10_t fix6_10_from_lower(int a)     { return (fix6_10_t ) a; }


 float   fix6_10_to_float(fix6_10_t a) { return (float)a / fix6_10_one; }
 double  fix6_10_to_dbl(fix6_10_t a)   { return (double)a / fix6_10_one; }




  int fix6_10_to_int(fix6_10_t a){    return (a >> 10); }


inline fix6_10_t fix6_10_from_float(float a)
{
        float temp = a * (fix6_10_one);


        return (fix6_10_t)temp ;

}



fix6_10_t fix6_10_mul(fix6_10_t inArg0, fix6_10_t inArg1)
{
int32_t r= (int16_t) inArg0;

r *=(int16_t)  inArg1;

r/=1024;

	return( (fix6_10_t) r );

}



int16_t mycos(uint16_t angle)
{
  int16_t h, hplus1;
  int32_t  delta;
 int16_t d;

  //angle += 16384; // shift 90 degree = 2^16 /4
  h=( int16_t ) (angle >> 9); //Higher bits 9-16 give table index
  h=(h+32) & 127;   // shift 90 degree = 128 /4

 

  hplus1= (h + 1 ) & 127;  // if h == 127 -> h=0


  delta = ((( int32_t ) SinTable[hplus1] -  ( int32_t ) SinTable[h]) * ((int32_t ) angle & 511 )); // 512 ;
  //d=( int16_t ) (delta >> 9);
  delta /= 512;
  d=( int16_t ) delta;
  //printf("h %d  h+1 %d delta %d", h, hplus1, ( int ) d );

  return( SinTable[h]+d )	;

}


int16_t mysin(uint16_t angle)
{
  int16_t h, hplus1;
  int32_t  delta;
 int16_t d;

  //angle += 16384; // shift 90 degree = 2^16 /4
  h=( int16_t ) (angle >> 9); //Higher bits 9-16 give table index
  h=h & 127;   // shift 90 degree = 128 /4

  hplus1= (h + 1 ) & 127;  // if h == 127 -> h=0


  delta = ((( int32_t )SinTable[hplus1] - ( int32_t ) SinTable[h]) * ((int32_t ) angle & 511 )); // 512 ;
  //d=( int16_t ) (delta >> 9);
  delta /= 512;
  d=( int16_t ) delta;

  //d=( int16_t ) (delta /512);
  //printf("h %d  h+1 %d delta %d", h, hplus1, ( int ) d );

  return( SinTable[h]+d )	;

}



#ifdef NIX


int16_t highfix2dec( fix6_10_t angle)
{
int16_t r;

if( angle <0 )
	{
	angle*= -1;
	angle >>= 10;
	r=angle;
	r*=-1;
	}
 else{
   angle >>= 10;
   r=  angle;
}
//printf("angle=%x  r=%x\n", r,angle );
return (r );

}



// converts the lower 10 bits of the fractional part  into a decimal intger from 0-999
// if negative the fractional part is converted positive, therefore unsigned integer




uint16_t lowfix2dec( fix6_10_t angle)
{
uint16_t r;
if( angle < 0 ) angle*=-1;

angle=angle  & 0xffff ;
angle >>= 6;
angle= fix6_10_mul(fix6_10_from_float( 0.9766) ,angle ) ; // 0.9766 = 1/1024
r=( int16_t) angle ;   // this conversion takes the higher part of angle to r

return (r );

}

#endif




int16_t highfix2dec( fix6_10_t angle)
{
int16_t r;

if( angle <0 )
	{
	angle*= -1;
	angle >>= 10;
	r=angle;
	//r*=-1;   // return always positive
	}
 else{
   angle >>= 10;
   r=  angle;
}
//printf("angle=%x  r=%x\n", r,angle );
return (r );

}






// converts the lower 10 bits of the fractional part  into a decimal intger from 0-999
// if negative the fractional part is converted positive, therefore unsigned integer

uint16_t lowfix2dec( fix6_10_t angle1)
{
uint16_t r;
uint32_t a;
if( angle1 < 0 ) angle1*=-1;

a = (int32_t ) angle1  & 1023 ;

a= (int32_t) 976 *angle1 ;
//a/=100;
a >>= 10 ;        // 0.9766 = 1/1024 *1000   0.0009765
r=( int16_t) a ;   


#ifdef NIX
angle1= fix6_10_mul(fix6_10_from_float( 0.9766) ,angle1 ) ; // 0.9766 = 1/1024
r=( int16_t) angle1 ;   // this conversion takes the higher part of angle to r
#endif

return (r );

}





