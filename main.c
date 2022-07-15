/*
 * UartBlower.c
 * Answering on command
 * Author : sshmykov
 */ 
#define F_CPU 16000000L
#include <avr/io.h>
//#include <util/delay.h>
#include <avr/interrupt.h>
//#include <avr/sleep.h>
#include <string.h>

#define USART_BAUDRATE 19200
#define BUFF_SIZE 32
#define UBRR_VALUE 51


void UartInit(void){
	UBRR0 = UBRR_VALUE; 
	UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);
	UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
	UCSR0B |= (1 << RXCIE0);
	UCSR0B |= (1 << UDRIE0);
}

char buff[BUFF_SIZE] = {0};
volatile char *buff_pointer = buff;
char *buff_begin = buff;

volatile uint8_t pos;
volatile uint8_t j = 0;
volatile uint8_t flag = 0;

char command_on[] = "On";
char command_off[] = "Off";
char command_pwm[] = "Power";

char command_mode_answer[] = "Mode: ";
char command_on_answer[] = "ON  ";
char command_off_answer[] = "OFF ";
char command_pwm_answer[] = "[power: ";
char command_end_answer[] = "32%]\r";
char command_error_answer[] = "Error\r";

volatile uint8_t answer = 2;

void buff_init(void) {
	memset(buff, '\n', BUFF_SIZE-1);
	pos = 0;
}

void wright_in_buff(uint8_t ch) {
	
	if ((pos >= (BUFF_SIZE - 1)) || (ch == '\0') || (ch == '\n') || (ch == '\r')) {
		if (pos <= (BUFF_SIZE - 1)){
			buff[pos] = '\0';
		} 
		if (!strcmp(buff_begin, command_off)){
			answer = 2;
		} else {
			if (!strcmp(buff_begin, command_on)){
				answer = 1;
			} else {
					flag = 4;
				}
			}
		UCSR0B |= (1 << UDRIE0);
	} 
	else {
		buff[pos++] = ch;
	}
}

ISR(USART_RX_vect){
	cli();
	wright_in_buff(UDR0);
	sei();
}

ISR(USART_UDRE_vect){
	cli();
	//mode: Off [power: 0]\n
	switch (flag){
		case 0://mode:
			if (command_mode_answer[j]){
				UDR0 = command_mode_answer[j++];
			}
			else {
				j = 0;
				flag = 1;
			}
			break;
		case 1://ON or OFF
			switch (answer){
				case 1://ON
					if (command_on_answer[j]) {
						UDR0 = command_on_answer[j++];
					}
					else{ 
						buff_init();
						flag = 2;
						j = 0;
					}
					break;
				case 2://OFF
					if (command_off_answer[j]) {
						UDR0 = command_off_answer[j++];
					}
					else{ 
						buff_init();
						flag = 2;
						j = 0;
					}
					break;
			}
			break;
		case 2://[power:
			if (command_pwm_answer[j]){
				UDR0 = command_pwm_answer[j++];
			}
			else {
				j = 0;
				flag = 3;
			}
			break;
		case 3:
			if (command_end_answer[j]){
				UDR0 = command_end_answer[j++];
			}
			else {
				UCSR0B &= ~(1 << UDRIE0);
				flag = 0;
				j = 0;
			}
			break;
		default:
			if (command_error_answer[j]){
				UDR0 = command_error_answer[j++];
			}
			else {
				UCSR0B &= ~(1 << UDRIE0);
				buff_init();
				flag = 0;
				j = 0;
			}
			break;
	}
	sei();
}


int main(void) {
	UartInit();
	buff_init();
	sei();
	while (1) {
		
		
	}
}


