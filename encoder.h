//#pragma once

/* 
 * File:   encoder.h
 * Author: gs
 *
 * Created on 11. November 2014, 21:20
 */
#include <stdint.h>


#ifndef ENCODER_H
#define	ENCODER_H

#ifdef	__cplusplus
extern "C" {
#endif





#ifdef	__cplusplus
}
#endif

#endif	/* ENCODER_H */


extern uint16_t cnt2omega_el(int16_t count);
extern uint16_t omega_el2cnt( uint16_t omega_el, uint16_t real_count );

extern void initEncoder();
extern void calc_vel();

extern uint16_t cnt[8];