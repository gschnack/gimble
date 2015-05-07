/* 
 * File:   fix6_10.h
 * Author: gs
 *
 * Created on 11. November 2014, 20:51
 */


#pragma once

#ifndef FIX6_10_H
#define	FIX6_10_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* FIX6_10_H */

typedef int16_t fix6_10_t;

extern fix6_10_t fix6_10_from_float(float a);


extern fix6_10_t fix6_10_mul(fix6_10_t inArg0, fix6_10_t inArg1);

extern const fix6_10_t fix6_10_one;   /*!< fix6_10_t value of 1   0..9 is below 1 */

extern uint16_t lowfix2dec( fix6_10_t angle);
extern int16_t highfix2dec( fix6_10_t angle);




int16_t mysin(uint16_t angle);
int16_t mycos(uint16_t angle);