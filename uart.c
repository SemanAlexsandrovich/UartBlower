/*
 * uart.c
 *
 * Created: 19.07.2022 14:21:29
 *  Author: sshmykov
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"
 
volatile extern char flag_recive;
static volatile uint8_t tail_tx  = 0;
static volatile uint8_t head_tx  = 0;
static volatile uint8_t count_tx = 0;
volatile extern uint8_t pos;
static volatile char cycle_buf_tx[SIZE_BUF];
extern char buff_rx[buff_rx_SIZE];

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