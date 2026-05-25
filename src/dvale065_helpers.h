/*        David Valenzuela dvale065@ucr.edu
*         Discussion Section: 22
*         Assignment: Custom Lab Demo 1
*
*         I acknowledge all content contained herein, excluding template or example code, is my own original work.
*
*         Demo Link: https://youtube.com/shorts/BF7kjmPTqQY?feature=share
*/


#ifndef CUSTOMPROJECT120B_DVALE065_HELPERS_H
#define CUSTOMPROJECT120B_DVALE065_HELPERS_H


static inline unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
  return (b ? (x | (0x01 << k)) : (x & ~(0x01 << k)) );
}
static inline unsigned char GetBit(unsigned char x, unsigned char k) {
  return ((x & (0x01 << k)) != 0);
}


#endif