/*
 * UartBlower.c
 * Answering on command
 * Author : sshmykov
 */ 
 /*
In this branch I am trying to make a rotation counter.
Add new Timer in pwm.c
 */
#define F_CPU 16000000L
#include <avr/io.h>
#include <avr/interrupt.h>
//#include <avr/sleep.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "pwm.h"
#include "uart.h"

char buff_rx[buff_rx_SIZE] = {0};
char *buff_rx_begin = buff_rx;

volatile uint8_t pos = 0;
volatile uint8_t j = 0;
volatile uint8_t flag = 0;
volatile uint8_t pwm = 0;
//volatile uint16_t number_of_pulses = 0;
//volatile uint32_t rev_per_min = 0;
volatile uint8_t flag_rev = 0;
volatile uint16_t analys_increment = 0;
uint32_t time_between_ticks[DEPTH_OF_ANALYSIS] = {0};

const char command_on[] = "On\r";
const char command_off[] = "Off\r";
const char command_pwm[] = "Power";

volatile uint8_t answer = 2;
volatile char flag_recive = 0;

int main(void) {
	pwm_init();
	timer_init();
	counter_init();
	UartInit();
	sei();
	
	char buff_to_send[SIZEOF_SENDBUF];
	sprintf((char*)buff_to_send, "mode:OFF [power: 0%%]\r\n");
	DebagUart(buff_to_send);
	UCSR0B |= (1 << UDRIE0);
	
	while (1) {
		if (flag_recive) {
			const char space[] = " ";
			char *until_space;
			char *after_space;
			until_space = strtok (buff_rx_begin, space);
			after_space = strtok (NULL, space);
			if ( after_space != NULL){
				*(after_space + 3) = '\0';
				pwm = strtol(after_space,0, 10);
				if (errno == ERANGE) {
					pwm = 100;//overflow
					errno = 0;
				} else {
					if (pwm > 100) {
						pwm = 100;
					} else {
						if (pwm <= 0) {
						pwm = 0;
						}
					}
				}
			}
			else {//str havnt spaces
				until_space = buff_rx_begin;
			}
			if (!strcmp((char *)until_space, command_off)){
				setPwmDuty(0);
				sprintf((char*)buff_to_send, "mode:OFF [power: 0%%]\r\n");
			} else {
				if (!strcmp((char *)until_space, command_on)){
					setPwmDuty(pwm);
					sprintf((char*)buff_to_send, "mode:ON   [power: %d%%]\r\n", pwm);
				} else {
					if (!strcmp((char *)until_space, command_pwm)){
						if (pwm) {
							setPwmDuty(pwm);
							sprintf((char*)buff_to_send, "mode:ON   [power: %d%%]\r\n", pwm);
						} else {
							setPwmDuty(0);
							sprintf((char*)buff_to_send, "mode:OFF [power: 0%%]\r\n");
						}
					} else {
						sprintf((char*)buff_to_send, "Error\r\n");
					}
				}
			}
			DebagUart(buff_to_send);
			UCSR0B |= (1 << UDRIE0);
			flag_recive = 0;
		} 
		if (flag_rev) {
			uint32_t max_time = time_between_ticks[0];
			uint32_t min_time = time_between_ticks[0];
			for (analys_increment = 0; analys_increment  < DEPTH_OF_ANALYSIS; analys_increment++){
				if(time_between_ticks[analys_increment] > max_time) {
					max_time = time_between_ticks[analys_increment];
				}
				if(time_between_ticks[analys_increment] <  min_time) {
						min_time = time_between_ticks[analys_increment];
				}
			}
			uint32_t mid_time_per_rev = (max_time + min_time);
			/*
			in one revolution we have two pulses, so
			mid_time = (max_time + min_time) /2;
			mid_time_per_rev = mid_time * 2;// or mid_time_per_rev = ((max_time + min_time)/2 )* 2 ;
			*/
			//1 tick timmer1 = 1 tick CPU. 1tick -> 1/F_CPU
			long double real_time = ((long double)mid_time_per_rev / F_CPU);//one rotation time = ticks_between_pulses * one_tick_CPU_time
			uint16_t rev_per_sec = 1 /real_time;//RPS = 1sec / one rotation time
			uint32_t rev_per_min = rev_per_sec * 60;//RPM = RPS * 60
			




			sprintf((char*)buff_to_send, "rev: %lu RPM\r\n", rev_per_min);
			DebagUart(buff_to_send);
			UCSR0B |= (1 << UDRIE0);
			memset(time_between_ticks, 0, 10);
			flag_rev = 0;
		}
	}
}


