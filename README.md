# nucleotype
A Terminal-Based Typing Test Emulator using the STM32 Microcontroller

Welcome to Nucleotype! This project is operated directly through the VT1000 serial terminal emulator, so no interaction with the hardware is necessary once the program is running. Once the device is connected to power, the reset button on the STM32 board can be pressed to load the program into the terminal.

At startup, you will be asked to choose from the following selections for timing mode by pressing the associated key:

[1] → 10 seconds 
[2] → 15 seconds
[3] → 30 seconds

This allows you to select how long they want their typing test to run for. Once the key associated with the timing mode has been pressed, you will be asked to press enter to confirm that you want to go to the test. A randomly generated set of 50 words will then be displayed in yellow text to indicate to you what you will have to type during their test. As indicated in the on-screen instructions, the goal is to type as many words as possible during the selected time, and once the test is complete, your typing speed in words per minute and typing accuracy as a percentage will be displayed. It is also noted that if you make a typing error, you will only be able to backspace within the word you are currently typing.

To start the test, simply start typing the first word. Once this happens, the LCD display connected to the STM32 board will start displaying a count of how many seconds are left before the typing test is up. This display updates every second, and once the test ends, the bottom row of the LCD is cleared. Once you have started typing, you should keep typing until time ends to try to get the highest typing speed you can. An important note about accuracy is that you can only get 100% accuracy if you type every word correctly and stop typing a new word before the timer goes off. If you are in the middle of a word when the timer goes off, the system will count that word as being spelled incorrectly.

When time is up, a buzzer tone will go off to let you know. You will also not be able to type any more words on the screen. Your results will be displayed below the generated words. If you want to start a new test, all you need to do is press enter on the results page and you will be taken back to the screen where you can select the timing mode.


