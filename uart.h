/*
 * uart.h
 *
 * Created: 19.07.2022 14:22:00
 *  Author: sshmykov
 */ 


#ifndef UART_H_
#define UART_H_


#define USART_BAUDRATE 19200
#define buff_rx_SIZE 32
#define UBRR_VALUE 51
#define SIZEOF_SENDBUF 20
#define SIZE_BUF 128

void UartInit(void);
void flush_buf_tx (void);
void USART_PutChar(const char);
void DebagUart(const char*);

#endif /* UART_H_ */