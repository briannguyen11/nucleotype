/*
 * helper.c
 *
 *  Created on: May 31, 2023
 *      Author: briannguyen
 */


#include "helper.h"
#include "usart.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INTIAL_CAPACITY 4
#define WAIT_TIME 200000

/*
 * Function configures ARR and CCR1 interrupts for TIM2
 * Does not return
*/
void TIM2_config(void) {
	// Configure TIM2 to interrupt every 15 seconds
	RCC->APB1ENR1 |= (RCC_APB1ENR1_TIM2EN);                  // enable TIM2
	TIM2->DIER |= (TIM_DIER_UIE | TIM_DIER_CC1IE);           // enable interrupt on update and CCR1 event
	TIM2->SR &= ~(TIM_SR_UIF | TIM_SR_CC1IF);                // clear update and CCR1 interrupt flag
	TIM2->CCMR1 &= ~(TIM_CCMR1_CC1S);                        // set CCR mode for output

	// enable TIM2 interrupts in NVIC
	NVIC->ISER[0] = (1 << (TIM2_IRQn & 0x1F));

	__enable_irq();   // enable interrupts globally
}

/*
 * Function checks if malloc can safely be called
 * Does not return
*/
void* checked_malloc(size_t size){
    int *p;
    p = malloc(size);
    if (p == NULL){
        perror("malloc");
        exit(1);
    }
    return p;
}

/*
 * Function configures GPIO to be used to activate buzzer
 * Does not return
*/
void Buzzer_config(void) {
    // enable clock for GPIOA
    RCC->AHB2ENR = RCC_AHB2ENR_GPIOAEN;

    // Configure PC0 for buzzer
    GPIOA->MODER &= ~(GPIO_MODER_MODE0);                     // clear MODES 0
    GPIOA->MODER |= (1 << GPIO_MODER_MODE0_Pos);             // set MODES 0
    GPIOA->OTYPER &= ~(GPIO_OTYPER_OT0);                     // push-pull
    GPIOA->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED0);               // low speed
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD0);                     // no pull up / pull down
}

/*
 * Function creates buzzer tone used when timer ends
 * Does not return
*/
void beepBuzzer(void){
	for(int i = 0; i < 5; i++) {
	  GPIOA->ODR &= ~(GPIO_ODR_OD0);             // turn off
	  for(int i = 0; i < WAIT_TIME; i++);                // wait
	  GPIOA->ODR |= (GPIO_ODR_OD0);              // turn on
	  for(int i = 0; i < WAIT_TIME; i++);                // wait
	}
	GPIOA->ODR &= ~(GPIO_ODR_OD0);             // turn off
}

/*
 * Function creates new arraylist with default attributes
 * Returns arraylist struct
*/
struct _arraylist* array_list_new()
{
    struct _arraylist* list = checked_malloc(sizeof(struct _arraylist));
    list->size = 0;
    list->capacity = INTIAL_CAPACITY;
    list->arraylist = calloc(INTIAL_CAPACITY, sizeof(char*));

    return list;
}

/*
 * Function adds new node to end of arraylist and increases size
 * Does not return
*/
void array_list_add_to_end(struct _arraylist* input_list, char *str){
    if(input_list->size == input_list->capacity){
        char* *arr_new = NULL;
        size_t new_size= input_list->capacity * 2;                                        //update capacity
        arr_new = realloc(input_list->arraylist, sizeof(char*) * new_size);
        input_list->arraylist = arr_new;
        input_list->capacity = new_size;
    }
    input_list->arraylist[input_list->size] = str;                                        //update element
    input_list->size += 1;                                                                //update size
}

/*
 * Function calculates and prints speed and accuracy
 * Does not return
 */
void analyzeInput(struct _arraylist* charList, struct _arraylist* originalSentence, int timeSelect){

	struct _arraylist* inputWordList = array_list_new();

	char tempWord[10];		// init temp word
	int charPos = 0;		// init char pos in word
	memset(tempWord, '\0', sizeof(tempWord));

	for (int i = 0; i < charList->size; i++){
		// space means end of word -> store
		if (charList->arraylist[i][0] == ' '){
			array_list_add_to_end(inputWordList, strdup(tempWord));
			memset(tempWord, '\0', sizeof(tempWord));
			charPos = 0;
		}
		// get the last word in char list
		else if (i == (charList->size - 1)){
			tempWord[charPos] = charList->arraylist[i][0];
			array_list_add_to_end(inputWordList, strdup(tempWord));
			memset(tempWord, '\0', sizeof(tempWord));
		}
		// build word from chars
		else {
			tempWord[charPos] = charList->arraylist[i][0];
			charPos += 1;
		}
	}
	// determine number of input words are valid
	int validCount = 0;
	for (int i = 0; i < inputWordList->size; i++){
		if (strcmp(inputWordList->arraylist[i], originalSentence->arraylist[i]) == 0){
			validCount += 1;
		}
	}

	// scale to wpm and print
	int scaledValidCount = 0;
	// scale to 60 seconds
	if (timeSelect == TIME1){
		scaledValidCount = validCount * 6;
	}
	if (timeSelect == TIME2){
		scaledValidCount = validCount * 4;
	}
	if (timeSelect == TIME3){
		scaledValidCount = validCount * 2;
	}
	char tempBuf1[10];
	UART_print_str(itoa(scaledValidCount, tempBuf1, 10));
	UART_print_str(" wpm | ");

	// calculate accuracy and print
	int accuracy = (validCount * 100.0) / inputWordList->size;
	char accBuf[10];
	UART_print_str(itoa(accuracy, accBuf, 10));
	UART_print_str("% accuracy");
}

