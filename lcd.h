// Code Attribution: Eric Huang, Douglas Liu

#include "main.h"

#ifndef SRC_LCD_H_
#define SRC_LCD_H_

#define LCD_PORT           (GPIOC)
#define LCD_PORT_EN        (RCC_AHB2ENR_GPIOCEN)

#define LCD_DATA           (0x0F) // PC0 - PC1 - PC2 - PC3
                              // DB4 - DB5 - DB6 - DB7
#define LCD_RS             (0x10) // PC4
#define LCD_RW             (0x20) // PC5
#define LCD_EN             (0x40) // PC6

#define LCD_FUNC_SET_4BIT  (0x28)
#define LCD_SET_CURSOR     (0x10)
#define LCD_DISP_ON_BLINK  (0x0F)
#define LCD_ENTRY_SET      (0x06)
#define LCD_TOP_LINE       (0x80)
#define LCD_BOT_LINE       (0xC0)
#define LCD_CLEAR_DISP     (0x01)

void SysTick_Init(void);
void delay_us(const uint32_t time_us);

void init_lcd_port();
void lcd_init();

void lcd_pulse_en();
void lcd_4b_command(uint8_t command);
void lcd_command(uint8_t command);
void lcd_write_char(char letter);
void lcd_write_string(char *phrase, int rowPosition);

void clear_row(int rowPosition);

#endif /* SRC_LCD_H_ */
