/*
 * pwm.c
 *
 * Created: 30.06.2022 17:56:58
 * Author : sshmykov
 */ 
#define F_CPU 16000000L
#include <avr/io.h>
#include <util/delay.h>

#define PWM_FREQ_HZ 25000L
#define PWM_PIN 1

#define TCNT1_TOP 320
#define ONE_SEC 1000

void pwm_init(void) {


	DDRB |= (1 << PWM_PIN);//Pin 9 of the arduino uno (PB1)
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1 = 0;
	TCCR1A |= (1 << COM1A1) | (1 << WGM11);
	TCCR1B |= (1 << WGM13) | (1 << CS10);
	ICR1 = (uint16_t) TCNT1_TOP;
}

void setPwmDuty(uint8_t duty) {
	OCR1A = (uint16_t) (duty * TCNT1_TOP) / 100;
}


