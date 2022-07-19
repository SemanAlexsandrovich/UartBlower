/*
 * UartBlower.c
 * Answering on command
 * Author : sshmykov
 */ 
 /*
 I will add all the UART functionality to UART.c 
 */
#define F_CPU 16000000L
#include <avr/io.h>
#include <avr/interrupt.h>
//#include <avr/sleep.h>
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

char command_on[] = "On\r";
char command_off[] = "Off\r";
char command_pwm[] = "Power";

volatile uint8_t answer = 2;

#define POWER_MODE_OFF "mode:OFF [power: 0%%]\r\n"
#define POWER_MODE_ON "mode:ON   [power: %d%%]\r\n"
#define ERROR "Error\r\n"

volatile char flag_recive = 0;

int main(void) {
	pwm_init();
	UartInit();
	sei();
	
	char buff_to_send[SIZEOF_SENDBUF];
	sprintf((char*)buff_to_send,POWER_MODE_OFF);
	DebagUart(buff_to_send);
	UCSR0B |= (1 << UDRIE0);
	
	while (1) {
		if (flag_recive) {
			const char space[] = " ";
			volatile char *until_space;
			const char *after_space;
			until_space = strtok (buff_rx_begin, space);
			after_space = strtok (NULL, space);
			if ( after_space != NULL){
				pwm = strtol(after_space,0, 10);
				//pwm = atoi(after_space);
				/*
				#include <errno.h>
				if (pwm == LONG_MIN && errno == ERANGE) {
				…underflow…
				} else if (pwm == LONG_MAX && errno == ERANGE) {
				…overflow…
				}
				*/
				if (pwm > 100) {
					pwm = 100;
				} else {
					if (pwm <= 0) {
					pwm = 0;
					}
				}
			}
			else {//str havnt spaces
				until_space = buff_rx_begin;
			}
			if (!strcmp((char *)until_space, command_off)){
				setPwmDuty(0);
				sprintf((char*)buff_to_send,POWER_MODE_OFF);
			} else {
				if (!strcmp((char *)until_space, command_on)){
					setPwmDuty(pwm);
					sprintf((char*)buff_to_send,POWER_MODE_ON, pwm);
				} else {
					if (!strcmp((char *)until_space, command_pwm)){
						if (pwm) {
							setPwmDuty(pwm);
							sprintf((char*)buff_to_send,POWER_MODE_ON, pwm);
						} else {
							setPwmDuty(0);
							sprintf((char*)buff_to_send,POWER_MODE_OFF);
						}
					} else {
						sprintf((char*)buff_to_send, ERROR);
					}
				}
			}
			DebagUart(buff_to_send);
			UCSR0B |= (1 << UDRIE0);
			flag_recive = 0;
		}
	}
}


