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

#define NUM_TASKS 3

const unsigned long INPUT_PERIOD = 50;
const unsigned long PADDLE_PERIOD = 50;
const unsigned long DISPLAY_PERIOD = 50;
const unsigned long GCD_PERIOD = 50;

const unsigned char numTasks = NUM_TASKS;
task tasks[NUM_TASKS];

void DrawTestScreen() {
  LCD_fillScreen(LCD_BLACK);

  // bricks
  for (unsigned char row = 0; row < 3; row++) {
    for (unsigned char col = 0; col < 6; col++) {
      unsigned char x = 105 - col * 20;
      unsigned char y = 132 - row * 10;

      unsigned int color;

      if (row == 0) {
        color = LCD_RED;
      }
      else if (row == 1) {
        color = LCD_YELLOW;
      }
      else {
        color = LCD_GREEN;
      }

      LCD_fillRect(x, y, 18, 8, color);
    }
  }

  // ball
  LCD_fillRect(61, 65, 5, 5, LCD_WHITE);

  // paddle
  LCD_fillRect(48, 10, 32, 5, LCD_WHITE);

  //LCD_fillRect(117, 149, 8, 8, LCD_CYAN);
  //LCD_fillRect(107, 149, 8, 8, LCD_CYAN);
  //LCD_fillRect(97, 149, 8, 8, LCD_CYAN);
}

void TimerISR() {
  for (unsigned char i = 0; i < numTasks; i++) {
    if (tasks[i].elapsedTime == tasks[i].period) {
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

  DDRC = SetBit(DDRC, PC0, 0);       // joystick input
  PORTC = SetBit(PORTC, PC0, 0);
}

void TasksInit() {
  unsigned char i = 0;

  tasks[i].state = 0;
  tasks[i].period = INPUT_PERIOD;
  tasks[i].elapsedTime = INPUT_PERIOD;
  tasks[i].TickFct = &TickFct_Input;
  i++;

  tasks[i].state = 0;
  tasks[i].period = PADDLE_PERIOD;
  tasks[i].elapsedTime = PADDLE_PERIOD;
  tasks[i].TickFct = &TickFct_Paddle;
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