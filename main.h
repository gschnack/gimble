/* 
 * File:   main.h
 * Author: gs
 *
 * Created on 11. November 2014, 22:52
 */

#ifndef MAIN_H
#define	MAIN_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* MAIN_H */



#define grnLEDON		PORTBbits.RB2 = 1; __builtin_nop()
#define grnLEDON1		PORTB =(1 <<2) | LATB ; __builtin_nop()

#define grnLEDOFF		PORTBbits.RB2 = 0; __builtin_nop()

#define redLEDON		PORTBbits.RB5 = 1; __builtin_nop()
#define redLEDON1		PORTB =(1 <<5)| LATB; __builtin_nop()

#define redLEDOFF		PORTBbits.RB5 = 0; __builtin_nop()

extern int16_t winkel;