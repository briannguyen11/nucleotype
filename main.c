/* USER CODE BEGIN Header */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "main.h"
#include "helper.h"
#include "usart.h"
#include "lcd.h"

#define CLEAR 0
#define POS_DF -1
#define CURSOR_DF 8

void SystemClock_Config(void);

typedef enum {
	WELCOME_STATE,
	INPUT_STATE,
	RESULT_STATE,
} state_var_type;

state_var_type state = WELCOME_STATE;
uint16_t mode = DFLT_MODE;
uint16_t timeSelect = CLEAR;
uint32_t period = 0;

uint16_t pos = POS_DF;
uint16_t cursorPos = CURSOR_DF;

uint16_t spaceFlg = CLEAR;
uint16_t timeFlg = CLEAR;

uint16_t spaceNum = 0;
uint16_t wordsPerLine = 0;

uint16_t constCount;
uint16_t count;
char count_buff[10];
char seconds_string[9] = " seconds";

struct _arraylist* charList;
struct _arraylist* sentenceCharList;

int main(void)
{
	// STM32
	HAL_Init();
	SystemClock_Config();

	// UART
	UART_init();

	// TIM2
	TIM2_config();

	// Buzzer
	Buzzer_config();

	//LCD
	SysTick_Init();
	init_lcd_port();
	lcd_init();

	// RNG
	srand(time(NULL));  // Initialize random number generator with a time-based seed

	struct _arraylist* currSentence;

	while (1)
	{
		switch(state){
		case WELCOME_STATE:
			pos = POS_DF;
			cursorPos = CURSOR_DF;

			timeSelect = CLEAR;
			TIM2->CNT = 0;

			timeFlg = CLEAR;
			wordsPerLine = 0;


			UART_ESC_code("37", 'm');		// white font
			UART_ESC_code("2", 'J'); 		// clear screen
			UART_ESC_code("", 'H');  		// top left

			/*** print instructions ***/
			UART_print_str("-------------------------");
			UART_ESC_code("2;0", 'H');
			UART_print_str("| WELCOME to NUCLEOTYPE |");
			UART_ESC_code("3;0", 'H');
			UART_print_str("-------------------------");

			UART_ESC_code("5;0", 'H');
			UART_print_str("--> press 1, 2, or 3 to select timing mode.");

			UART_ESC_code("7;0", 'H');
			UART_print_str("[1] 10 seconds to type as many words as you can");

			UART_ESC_code("8;0", 'H');
			UART_print_str("[2] 15 seconds to type as many words as you can");

			UART_ESC_code("9;0", 'H');
			UART_print_str("[3] 30 seconds to type as many words as you can");

			/*** wait for time selection ***/
			period = 0;
			while(1){
				if (timeSelect != 0){
					if (timeSelect == TIME1){
						period = PERIOD1;
						count = 10;
						constCount = 10;
					}
					if (timeSelect == TIME2){
						period = PERIOD2;
						count = 15;
						constCount = 15;
					}
					if (timeSelect == TIME3){
						period = PERIOD3;
						count = 30;
						constCount = 30;
					}
					char timeBuf[10];
					UART_ESC_code("11;0", 'H');
					UART_print_str("Mode [");
					UART_print_str(itoa(timeSelect, timeBuf, 10));
					UART_print_str("] selected");
					break;
				}
			}

			TIM2->ARR = period - 1;
			TIM2->CCR1 = period / 15;

			UART_ESC_code("13;0", 'H');
			UART_print_str("--> press ENTER to continue");

			/*** wait for mode select ***/
			while(1){
				if (mode != DFLT_MODE){
					break;
				}
			}

			/*** switch to desired mode ***/
			state = INPUT_STATE;

		break;

		case INPUT_STATE:
			/*** print title and instructions ***/
			UART_ESC_code("2", 'J'); 		// clear screen
			UART_ESC_code("", 'H');  		// top left
			UART_print_str("-------------");
			UART_ESC_code("2;0", 'H');
			UART_print_str("| TIME TEST |");
			UART_ESC_code("3;0", 'H');
			UART_print_str("-------------");

			UART_ESC_code("5;0", 'H');
			UART_print_str("1) Your goal is to type as many words as possible");

			currSentence = getSentence();
			sentenceCharList = getCharFromSentence(currSentence);

			/*** print sentence and get user input ***/

			UART_ESC_code("6;0", 'H');
			UART_print_str("2) Start typing to begin!");

			UART_ESC_code("7;0", 'H');
			UART_print_str("** CAUTION: you can only backpace within a word but not to a previous completed word");

			UART_ESC_code("9;0", 'H');
			UART_ESC_code("33", 'm');		// yellow color

			printSentence(currSentence);	// print prompt sentence

			UART_ESC_code("37", 'm');		// white color
			UART_ESC_code("9;0", 'H');

			/*** store and analyze user input ***/
			charList = array_list_new();

			while(timeFlg != 1);
			TIM2->CR1 &= ~(TIM_CR1_CEN); 	// stop timer
			mode = DFLT_MODE;				// reset mode

			beepBuzzer();
			clear_row(2);

			state = RESULT_STATE;

		break;

		case RESULT_STATE:

			UART_ESC_code("37", 'm');		// white color
			UART_ESC_code("15;0", 'H');
			UART_print_str("RESULTS:");

			UART_ESC_code("16;0", 'H');
			analyzeInput(charList, currSentence, timeSelect);

			UART_ESC_code("18;0", 'H');
			UART_print_str("--> press ENTER to go back to HOME");

			/*** wait for home input ***/
			while(1){
				if (mode == NEXT_MODE){
					break;
				}
			}
			mode = DFLT_MODE;				// reset mode

			/*** free memory ***/
			free(currSentence->arraylist);
			free(currSentence);
			free(sentenceCharList->arraylist);
			free(sentenceCharList);
			free(charList->arraylist);
			free(charList);

			/*** switch back to welcome ***/
			state = WELCOME_STATE;

		break;

		}
	}
}

