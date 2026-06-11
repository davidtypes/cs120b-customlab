/*        David Valenzuela dvale065@ucr.edu
 *         Discussion Section: 22
 *         Assignment: Custom Lab Demo 2
 *
 *         I acknowledge all content contained herein, excluding template or example code, is my own original work.
 *
 *         Demo Link:
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>

#include "timerISR.h"
#include "spiAVR.h"
#include "dvale065_TFTLCD.h"
#include "dvale065_helpers.h"
#include "dvale065_tasks.h"
#include "dvale065_game.h"

#define NUM_TASKS 5

const unsigned long INPUT_PERIOD = 20;
const unsigned long RESET_PERIOD = 50;
const unsigned long PADDLE_PERIOD = 20;
const unsigned long BALL_PERIOD = 40;
const unsigned long DISPLAY_PERIOD = 20;
const unsigned long GCD_PERIOD = 10;

const unsigned char numTasks = NUM_TASKS;
task tasks[NUM_TASKS];

void TimerISR() {
  for (unsigned char i = 0; i < numTasks; i++) {
    if (tasks[i].elapsedTime >= tasks[i].period) {
      tasks[i].state = tasks[i].TickFct(tasks[i].state);
      tasks[i].elapsedTime = 0;
    }

    tasks[i].elapsedTime += GCD_PERIOD;
  }
}

// ports
void HardwareInit() {
  SPI_INIT();

  DDRB = SetBit(DDRB, LCD_DC, 1);    // dc is output
  DDRD = SetBit(DDRD, LCD_RES, 1);   // res is output

  PORTB = SetBit(PORTB, LCD_CS, 1);  // lcd not selected
  PORTB = SetBit(PORTB, LCD_DC, 1);  // data mode
  PORTD = SetBit(PORTD, LCD_RES, 1); // not resetting

  DDRC = SetBit(DDRC, JOYSTICK_X_BIT, 0);
  PORTC = SetBit(PORTC, JOYSTICK_X_BIT, 0);

  DDRD = SetBit(DDRD, RESET_BUTTON_BIT, 0);
  PORTD = SetBit(PORTD, RESET_BUTTON_BIT, 1); // internal pull-up
}

void TasksInit() {
  unsigned char i = 0;

  tasks[i].state = 0;
  tasks[i].period = INPUT_PERIOD;
  tasks[i].elapsedTime = INPUT_PERIOD;
  tasks[i].TickFct = &TickFct_Input;
  i++;

  tasks[i].state = 0;
  tasks[i].period = RESET_PERIOD;
  tasks[i].elapsedTime = RESET_PERIOD;
  tasks[i].TickFct = &TickFct_Reset;
  i++;

  tasks[i].state = 0;
  tasks[i].period = PADDLE_PERIOD;
  tasks[i].elapsedTime = PADDLE_PERIOD;
  tasks[i].TickFct = &TickFct_Paddle;
  i++;

  tasks[i].state = 0;
  tasks[i].period = BALL_PERIOD;
  tasks[i].elapsedTime = BALL_PERIOD;
  tasks[i].TickFct = &TickFct_Ball;
  i++;

  tasks[i].state = 0;
  tasks[i].period = DISPLAY_PERIOD;
  tasks[i].elapsedTime = DISPLAY_PERIOD;
  tasks[i].TickFct = &TickFct_Display;
}

int main() {
  HardwareInit();

  LCD_init();
  ADC_init();

  gameInit();
  TasksInit();

  TimerSet(GCD_PERIOD);
  TimerOn();

  while (1) {}

  return 0;
}
