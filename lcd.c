/*
 * lcd.c
 *
 *  Created on: Jun 5, 2023
 *      Author: briannguyen
 */


// Code Attribution: Eric Huang, Douglas Liu

#include "lcd.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Configures GPIO and RCC for port C (for LCD Display)
void init_lcd_port() {
	// Enable clock for port D
	RCC->AHB2ENR |= LCD_PORT_EN;

	// Clear mode registers for configuration
	LCD_PORT->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1 |
						 GPIO_MODER_MODE2 | GPIO_MODER_MODE3 |
						 GPIO_MODER_MODE4 | GPIO_MODER_MODE5 |
						 GPIO_MODER_MODE6);

	// Finish configuring ports as GP outputs
	LCD_PORT->MODER |= (GPIO_MODER_MODE0_0 | GPIO_MODER_MODE1_0 |
						GPIO_MODER_MODE2_0 | GPIO_MODER_MODE3_0 |
						GPIO_MODER_MODE4_0 | GPIO_MODER_MODE5_0 |
						GPIO_MODER_MODE6_0);

	// Putting output bits into output push pull
	LCD_PORT->OTYPER &= ~(GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1 |
						  GPIO_OTYPER_OT2 | GPIO_OTYPER_OT3 |
						  GPIO_OTYPER_OT4 | GPIO_OTYPER_OT5 |
						  GPIO_OTYPER_OT6);
	// No pull up pull down
	LCD_PORT->PUPDR &= ~(GPIO_PUPDR_PUPD0 |
						 GPIO_PUPDR_PUPD1 |
						 GPIO_PUPDR_PUPD2 |
						 GPIO_PUPDR_PUPD3 |
						 GPIO_PUPDR_PUPD4 |
						 GPIO_PUPDR_PUPD5 |
						 GPIO_PUPDR_PUPD6);

	// Set output speed of registers to very fast
	LCD_PORT->OSPEEDR |= ((3 << GPIO_OSPEEDR_OSPEED0_Pos) |
						  (3 << GPIO_OSPEEDR_OSPEED1_Pos) |
						  (3 << GPIO_OSPEEDR_OSPEED2_Pos) |
						  (3 << GPIO_OSPEEDR_OSPEED3_Pos) |
						  (3 << GPIO_OSPEEDR_OSPEED4_Pos) |
						  (3 << GPIO_OSPEEDR_OSPEED5_Pos) |
						  (3 << GPIO_OSPEEDR_OSPEED6_Pos));
}

// Turns DISPLAY ON and SETS CURSOR.
void lcd_init() {
	delay_us(40000);                 // Power up wait > 40 ms
	for (int idx = 0; idx < 3; idx++) { // wake up 1,2,3: DATA = 0011 XXXX
		lcd_4b_command(0x30);         // Send higher 4 bits of 8 bit cmd
		delay_us(200);                // wait > 160 us
	}
	lcd_4b_command(0x20);            // Put 0x20 on output port
	delay_us(40);
	lcd_command(LCD_FUNC_SET_4BIT);  // FUNCTION SET : 4 bit/ 2 line
	delay_us(40);
	lcd_command(LCD_SET_CURSOR);     // SET CURSOR
	delay_us(40);
	lcd_command(LCD_DISP_ON_BLINK);  // DISP ON, cursor blinks
	delay_us(40);
	lcd_command(LCD_CLEAR_DISP);     // CLEAR Display
	delay_us(40);
	lcd_command(LCD_ENTRY_SET);      // ENTRY MODE SET
	delay_us(5000);                  // Extra long to allow for enough time
									 // to finish initializing LCD
}

// Pulses the LCD ENABLE 0->1->0
void lcd_pulse_en() {
	LCD_PORT->ODR |= (LCD_EN);      // ENABLE HIGH
	delay_us(30);                     // Delay > 300 ns
	LCD_PORT->ODR &= ~(LCD_EN);     // ENABLE LOW
	delay_us(30);                     // Delay > 300 ns
}

// Sends the LCD only the higher 4 bit nibble of command
void lcd_4b_command(uint8_t command) {
	// Function primarily used for 'wake-up' 0x30 commands
	LCD_PORT->ODR &= ~(LCD_DATA); 	  // clear DATA bits
	LCD_PORT->ODR |= (command >> 4);   // DATA = higher 4 bits of command
	delay_us(5);                       // Delay > 5 ms
	lcd_pulse_en();
}

// Sends the higher 4 bit nibble first, then the lower of command
void lcd_command(uint8_t command) {
	LCD_PORT->ODR &= ~(LCD_DATA);               // isolate cmd bits
	LCD_PORT->ODR |= ((command >> 4) & LCD_DATA); // HIGH shifted low
	delay_us(30);
	lcd_pulse_en();                               // Latch HIGH NIBBLE

	LCD_PORT->ODR &= ~(LCD_DATA);               // Isolate cmd bits
	LCD_PORT->ODR |= (command & LCD_DATA);      // LOW nibble
	delay_us(30);
	lcd_pulse_en();                               // Latch LOW NIBBLE
}

// Writes a single character to LCD
void lcd_write_char(char letter) {
	LCD_PORT->ODR |= (LCD_RS);       // RS = HI for data to address
	delay_us(30);
	lcd_command(letter);      	     // character to print
	LCD_PORT->ODR &= ~(LCD_RS);      // RS = LO
}

// Writes a string of characters (phrase) at a specified row of
void lcd_write_string(char *phrase, int rowPosition) {
	// Clear the specified row on LCD
	clear_row(rowPosition);

	// Set cursor to specified row on LCD
	if (rowPosition == 2)
		lcd_command(LCD_BOT_LINE);
	else
		lcd_command(LCD_TOP_LINE);

	// For every character in the string of characters
	// Output to LCD
	for (int idx = 0; phrase[idx] != '\0'; idx++) {
		lcd_write_char(phrase[idx]);
	}
}

// Clears only one specified row of the LCD (either 1 or 2)
void clear_row(int rowPosition) {
	// Set cursor to specified row on LCD
	if (rowPosition == 2)
		lcd_command(LCD_BOT_LINE);
	else
		lcd_command(LCD_TOP_LINE);

	// Write blank spaces to every 16 cells of that row of the LCD
	for (int idx = 0; idx < 16; idx++) {
		lcd_write_char(0b11111110);
	}
}


// Configures SysTick timer for use in delay function
void SysTick_Init(void) {
	// Enable SysTick timer & select CPU clock
	SysTick->CTRL |= (SysTick_CTRL_ENABLE_Msk |
                      SysTick_CTRL_CLKSOURCE_Msk);
	// Disable interrupt
	SysTick->CTRL &= ~(SysTick_CTRL_TICKINT_Msk);
}

// Delay function to create more precise delay
void delay_us(const uint32_t time_us) {
	// set the counts for the specified delay
	SysTick->LOAD = (uint32_t)((time_us * (SystemCoreClock / 1000000)) - 1);
	// Clear timer count
	SysTick->VAL = 0;
	// Clear count flag
	SysTick->CTRL &= ~(SysTick_CTRL_COUNTFLAG_Msk);
	// Wait for flag
	while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
}
