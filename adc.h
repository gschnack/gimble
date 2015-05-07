/* 
 * File:   adc.h
 * Author: gs
 *
 * Created on 11. November 2014, 21:41
 */

#ifndef ADC_H
#define	ADC_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* ADC_H */

extern void initADC(void);
extern fix6_10_t dac2current( int32_t i );
extern void PWMinit(void);
extern void __attribute__((__interrupt__, auto_psv)) _ADC1Interrupt(void);


extern volatile int adc1, adc2 , adc3, adc0 ;
extern volatile int16_t IqRef;
extern volatile uint16_t index1;