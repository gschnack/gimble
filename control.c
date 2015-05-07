//#pragma once

#include <stdint.h>

#include "fix6_10.h"
#include "control.h"
#include "main.h"





struct spictrl1 SPIC;
/*

y1=U/I

Transfer function 'Hpi' from input 'u1' to output ...

      3.6 s + 3720
 y1:  ------------
           s


Transfer function 'Hpiz' from input 'u1' to output ...

      3.6 z - 3.414
 y1:  -------------
          z - 1

Sampling time: 5e-05 s


*/

void init_spic()
{
//#ifdef NIX
  SPIC.Uqlast = 0; //fix6_10_from_float(0.0);
  SPIC.deltaIqlast=0; // = fix6_10_from_float(0.0);

  SPIC.sK1 = fix6_10_from_float(3.6);
  SPIC.sK2 = fix6_10_from_float(3.414);

SPIC.dcBus = fix6_10_from_float(8.0);
//#endif

SPIC.prevPos=0;
SPIC.Pos=0;
SPIC.velocity=0;
}



// needs input delta Iq
// output Uq

void tf_spi()
{
//#ifdef NIX
	SPIC.Uq=SPIC.Uqlast+ fix6_10_mul(SPIC.sK1, SPIC.deltaIq)  - fix6_10_mul(SPIC.sK2, SPIC.deltaIqlast );

//printf(" %f  ",fix6_10_to_float(fix6_10_mul(SPIC.sK1, SPIC.deltaIq) ));


	if( SPIC.Uq >SPIC.dcBus )
		SPIC.Uq = SPIC.dcBus;


	if( SPIC.Uq < -SPIC.dcBus )
		SPIC.Uq = -SPIC.dcBus;

	SPIC.deltaIqlast=SPIC.deltaIq;
	SPIC.Uqlast=SPIC.Uq;

  //      #endif
}
