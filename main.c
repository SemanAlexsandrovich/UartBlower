/*
 * UartBlower.c
 * Answering on command
 * Author : sshmykov
 */ 
 /*
this is the final version of the project without the LCD display
 */
#define F_CPU 16000000L
#define TICK_PWM 23//(1/F_PWM) = 1/43360 = 23/1 000 000
#include <avr/io.h>
#include <avr/interrupt.h>
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
volatile uint16_t time_fixation = 0;
volatile uint8_t flag_rev = 0;
volatile uint16_t analys_increment = 0;
volatile uint8_t pulse_capture_flag = 0;
volatile uint32_t max_ticks_between_pulses = 0;
volatile uint32_t min_ticks_between_pulses = 65535;
volatile uint16_t rev_per_min = 0;
volatile uint16_t rev_per_sec = 0;
volatile uint32_t mid_ticks_between_pulses;
volatile uint32_t real_time_between_pulses;
volatile uint32_t real_rev_time;

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
		if (pulse_capture_flag) {
			if (time_fixation > max_ticks_between_pulses) {
				max_ticks_between_pulses = time_fixation;
			}
			if (time_fixation < min_ticks_between_pulses) {
				min_ticks_between_pulses = time_fixation;
			}
			pulse_capture_flag = 0;
		}
		if (flag_rev) { 
			if (max_ticks_between_pulses >= min_ticks_between_pulses) {
				mid_ticks_between_pulses = ((max_ticks_between_pulses + min_ticks_between_pulses) >> 1);
				real_time_between_pulses = (mid_ticks_between_pulses * TICK_PWM);//real_time_between_pulses * 1 000 000
				real_rev_time = 2 * real_time_between_pulses;//real_rev_time (* 1000
				rev_per_sec = 1000000/real_rev_time;//RPS = 1sec/real_rev_time		
			} else {
				rev_per_sec = 0;
			}
			rev_per_min = rev_per_sec * 60;//RPM = RPS * 60
			sprintf((char*)buff_to_send, "rev: %d RPM\r\n", rev_per_min);
			DebagUart(buff_to_send);
			UCSR0B |= (1 << UDRIE0);
			max_ticks_between_pulses = 0;
			min_ticks_between_pulses = 65535;
			flag_rev = 0;
		}
	}
}


