/*
 * usart.c
 *
 *  Created on: May 3, 2023
 *      Author: briannguyen
 */


#include "string.h"
#include "main.h"
#define DELAY 1000

#define BRR_CLEAR 0xFFFF
#define BRR_SET	0xD0
#define OUT_PORT GPIOA

void UART_init(void){

	// enable clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	// set mode to alternate function [2 is b'10] for alternate function mode
	OUT_PORT->MODER &= ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3);
	OUT_PORT->MODER |= (2 << GPIO_MODER_MODE2_Pos | 2 << GPIO_MODER_MODE3_Pos);
	// establish ports A2, A3
	OUT_PORT->AFR[0] &= ~(GPIO_AFRL_AFSEL2 | GPIO_AFRL_AFSEL3);
	// shift 7 because AF7
	OUT_PORT->AFR[0] |= (7 << GPIO_AFRL_AFSEL2_Pos | 7 << GPIO_AFRL_AFSEL3_Pos);
	// set to fast speed where 3 is b'11 for fast speed
	OUT_PORT->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED2 | GPIO_OSPEEDR_OSPEED3);
	OUT_PORT->OSPEEDR |= (3 << GPIO_OSPEEDR_OSPEED2_Pos | 3 << GPIO_OSPEEDR_OSPEED3_Pos);

	RCC->APB1ENR1 |= (RCC_APB1ENR1_USART2EN); // enable clock

	USART2->CR1 &= ~(USART_CR1_OVER8);

	USART2->BRR &= ~(BRR_CLEAR);
	USART2->BRR |= (BRR_SET);			// set baud rate to 0xD0

	USART2->CR1 &= ~(USART_CR1_M);		// set 01 for word length of 8

	USART2->CR2 &= ~(USART_CR2_STOP); 	// set the  STOP bit to 0 for 1 stop

	USART2->CR1 |= (USART_CR1_UE);		// set the enable to 1

	USART2->CR1 |= (USART_CR1_TE);		// set the transmitter enable to 1

	USART2->CR1 |= (USART_CR1_RE);		// set the receiver enable to 1

	USART2->CR1 |= (USART_CR1_RXNEIE);	// set the receiver interrupt enable to 1

	NVIC->ISER[1] |= (1 << (USART2_IRQn & 0x1F));
	__enable_irq();					// enable interrupt globally
}

void UART_print(char data){
	while ((USART2->ISR & USART_ISR_TXE) == 0){} 	// if TXE is high, it is empty, do nothing otherwise
	USART2->TDR = data;
	for (int i = 0; i < DELAY; i++);
}

void UART_print_str(char *str){
	for (int pos = 0; pos < strlen(str); pos++){
		UART_print(str[pos]);
	}
}

void UART_ESC_code(char *num, char letter){
	while ((USART2->ISR & USART_ISR_TXE) == 0){} 	// if TXE is high, it is empty, do nothing otherwise
	USART2->TDR = 0x1B;

	for (int i = 0; i < DELAY; i++);

	UART_print('[');
	UART_print_str(num);
	UART_print(letter);
}













