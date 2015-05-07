/* 
 * File:   uart.h
 * Author: gs
 *
 * Created on 11. November 2014, 20:19
 */

#ifndef UART_H
#define	UART_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* UART_H */


#include "fix6_10.h"


//extern int16_t fix6_10_t;

#define MAX_LENGTH 32




extern void InitUART();
extern void __attribute__((__interrupt__, no_auto_psv)) _U1RXInterrupt(void);
extern void __attribute__((__interrupt__, no_auto_psv)) _U1TXInterrupt(void);
extern void RS232print(char *source, int len);
extern int RS232received(void);
extern char RS232getch(void);

extern void print_str(char *S,int l);
extern void print_CR();

extern void print_int16( int16_t value);


extern void print_fix6_10( fix6_10_t value);

extern int myAtoi(char *str);

extern void itoa_1(unsigned int n, char *s);