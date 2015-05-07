#pragma once

/* 
 * File:   control.h
 * Author: gs
 *
 * Created on 2. Dezember 2014, 10:07
 */



#ifndef CONTROL_H
#define	CONTROL_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* CONTROL_H */





#include <stdint.h>
#include "fix6_10.h"


extern struct spictrl1 SPIC;



struct spictrl1
{
	fix6_10_t deltaIq;
	fix6_10_t deltaIqlast;
	fix6_10_t Uq;
	fix6_10_t Uqlast;

	fix6_10_t sK1;
	fix6_10_t sK2;
	fix6_10_t dcBus;

        int16_t Pos;
        int16_t prevPos;
        int16_t velocity;
        int16_t target;
        int16_t deltax;

};



void init_spic();

void tf_spic();