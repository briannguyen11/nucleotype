/*
 * helper.h
 *
 *  Created on: May 31, 2023
 *      Author: briannguyen
 */
#include <stddef.h>
#include "main.h"
#ifndef SRC_HELPER_H_
#define SRC_HELPER_H_

#define DFLT_MODE 0
#define NEXT_MODE 1

#define TIME1 1
#define TIME2 2
#define TIME3 3

#define NUM_WORDS 50
#define BANK_SIZE 100

#define PERIOD1 240000000
#define PERIOD2 360000000
#define PERIOD3 720000000

void* checked_malloc(size_t size);

struct _arraylist
{
    size_t size;     // size of the list
    size_t capacity; // max size of list
    char* *arraylist;
};

struct _arraylist* array_list_new();

void TIM2_config(void);

void Buzzer_config(void);

void beepBuzzer(void);

void array_list_add_to_end(struct _arraylist* input_list, char *str);

void analyzeInput(struct _arraylist* charList, struct _arraylist* originalSentence, int timeSelect);

int random_number(int min_num, int max_num);

struct _arraylist* getSentence();

struct _arraylist* getCharFromSentence(struct _arraylist* sentence);

void printSentence(struct _arraylist* sentence);

uint16_t moveCursor(uint16_t pos, struct _arraylist* sentenceCharList);

#endif /* SRC_HELPER_H_ */
