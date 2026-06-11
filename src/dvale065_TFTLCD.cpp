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

#include "dvale065_TFTLCD.h"
#include "dvale065_helpers.h"
#include <util/delay.h>


extern void SPI_SEND(char data);

// lcd ignores spi when cs is 1
static void LCD_select() {
  PORTB = SetBit(PORTB, LCD_CS, 0);
}
static void LCD_deselect() {
  PORTB = SetBit(PORTB, LCD_CS, 1);
}

// dc ? data : command
static void LCD_commandMode() {
  PORTB = SetBit(PORTB, LCD_DC, 0);
}
static void LCD_dataMode() {
  PORTB = SetBit(PORTB, LCD_DC, 1);
}

static void LCD_hardwareReset() {
  PORTD = SetBit(PORTD, LCD_RES, 0);
  _delay_ms(200);
  PORTD = SetBit(PORTD, LCD_RES, 1);
  _delay_ms(200);
}



void LCD_sendCommand(unsigned char command) {
  LCD_select();
  LCD_commandMode();
  SPI_SEND(static_cast<char>(command));
  LCD_deselect();
}

void LCD_sendData(unsigned char data) {
  LCD_select();
  LCD_dataMode();
  SPI_SEND(static_cast<char>(data));
  LCD_deselect();
}

void LCD_init() {
  LCD_hardwareReset();
  LCD_sendCommand(ST7735_SWRESET);
  _delay_ms(150);
  LCD_sendCommand(ST7735_SLPOUT);
  _delay_ms(200);
  LCD_sendCommand(ST7735_COLMOD);
  LCD_sendData(0x05);               // 16-bit color
  _delay_ms(10);
  LCD_sendCommand(ST7735_DISPON);
  _delay_ms(200);
}

void LCD_setDrawingRange(unsigned char x0, unsigned char y0,
                          unsigned char x1, unsigned char y1) {
  LCD_sendCommand(ST7735_CASET);
  LCD_sendData(0x00); // "high byte"
  LCD_sendData(x0); // "low byte"
  LCD_sendData(0x00);
  LCD_sendData(x1);

  LCD_sendCommand(ST7735_RASET);
  LCD_sendData(0x00);
  LCD_sendData(y0);
  LCD_sendData(0x00);
  LCD_sendData(y1);

  LCD_sendCommand(ST7735_RAMWR);
}

void LCD_fillRect(unsigned char x, unsigned char y,
                  unsigned char width, unsigned char height,
                  unsigned int color) {
  // trivial cases
  if (width == 0 || height == 0) {
    return;
  }
  if (x >= LCD_WIDTH || y >= LCD_HEIGHT) {
    return;
  }

  // clip if too big
  if ((x + width - 1) >= LCD_WIDTH) {
    width = LCD_WIDTH - x;
  }
  if ((y + height - 1) >= LCD_HEIGHT) {
    height = LCD_HEIGHT - y;
  }

  unsigned int pixels = width * height;

  LCD_setDrawingRange(x, y, x + width - 1, y + height - 1);

  LCD_select();
  LCD_dataMode();
  for (unsigned int i = 0; i < pixels; i++) {
    // 16-bit color is sent as two bytes: high byte, then low byte
    SPI_SEND((char)(color >> 8));
    SPI_SEND((char)(color & 0xFF));
  }

  LCD_deselect();
}

void LCD_fillScreen(unsigned int color) {
  LCD_fillRect(0, 0, LCD_WIDTH, LCD_HEIGHT, color);
}
