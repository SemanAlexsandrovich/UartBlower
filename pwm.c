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

//#define PWM_FREQ_HZ 25000L
#define PWM_PIN 6
#define COUNT_PIN 3
#define TOP 255
#define ONE_SEC 1000

volatile extern uint8_t flag_rev;
//volatile extern uint16_t number_of_pulses;
volatile extern uint32_t rev_per_min;
volatile extern uint16_t analys_increment;
extern uint32_t time_between_ticks[DEPTH_OF_ANALYSIS];

volatile uint8_t point_flag = 1;
volatile uint8_t first_interrrupt_flag = 1;
volatile uint16_t starting_point = 0;
volatile uint16_t end_of_point = 0;

ISR(TIMER1_COMPA_vect) {//timer interrupt once per second
	cli();
	/*
		приведение времени между импульсами к оборотам 
		сделаю в основном цикле,чтобы не нагружать прерывание.
		возможно нужно перенести всю эту обработку в основной цикл а сдесь лишь поднять флаг 
	*/

	flag_rev = 1;

	sei();
}

ISR(INT1_vect) {
	cli();
	//the first interruption must be skipped
	if (first_interrrupt_flag) {
		if (point_flag) {//end previos impulse
			starting_point = TCNT1;
		} else {//begin new impulse
			end_of_point = TCNT1;
			/*
			after obtaining the necessary information 
			(end of the previous pulse, start of a new pulse) 
			we will immediately analyze.
			*/
			if (end_of_point < starting_point) {
				end_of_point = (uint32_t)end_of_point + 65536;
			}
			if (analys_increment  < DEPTH_OF_ANALYSIS){
				time_between_ticks[analys_increment] = end_of_point - (uint32_t)starting_point;
				analys_increment++;
			}
			//нужно ли приводить типы обратно?
			starting_point = (uint16_t)starting_point;
			end_of_point = (uint16_t)end_of_point;
			/*
			необходимо сделать анализ time_between_ticks:
			выбрать максимальное и минимальное
			чтобы не засорять это прерывание, сделаю анализ в прерывании по секунде.
			А здесь сохраню в массив лишь 10 значений time_between_ticks.
			так как у нас происходит около 1000 импульсов в секуду, 
			то для большей точности нужен массив из 1000 
			элементов с time_between_ticks, но это очень затратно
			*/
		}
		point_flag = !point_flag;
	}
	first_interrrupt_flag = 1;
	sei();
}

void timer_init(void) {
  	// 1 Hz (16000000/((15624+1)*1024))
	OCR1A = 15624;
	// CTC
	TCCR1B |= (1 << WGM12);
	// Prescaler 1024
	TCCR1B |= (1 << CS12) | (1 << CS10);
	// Output Compare Match A Interrupt Enable
	TIMSK1 |= (1 << OCIE1A);
	TCNT1 = 0; 
	memset(time_between_ticks, 0, 10);
}

void counter_init(void) {
	DDRD &= ~(1 << COUNT_PIN);//Pin 3 of the arduino uno(PD2)
	PORTD |= (1 << COUNT_PIN); 

	EIMSK = (1 << INT1);
	/*
	Any logical change on INT0 generates an interrupt request.
	does not fit because for one tachometer signal 
	we will increase the counter by 2, not by 1
	*/
	//The rising edge of INT1 generates an interrupt request.
	EICRA |= (1 << ISC10);
	EICRA |= (1 << ISC11);
}

void pwm_init(void) {

	DDRD |= (1 << PWM_PIN);//Pin 9 of the arduino uno (PB1)

	
	TCCR0A |= (1 << WGM00) | (1 << COM0A1);
	TCCR0B |= (1 << WGM01) | (1 << CS00);
	TCNT0 = 0;

}

void setPwmDuty(uint8_t duty) {
	OCR0A = (uint16_t) (duty * TOP) / 100;
}


