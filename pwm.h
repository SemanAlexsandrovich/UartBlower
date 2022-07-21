/*
 * pwm.h
 *
 * Created: 19.07.2022 12:54:56
 *  Author: sshmykov
 */ 


#ifndef PWM_H_
#define PWM_H_

#define DEPTH_OF_ANALYSIS 10
void pwm_init(void);
void setPwmDuty(uint8_t);
void counter_init(void);
void timer_init(void);

#endif /* PWM_H_ */