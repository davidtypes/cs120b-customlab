/*        David Valenzuela dvale065@ucr.edu
*         Discussion Section: 22
*         Assignment: Custom Lab Demo 1
*
*         I acknowledge all content contained herein, excluding template or example code, is my own original work.
*
*         Demo Link: https://youtube.com/shorts/BF7kjmPTqQY?feature=share
*/


#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

#include "spiAVR.h"
#include "dvale065_TFTLCD.h"
#include "dvale065_helpers.h"

// initializes ports
void HardwareInit() {
  SPI_INIT();
  DDRB = SetBit(DDRB, LCD_DC, 1); // dc is output
  DDRD = SetBit(DDRD, LCD_RES, 1); // res is output

  PORTB = SetBit(PORTB, LCD_CS, 1);
  PORTB = SetBit(PORTB, LCD_DC, 1);
  PORTD = SetBit(PORTD, LCD_RES, 1);
}

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

int main(void) {
  HardwareInit();
  LCD_init();
  DrawTestScreen();

  while (1) {
    // will use later
  }

  return 0;
}