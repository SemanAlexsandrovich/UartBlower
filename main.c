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
#include <stdio.h>

#define USART_BAUDRATE 19200
#define buff_rx_SIZE 32
#define UBRR_VALUE 51
#define SIZEOF_SENDBUF 20

volatile char buff_rx[buff_rx_SIZE] = {0};
volatile const char *buff_rx_begin = buff_rx;


#define SIZE_BUF 128

static volatile char cycle_buf_tx[SIZE_BUF];
static volatile uint8_t tail_tx  = 0;
static volatile uint8_t head_tx  = 0;
static volatile uint8_t count_tx = 0;


volatile uint8_t pos = 0;
volatile uint8_t j = 0;
volatile uint8_t flag = 0;
volatile uint8_t pwm = 10;

char command_on[] = "On\r";
char command_off[] = "Off\r";
char command_pwm[] = "Power\r";

volatile uint8_t answer = 2;

#define POWER_MODE_OFF "mode:OFF [power: 0%%]\r\n"
#define POWER_MODE_ON "mode:ON   [power: %d%%]\r\n"
#define ERROR "Error\r\n"


void UartInit(void){
	UBRR0 = UBRR_VALUE; 
	UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);
	UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
	UCSR0B |= (1 << RXCIE0);
	UCSR0B |= (1 << UDRIE0);
}

void flush_buf_tx (void) {
	tail_tx  = 0;
	head_tx  = 0;
	count_tx = 0;
}


volatile char flag_recive = 0;

ISR(USART_RX_vect){
	uint8_t ch = UDR0;
	if ((pos >= (buff_rx_SIZE - 1)) || (ch == '\0') || (ch == '\n')) {
		buff_rx[pos] = '\0';
		pos = 0;
		flag_recive = 1;
	} 
	else {
		buff_rx[pos++] = ch;
	}
}

ISR(USART_UDRE_vect){
	if (count_tx > 0){
		UDR0 = cycle_buf_tx[head_tx];
		count_tx--;
		head_tx++;
		if (head_tx == SIZE_BUF) {
			head_tx = 0;
		}
	} else {
		UCSR0B &= ~(1 << UDRIE0);
	}
}

void USART_PutChar(const char sym) {
cli();
	UCSR0B |= (1 << UDRIE0) | (1 << TXEN0);
	if(((UCSR0A & (1<<UDRE0)) != 0) && (count_tx == 0)) {
		UDR0 = sym;
	} else {
		if (count_tx < SIZE_BUF){
			cycle_buf_tx[tail_tx] = sym;
			count_tx++;
			tail_tx++;
			if (tail_tx == SIZE_BUF) {
				tail_tx = 0;
			}
		}
	}
sei();
}

void DebagUart( const char * data ) {
	char sym;
	while(*data){
		sym = *data++;
		USART_PutChar(sym);
	}
}

int main(void) {
	UartInit();
	sei();
	while (1) {
		if (flag_recive) {
			char buff_to_send[SIZEOF_SENDBUF];
			if (!strcmp((char *)buff_rx_begin, command_off)){
				sprintf((char*)buff_to_send,POWER_MODE_OFF);
			} else {
				if (!strcmp((char *)buff_rx_begin, command_on)){
					sprintf((char*)buff_to_send,POWER_MODE_ON, pwm);
				} else {
					sprintf((char*)buff_to_send, ERROR);//ERROR
				}
			}
			DebagUart(buff_to_send);
			UCSR0B |= (1 << UDRIE0);
			flag_recive = 0;
		}
	}
}


