/*
 * pwm.c
 *
 * Created: 30.06.2022 17:56:58
 * Author : sshmykov
 */ 
#define F_CPU 16000000L
#include <avr/io.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "pwm.h"

#define PWM_PIN 6
#define COUNT_PIN 3
#define TOP 255
#define ONE_SEC 1000

volatile extern uint8_t flag_rev;
volatile extern uint16_t time_fixation;
volatile extern uint8_t pulse_capture_flag;
volatile uint8_t interrupt_flag = 0;
volatile uint16_t tick_counter = 0;

ISR(TIMER1_COMPA_vect) {//couunt ticks from pulse to pulse
	cli();
	if (interrupt_flag) {
		tick_counter++;		
	}
	//couunt one second
	static uint16_t count_to_the_second = 0;
	count_to_the_second++;
	if (count_to_the_second == 43360) {
		count_to_the_second = 0;
		flag_rev = 1;
	}
	sei();
}

ISR(INT1_vect) {//pulse_capture
	cli();
	interrupt_flag = !interrupt_flag;
	if (interrupt_flag == 0) {
		if (pulse_capture_flag == 0) {
			time_fixation = tick_counter;
			tick_counter = 0;
			pulse_capture_flag = 1;
		}
		interrupt_flag = 1;
	}
	sei();
}

void timer_init(void) {
	// 43360.433604336045 Hz (16000000/((368+1)*1))
	//this frequency is sufficient to ensure the accuracy of the RPM
	OCR1A = 368;
 	// CTC
	TCCR1B |= (1 << WGM12);
	// Prescaler 1
	TCCR1B |= (1 << CS10);
	//Output Compare Match A Interrupt Enable
	TIMSK1 |= (1 << OCIE1A);
}

void counter_init(void) {
	DDRD &= ~(1 << COUNT_PIN);//INT1 Pin 3 of the arduino uno(PD3)
	PORTD |= (1 << COUNT_PIN); 

	EIMSK = (1 << INT1);
	//The rising edge of INT1 generates an interrupt request.
	EICRA |= (1 << ISC10);
	EICRA |= (1 << ISC11);
}

void pwm_init(void) {

	DDRD |= (1 << PWM_PIN);//Pin 6 of the arduino uno (PD6)

	
	TCCR0A |= (1 << WGM00) | (1 << COM0A1);
	TCCR0B |= (1 << WGM01) | (1 << CS00);
	TCNT0 = 0;

}

void setPwmDuty(uint8_t duty) {
	OCR0A = (uint16_t) (duty * TOP) / 100;
}


