/*
 * pwm.c
 *
 * Created: 30.06.2022 17:56:58
 * Author : sshmykov
 */ 
#define F_CPU 16000000L
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define PWM_FREQ_HZ 25000L
#define PWM_PIN 1

#define TCNT1_TOP 320
#define ONE_SEC 1000

volatile extern uint8_t flag_rev;
volatile extern uint16_t counter;
//volatile extern uint32_t rev_per_min;
volatile extern uint16_t rev_per_min;

ISR(TIMER0_COMPA_vect) {//timer interrupt once per second
cli();
	static uint8_t timer_hold = 100;
	timer_hold--;
	if (timer_hold == 0) {
		//rev_per_min = counter * 60;
		rev_per_min = (uint32_t)counter;
		counter = 0;
		timer_hold = 100;
		flag_rev = 1;
	}
sei();
}

ISR(INT0_vect) {
cli();
	counter++;
sei();
}

void timer_init(void) {
	//Set the timer to overflow after one second
	//The minimum frequency that can be achieved is 100Hz
	//but need 1Hz
	TCCR0A |= (1 << WGM01);//ctc mode
	OCR0A = 155;
	TCCR0B = (1 << CS00) | (1 << CS02);//15625Hz
	TIMSK0 |= (1 << OCIE0A);
	TCNT0 = 0; 
}

void counter_init(void) {
	DDRD &= ~(1 << PIND2);//Pin 2 of the arduino uno(PD2)
	PORTD |= (1 << PIND2); 

	EIMSK = (1 << INT0);
	EICRA |= (1 << ISC00);
	EICRA &= ~(1 << ISC01);
}

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


