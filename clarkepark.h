#pragma once
/* 
 * File:   clarkepark.h
 * Author: gs
 *
 * Created on 11. November 2014, 21:50
 */




#ifndef CLARKEPARK_H
#define	CLARKEPARK_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* CLARKEPARK_H */

#define PWM 1847
#define PWM_MIN 47


extern void CalcTimes();




struct sclarkepark
{
	unsigned int angle;
	fix6_10_t cosangle;
	fix6_10_t sinangle;
	fix6_10_t Ia;
	fix6_10_t Ib;
	fix6_10_t Ialpha;
	fix6_10_t Ibeta;

	fix6_10_t Ualpha;
	fix6_10_t Ubeta;
	fix6_10_t Id;
	fix6_10_t Iq;
	fix6_10_t Vd;
	fix6_10_t Vq;
	fix6_10_t V1,V2,V3;
	fix6_10_t V1_30,V2_30,V3_30;
	fix6_10_t  T1,T2;

	int16_t  Tl,Ts,Tm; // long short medium
        int16_t  P1,P2,P3;

	unsigned int sector;


};
extern struct sclarkepark SCP;

void invClarkePark_30();

void CalcRefVec30();

void ClarkePark();