
#include <stdint.h>

#include <libpic30.h>

#include <p33FJ12MC202.h>
#include "fix6_10.h"
#include "cordic.h"







fix16_t fix16_mul(fix16_t inArg0, fix16_t inArg1)
{
        // Each argument is divided to 16-bit parts.
        //                                      AB
        //                      *        CD
        // -----------
        //                                      BD      16 * 16 -> 32 bit products
        //                               CB
        //                               AD
        //                              AC
        //                       |----| 64 bit product
        int32_t A = (inArg0 >> 16), C = (inArg1 >> 16);
        uint32_t B = (inArg0 & 0xFFFF), D = (inArg1 & 0xFFFF);

        int32_t AC = A*C;
        int32_t AD_CB = A*D + C*B;
        uint32_t BD = B*D;

        int32_t product_hi = AC + (AD_CB >> 16);

        // Handle carry from lower 32 bits to upper part of result.
        uint32_t ad_cb_temp = AD_CB << 16;
        uint32_t product_lo = BD + ad_cb_temp;
        if (product_lo < BD)
                product_hi++;


        return (product_hi << 16) | (product_lo >> 16);

}




fix16_t phiatan[17] = {0x5A0000,  0x2d0000,0x1a90a7,0xe0947,0x72001,0x3938b,0x1ca38,0xe52a,0x7297, 0x394c,
0x1ca5, 0xe52, 0x729, 0x394, 0x1ca,0xe5,0x72};





fix16_t cordic( fix16_t x, fix16_t y)
{
	int i=0;
	int xneg=0;
	fix16_t z;
	fix16_t zp1=0,xp1=0,yp1=0;




	fix16_t d;
	fix16_t  twoshift=fix16_one;
	fix16_t sum=0;
	//float f=0.0,fs=0.0;
	z=0; // fix16_mul (fix16_one, 30<<16 );

	//printf("x=%f y=%f z=%f\n",fix16_to_float(x),fix16_to_float(y),fix16_to_float(z));

	if( x<0 ){
		x=-x;
		xneg=1;
		}


	if ( y < 0 )
		d=-fix16_one;
	else
		d=fix16_one;

	xp1 = - fix16_mul(d , y);
	yp1 = fix16_mul( d ,x);
 	sum = fix16_mul(d,  phiatan[0]);
	x=xp1;
	y=yp1;
	for( i=1; i<16;i++ )
	{


	xp1= x- fix16_mul(d , fix16_mul (y,  twoshift));
	yp1 =y+ fix16_mul( d , fix16_mul (x,  twoshift));

	//zp1= z- fix16_mul(d,  phiatan[i]);
	sum+= fix16_mul(d,  phiatan[i]);

	x=xp1;
	y=yp1;
	//z=zp1;

	//printf("%d x=%f y=%f z=%f %f %f\n",i,fix16_to_float(x),fix16_to_float(y),fix16_to_float(z),fix16_to_float(fix16_mul(phiatan[i],d )),fix16_to_float(sum) );

	twoshift >>=1;
	if( y < 0 )
	 d=-fix16_one;
	else
	d=fix16_one;

	}
	if( xneg )
		sum=sum;
	else
	sum=-(sum- 180* fix16_one);

	if (sum <0)  // present from 0..359
		sum=360* fix16_one+sum;
	return( sum );
}



int16_t cordic6_10(fix6_10_t x,fix6_10_t y)
{
	fix16_t xf,yf;

	uint32_t r;
	xf = x ;
	xf*= 32;


	yf = y;
	yf*= 32;
	r= cordic( xf,yf ) >>16;
	return ( r);
}



int16_t cordic6_10enc(fix6_10_t x,fix6_10_t y)
{
	fix16_t xf,yf;

	uint32_t r;
	xf = x ;
	xf*= 32;


	yf = y;
	yf*= 32;
	r= cordic( xf,yf ) /360;
	return ( r);
}