/*
 * Function generates a random number within min and max
 * Returns random number
*/
int random_number(int min_num, int max_num) {
	int min = min_num;
	int max = max_num;
	int randomNum;
	randomNum = (rand() % (max - min + 1)) + min;
	return randomNum;
}

/*
 * Function generates a sentence of 50 non-repeating words
 * Returns ArrayList containing words for the prompted sentence
*/
struct _arraylist* getSentence() {
	struct _arraylist* final_sentence = array_list_new();
	char word_bank[BANK_SIZE][9] =
	   {{"are"}, {"my"}, {"is"}, {"will"}, {"be"},
		{"can"}, {"what"}, {"for"}, {"might"}, {"need"},
		{"take"}, {"year"}, {"play"}, {"off"}, {"such"},
		{"head"}, {"house"}, {"number"}, {"sick"}, {"through"},
		{"she"}, {"mine"}, {"those"}, {"tasty"}, {"you"},
		{"mild"}, {"school"}, {"mean"}, {"a"}, {"look"},
		{"he"}, {"dead"}, {"people"}, {"more"}, {"expert"},
		{"chaos"}, {"come"}, {"same"}, {"help"}, {"get"},
		{"nature"}, {"problem"}, {"think"}, {"group"}, {"love"},
		{"benefit"}, {"lake"}, {"pure"}, {"heat"}, {"even"},
		{"future"}, {"square"}, {"level"}, {"could"}, {"plan"},
		{"world"}, {"lie"}, {"robot"}, {"bet"}, {"hand"},
		{"cottage"}, {"back"}, {"write"}, {"blade"}, {"hold"},
		{"bottom"}, {"could"}, {"major"}, {"now"}, {"beard"},
		{"where"}, {"your"}, {"at"}, {"many"}, {"into"},
		{"point"}, {"beat"}, {"consider"}, {"take"}, {"bold"},
		{"music"}, {"explain"}, {"bottle"}, {"chair"}, {"board"},
		{"scream"}, {"cat"}, {"smile"}, {"ready"}, {"hair"},
		{"major"}, {"around"}, {"light"}, {"time"}, {"word"},
		{"increase"}, {"feel"}, {"shoe"}, {"bake"}, {"aware"}};

	int used_array[BANK_SIZE];
	int random_array[NUM_WORDS];
	int random_num, count = 0;
	memset(used_array, 0, sizeof(used_array));          // initialize each element to 0

	while (count < NUM_WORDS) {
		random_num = random_number(0, BANK_SIZE - 1);   // generate random number
		if (!used_array[random_num]) {                  // check if number used before
			random_array[count] = random_num;           // add num  to array
			used_array[random_num] = 1;                 // mark num as used
			count++;                                    // increase random num count
		}
	}
	// for each random number index, add associated word to final sentence arraylist
	for (int index = 0; index < NUM_WORDS; index++) {
		array_list_add_to_end(final_sentence, strdup(word_bank[random_array[index]]));
	}
	return final_sentence;
}

/*
 * Function converts ArrayList of words into ArrayList of chars
 * Returns ArrayList of chars from original prompted sentence
*/
struct _arraylist* getCharFromSentence(struct _arraylist* sentence){
	struct _arraylist* sentenceCharList = array_list_new();
	for (int i = 0; i < sentence->size; i++){
		int length = strlen(sentence->arraylist[i]);
		for (int j = 0; j < length; j++){
			char ptr[2];
			ptr[0] = sentence->arraylist[i][j];
			ptr[1] = '\0';
			if (j == length - 1){
				// UART_print_str(ptr);
				array_list_add_to_end(sentenceCharList, strdup(ptr));
				array_list_add_to_end(sentenceCharList, " ");
			}
			else {
				array_list_add_to_end(sentenceCharList, strdup(ptr));
			}
		}
	}
	return sentenceCharList;
}
/*
 * Function prints the prompted sentence (10 words per line)
 * Deos not return
*/
void printSentence(struct _arraylist* sentence) {
    char temp_count[5];
    for (int word = 0; word < sentence->size; word++) {
        if ((word % 10) == 0) {
        	int temp_num = (word / 10) + 8; 						// 8 is offset for formatting
            UART_ESC_code("", 'H');
            UART_ESC_code(itoa(temp_num, temp_count, 10), 'B'); 	// go to new line every 10 words
        }
        UART_print_str(sentence->arraylist[word]);
        UART_print(' ');
    }
}

/*
 * Function finds where the next word is in the prompted sentence
 * Returns amount cursor needs to shift to the right
*/
uint16_t moveCursor(uint16_t pos, struct _arraylist* sentenceCharList){
	for (int i = pos; i < sentenceCharList->size; i++){
		if (strcmp(sentenceCharList->arraylist[i], " ") == 0){
			return i;
		}
	}
	return -1;
}