void USART2_IRQHandler(void){
	if((USART2->ISR & USART_ISR_RXNE) != 0){
		char receive_char = USART2->RDR;

		if (receive_char == '1'){
			timeSelect = TIME1;
		}
		else if (receive_char == '2'){
			timeSelect = TIME2;
		}
		else if (receive_char == '3'){
			timeSelect = TIME3;
		}
		else if (receive_char == '\r'){
			mode = NEXT_MODE;
		}
		else if (receive_char == '\x7F'){
			if (mode != DFLT_MODE){
				char* letter = sentenceCharList->arraylist[pos];
				if (strcmp(letter, " ") != 0){
					UART_ESC_code("1", 'D'); 		// left 1

					UART_ESC_code("33", 'm');		// yellow font


					UART_print_str(letter);
					UART_ESC_code("37", 'm');		// white font

					UART_ESC_code("1", 'D'); 		// left 1

					if (charList->size != 0){
						charList->size -= 1;		// decrease input charlist by 1
					}
					pos -= 1;						// shift pos back
				}
			}
		}
		else if (receive_char == ' '){
			if (mode != DFLT_MODE){

				// move cursor to next word
				if (spaceNum < 1){
					char buf[10];
					uint16_t tempPos = moveCursor(pos, sentenceCharList);
					char* move = itoa(tempPos - pos, buf, 10);

					UART_ESC_code(move, 'C');
					pos = tempPos;

					array_list_add_to_end(charList, " ");
					spaceNum += 1;
					wordsPerLine += 1;
				}
			}
		}
		else {
			if (mode != DFLT_MODE){
				// increment global pos for sentenceCharList
				pos += 1;
				spaceNum = 0;

				// start count one first keypress
				if (pos == 0){
					TIM2->CR1 |= (TIM_CR1_CEN); 	// start timer
					 lcd_write_string("Time Remaining:", 1);
					 lcd_write_string(strcat(itoa(count, count_buff, 10), seconds_string), 2);
				}

				if (wordsPerLine == 10){
					cursorPos += 1;
					char cursorBuf[10];
					UART_ESC_code("", 'H');  									// top left
					UART_ESC_code(itoa(cursorPos, cursorBuf, 10), 'B'); 		// down X

					wordsPerLine = 0;
				}

				// ensure no overtyping a word for misalignment
				if (sentenceCharList->arraylist[pos][0] != ' '){
					// check if input char is incorrect
					if (receive_char != sentenceCharList->arraylist[pos][0]){
						UART_ESC_code("31", 'm');	// red font
					}
					// output char typed in
					USART2->TDR = receive_char;
					UART_ESC_code("37", 'm');		// white font

					// convert received char into string and add to arraylist
					char str[2];
					str[0] = receive_char;
					str[1] = '\0';

					array_list_add_to_end(charList, strdup(str));
				}
				else {
					pos -= 1;

				}
			}
		}
	}
}

void TIM2_IRQHandler(void) {
    // check status register for update event flag
	if (TIM2->SR & TIM_SR_UIF) {
		TIM2->SR &= ~(TIM_SR_UIF);  					// clear flag
		timeFlg = 1;
		count = 1;
	}
	else if (TIM2->SR & TIM_SR_CC1IF)
	{
		TIM2->SR &= ~(TIM_SR_CC1IF);          		    // clear ccr interrupt flag

		count -= 1;

		lcd_write_string("Time Remaining:", 1);
		if (count == 1) {
			lcd_write_string(strcat(itoa(count, count_buff, 10), " second"), 2); // print count in seconds
		}
		else {
			lcd_write_string(strcat(itoa(count, count_buff, 10), seconds_string), 2); // print count in seconds
		}

		TIM2->CCR1 += (period / constCount) - 1;         		// increment CCR by num clocks in 1 second
	}
}


/**
  * @brief System Clock Configuration
  * @retval Nones
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_9;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
