/*
 * usart.h
 *
 *  Created on: May 3, 2023
 *      Author: briannguyen
 */

#ifndef SRC_USART_H_
#define SRC_USART_H_

void UART_init(void);
void UART_print(char data);
void UART_print_str(char *str);
void UART_ESC_code(char *num, char letter);


#endif /* SRC_USART_H_ */
