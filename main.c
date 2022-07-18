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


volatile char buff_rx[buff_rx_SIZE] = {0};
volatile char *buff_rx_pointer = buff_rx;
volatile const char *buff_rx_begin = buff_rx;


#define SIZE_BUF 128
//kolcevoj cyklicheskij bufer
static volatile char cycle_buf_tx[SIZE_BUF];
static volatile uint8_t tail_tx  = 0;      //hvost bufera
static volatile uint8_t head_tx  = 0;      //golova
static volatile uint8_t count_tx = 0;     //cymbol counter


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

#define POWER_PR "[power: %d]\n\r"

void UartInit(void){
	UBRR0 = UBRR_VALUE; 
	UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);
	UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
	UCSR0B |= (1 << RXCIE0);
	UCSR0B |= (1 << UDRIE0);
}

void buff_rx_init(void) {
	memset((char *)buff_rx, '\n', buff_rx_SIZE-1);
	pos = 0;
}

void flush_buf_tx (void) {
	tail_tx  = 0;      //hvost bufera
	head_tx  = 0;      //golova
	count_tx = 0;     //cymbol counter	
}


volatile char flag_recive = 0;

ISR(USART_RX_vect){
	uint8_t ch = UDR0;
	if ((pos >= (buff_rx_SIZE - 1)) || (ch == '\0') || (ch == '\n') || (ch == '\r')) {
		if (pos <= (buff_rx_SIZE - 1)){
			buff_rx[pos] = '\0';
			flag_recive++;
		}
	} 
	else {
		buff_rx[pos++] = ch;
	}
}

ISR(USART_UDRE_vect){
	if (count_tx > 0){                              //если буфер не пустой
		UDR0 = cycle_buf_tx[head_tx];         //записываем в UDR символ из буфера
		count_tx--;                                      //уменьшаем счетчик символов
		head_tx++;                                 //инкрементируем индекс головы буфера
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
		if (count_tx < SIZE_BUF){                    //esli v bufere eshche est' mesto
			cycle_buf_tx[tail_tx] = sym;             //pomeshchaem v nego simvol
			count_tx++;                              //inkrementiruem schetchik simvolov
			tail_tx++;                               //i indeks hvosta bufera
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

void DebagUart_uint8(const char * mess, const int data) {
	char buf[40];
	sprintf((char*)buf,"%s %d\r\n",mess,data);
	DebagUart(buf);
}

void send_to_tx(void) {
	//mode: Off [power: 0]\n
	//printf ("mode: Off [power: %d]",);
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
						buff_rx_init();
						flag = 2;
						j = 0;
					}
					break;
				case 2://OFF
					if (command_off_answer[j]) {
						UDR0 = command_off_answer[j++];
					}
					else{ 
						buff_rx_init();
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
				buff_rx_init();
				flag = 0;
				j = 0;
			}
			break;
	}
}

int main(void) {
	UartInit();
	buff_rx_init();
	sei();
	while (1) {
		if (flag_recive) {
			//DebagUart("Hello\r\n");
			char buff_to_send[40];
			sprintf((char*)buff_to_send,POWER_PR, 100);
			DebagUart(buff_to_send);
			if (!strcmp((char *)buff_rx_begin, command_off)){
				answer = 2;
			} else {
				if (!strcmp((char *)buff_rx_begin, command_on)){
					answer = 1;
				} else {
						flag = 4;
					}
				}
			UCSR0B |= (1 << UDRIE0);
			flag_recive = 0;
		}
	}
}


