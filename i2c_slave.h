/* 
 * File:   ic2_slave.h
 * Author: gs
 *
 * Created on 11. November 2014, 20:19
 */

#include "fix6_10.h"



void I2C1_Init( void );
void __attribute__ ( (interrupt, no_auto_psv) ) _SI2C1Interrupt( void );
void I2Cprint(char *source, int len);


