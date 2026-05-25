/*        David Valenzuela dvale065@ucr.edu
*         Discussion Section: 22
*         Assignment: Custom Lab Demo 1
*
*         I acknowledge all content contained herein, excluding template or example code, is my own original work.
*
*         Demo Link: https://youtube.com/shorts/BF7kjmPTqQY?feature=share
*/


#ifndef CUSTOMPROJECT120B_DVALE065_TFTLCD_H
#define CUSTOMPROJECT120B_DVALE065_TFTLCD_H

#include <avr/io.h>

// SCL on D13 -> PB5
// SDA on D11 -> PB3
// CS on D10 -> PB2
// DC on D8  -> PB0
// RES on D7  -> PD7

#define LCD_CS   PB2
#define LCD_DC   PB0
#define LCD_RES  PD7
#define LCD_WIDTH   128
#define LCD_HEIGHT  160

// command values
#define ST7735_SWRESET  0x01
#define ST7735_SLPOUT   0x11
#define ST7735_COLMOD   0x3A
#define ST7735_DISPON   0x29
#define ST7735_CASET    0x2A
#define ST7735_RASET    0x2B
#define ST7735_RAMWR    0x2C

// color values
#define LCD_BLACK    0x0000
#define LCD_WHITE    0xFFFF
#define LCD_RED      0xF800
#define LCD_GREEN    0x07E0
#define LCD_BLUE     0x001F
#define LCD_YELLOW   0xFFE0
#define LCD_CYAN     0x07FF
#define LCD_MAGENTA  0xF81F
#define LCD_ORANGE   0xFC00


void LCD_init();

void LCD_sendCommand(unsigned char command);
void LCD_sendData(unsigned char data);

void LCD_setDrawingRange(unsigned char x0, unsigned char y0,
                         unsigned char x1, unsigned char y1);

void LCD_fillScreen(unsigned int color);

void LCD_fillRect(unsigned char x, unsigned char y,
                  unsigned char width, unsigned char height,
                  unsigned int color);

#endif